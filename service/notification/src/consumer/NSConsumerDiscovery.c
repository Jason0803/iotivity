//******************************************************************
//
// Copyright 2016 Samsung Electronics All Rights Reserved.
//
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

#include "NSConsumerDiscovery.h"

#include <string.h>
#include "NSCommon.h"
#include "NSConsumerCommon.h"
#include "NSConstants.h"
#include "ocpayload.h"
#include "oic_malloc.h"
#include "oic_string.h"

#define NS_DISCOVER_QUERY "/oic/res?rt=oic.r.notification"
#define NS_PRESENCE_SUBSCRIBE_QUERY "coap://224.0.1.187:5683/oic/ad?rt=oic.r.notification"
#define NS_GET_INFORMATION_QUERY "/notification?if=oic.if.notification"

NSProvider_internal * NSGetProvider(OCClientResponse * clientResponse);

OCStackApplicationResult NSConsumerPresenceListener(
        void * ctx, OCDoHandle handle, OCClientResponse * clientResponse)
{
    (void) ctx;
    (void) handle;

    NS_VERTIFY_NOT_NULL(clientResponse, OC_STACK_KEEP_TRANSACTION);
    NS_VERTIFY_STACK_OK(clientResponse->result, OC_STACK_KEEP_TRANSACTION);

    NS_LOG_V(DEBUG, "Presence income : %s:%d",
            clientResponse->devAddr.addr, clientResponse->devAddr.port);
    NS_LOG_V(DEBUG, "Presence result : %d",
            clientResponse->result);
    NS_LOG_V(DEBUG, "Presence sequenceNum : %d",
            clientResponse->sequenceNumber);

    if (!NSIsStartedConsumer())
    {
        return OC_STACK_DELETE_TRANSACTION;
    }

    OCPresencePayload * payload = (OCPresencePayload *)clientResponse->payload;
    if (payload->trigger == OC_PRESENCE_TRIGGER_DELETE ||
            clientResponse->result == OC_STACK_PRESENCE_STOPPED)
    {
        // TODO find request and cancel
        NS_LOG(DEBUG, "stopped presence or resource is deleted.");
        //OCCancel(handle, NS_QOS, NULL, 0);
    }

    else if (payload->trigger == OC_PRESENCE_TRIGGER_CREATE)
    {
        NS_LOG(DEBUG, "started presence or resource is created.");
        NSInvokeRequest(NULL, OC_REST_DISCOVER, clientResponse->addr,
            NS_DISCOVER_QUERY, NULL, NSProviderDiscoverListener, NULL);
    }

    return OC_STACK_KEEP_TRANSACTION;
}

OCStackApplicationResult NSProviderDiscoverListener(
        void * ctx, OCDoHandle handle, OCClientResponse * clientResponse)
{
    (void) ctx;
    (void) handle;

    NS_VERTIFY_NOT_NULL(clientResponse, OC_STACK_KEEP_TRANSACTION);
    NS_VERTIFY_NOT_NULL(clientResponse->payload, OC_STACK_KEEP_TRANSACTION);
    NS_VERTIFY_STACK_OK(clientResponse->result, OC_STACK_KEEP_TRANSACTION);

    NS_LOG_V(DEBUG, "Discover income : %s:%d",
            clientResponse->devAddr.addr, clientResponse->devAddr.port);
    NS_LOG_V(DEBUG, "Discover result : %d",
            clientResponse->result);
    NS_LOG_V(DEBUG, "Discover sequenceNum : %d",
            clientResponse->sequenceNumber);

    if (!NSIsStartedConsumer())
    {
        return OC_STACK_DELETE_TRANSACTION;
    }

    OCResourcePayload * resource = ((OCDiscoveryPayload *)clientResponse->payload)->resources;
    while (resource)
    {
        if (!strcmp(resource->uri, NS_RESOURCE_URI))
        {
            NSInvokeRequest(NULL, OC_REST_GET, clientResponse->addr,
                    NS_RESOURCE_URI, NULL, NSIntrospectProvider, NULL);
        }
        resource = resource->next;
    }

    return OC_STACK_KEEP_TRANSACTION;
}

void NSRemoveProviderObj(NSProvider_internal * provider)
{
    NSOICFree(provider->messageUri);
    NSOICFree(provider->syncUri);

    provider->messageHandle = NULL;
    provider->syncHandle = NULL;
    NSOICFree(provider->addr);

    NSOICFree(provider);
}

OCStackApplicationResult NSIntrospectProvider(
        void * ctx, OCDoHandle handle, OCClientResponse * clientResponse)
{
    (void) ctx;
    (void) handle;

    NS_VERTIFY_NOT_NULL(clientResponse, OC_STACK_KEEP_TRANSACTION);
    NS_VERTIFY_STACK_OK(clientResponse->result, OC_STACK_KEEP_TRANSACTION);

    NS_LOG_V(DEBUG, "GET response income : %s:%d",
            clientResponse->devAddr.addr, clientResponse->devAddr.port);
    NS_LOG_V(DEBUG, "GET response result : %d",
            clientResponse->result);
    NS_LOG_V(DEBUG, "GET response sequenceNum : %d",
            clientResponse->sequenceNumber);
    NS_LOG_V(DEBUG, "GET response resource uri : %s",
            clientResponse->resourceUri);

    if (!NSIsStartedConsumer())
    {
        return OC_STACK_DELETE_TRANSACTION;
    }

    NSProvider_internal * newProvider = NSGetProvider(clientResponse);
    NS_VERTIFY_NOT_NULL(newProvider, OC_STACK_KEEP_TRANSACTION);

    NS_LOG(DEBUG, "build NSTask");
    NSTask * task = NSMakeTask(TASK_CONSUMER_PROVIDER_DISCOVERED, (void *) newProvider);
    NS_VERTIFY_NOT_NULL_WITH_POST_CLEANING(task, NS_ERROR, NSRemoveProviderObj(newProvider));

    NSConsumerPushEvent(task);

    return OC_STACK_KEEP_TRANSACTION;
}

void NSGetProviderPostClean(char * pId, char * mUri, char * sUri, OCDevAddr * addr)
{
    NSOICFree(pId);
    NSOICFree(mUri);
    NSOICFree(sUri);
    NSOICFree(addr);
}

NSProvider_internal * NSGetProvider(OCClientResponse * clientResponse)
{
    NS_LOG(DEBUG, "create NSProvider");
    NS_VERTIFY_NOT_NULL(clientResponse->payload, NULL);

    OCRepPayload * payload = (OCRepPayload *)clientResponse->payload;
    while (payload)
    {
        NS_LOG_V(DEBUG, "Payload Key : %s", payload->values->name);
        payload = payload->next;
    }

    payload = (OCRepPayload *)clientResponse->payload;

    char * providerId = NULL;
    char * messageUri = NULL;
    char * syncUri = NULL;
    int64_t accepter = 0;
    OCDevAddr * addr = NULL;

    NS_LOG(DEBUG, "get information of accepter");
    bool getResult = OCRepPayloadGetPropInt(payload, NS_ATTRIBUTE_POLICY, & accepter);
    NS_VERTIFY_NOT_NULL(getResult == true ? (void *) 1 : NULL, NULL);

    NS_LOG(DEBUG, "get provider ID");
    getResult = OCRepPayloadGetPropString(payload, NS_ATTRIBUTE_PROVIDER_ID, & providerId);
    NS_VERTIFY_NOT_NULL(getResult == true ? (void *) 1 : NULL, NULL);

    NS_LOG(DEBUG, "get message URI");
    getResult = OCRepPayloadGetPropString(payload, NS_ATTRIBUTE_MESSAGE, & messageUri);
    NS_VERTIFY_NOT_NULL_WITH_POST_CLEANING(getResult == true ? (void *) 1 : NULL, NULL,
            NSGetProviderPostClean(providerId, messageUri, syncUri, addr));

    NS_LOG(DEBUG, "get sync URI");
    getResult = OCRepPayloadGetPropString(payload, NS_ATTRIBUTE_SYNC, & syncUri);
    NS_VERTIFY_NOT_NULL_WITH_POST_CLEANING(getResult == true ? (void *) 1 : NULL, NULL,
            NSGetProviderPostClean(providerId, messageUri, syncUri, addr));

    NS_LOG(DEBUG, "get provider address");
    addr = (OCDevAddr *)OICMalloc(sizeof(OCDevAddr));
    NS_VERTIFY_NOT_NULL_WITH_POST_CLEANING(addr, NULL,
           NSGetProviderPostClean(providerId, messageUri, syncUri, addr));
    memcpy(addr, clientResponse->addr, sizeof(OCDevAddr));

    NSProvider_internal * newProvider
        = (NSProvider_internal *)OICMalloc(sizeof(NSProvider_internal));
    NS_VERTIFY_NOT_NULL(newProvider, NULL);

    OICStrcpy(newProvider->providerId, sizeof(char) * NS_DEVICE_ID_LENGTH, providerId);
    NSOICFree(providerId);
    newProvider->messageUri = messageUri;
    newProvider->syncUri = syncUri;
    newProvider->accessPolicy = (NSAccessPolicy)accepter;
    newProvider->addr = addr;
    newProvider->messageHandle = NULL;
    newProvider->syncHandle = NULL;

    return newProvider;
}

void NSConsumerDiscoveryTaskProcessing(NSTask * task)
{
    NS_VERTIFY_NOT_NULL_V(task);

    NS_LOG_V(DEBUG, "Receive Event : %d", (int)task->taskType);
    if (task->taskType == TASK_EVENT_CONNECTED || task->taskType == TASK_CONSUMER_REQ_DISCOVER)
    {
        NSInvokeRequest(NULL, OC_REST_DISCOVER, NULL, NS_DISCOVER_QUERY,
                NULL, NSProviderDiscoverListener, NULL);
    }
    else
    {
        NS_LOG(ERROR, "Unknown type message");
    }
}
