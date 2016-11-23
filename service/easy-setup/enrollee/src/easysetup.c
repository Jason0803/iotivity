//******************************************************************
//
// Copyright 2015 Samsung Electronics All Rights Reserved.
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

/**
 * @file
 *
 * This file contains the implementation for EasySetup Enrollee device
 */

#include "easysetup.h"
#include "logger.h"
#include "resourcehandler.h"
#include "oic_string.h"

/**
 * @var ES_ENROLLEE_TAG
 * @brief Logging tag for module name.
 */
#define ES_ENROLLEE_TAG "ES"

//-----------------------------------------------------------------------------
// Private variables
//-----------------------------------------------------------------------------

static bool gIsSecured = false;

static ESProvisioningCallbacks gESProvisioningCb;
static ESDeviceProperty gESDeviceProperty;

void ESWiFiRsrcCallback(ESResult esResult, ESWiFiProvData *eventData)
{
    OIC_LOG_V(DEBUG, ES_ENROLLEE_TAG, "ESWiFiRsrcCallback IN");

    if(esResult != ES_OK)
    {
        OIC_LOG_V(ERROR, ES_ENROLLEE_TAG, "ESWiFiRsrcCallback Error Occured");
        return;
    }

    // deliver data to ESProvisioningCallbacks
    if(gESProvisioningCb.WiFiProvCb != NULL)
    {
        gESProvisioningCb.WiFiProvCb(eventData);
    }
    else
    {
        OIC_LOG_V(ERROR, ES_ENROLLEE_TAG, "WiFiProvCb is NULL");
        return;
    }
}

void ESCloudRsrcCallback(ESResult esResult, ESCloudProvData *eventData)
{
    OIC_LOG_V(DEBUG, ES_ENROLLEE_TAG, "ESCloudRsrcCallback IN");

    if(esResult != ES_OK)
    {
        OIC_LOG_V(ERROR, ES_ENROLLEE_TAG, "ESCloudRsrcCallback Error Occured");
        return;
    }

    if(gESProvisioningCb.CloudDataProvCb != NULL)
    {
        gESProvisioningCb.CloudDataProvCb(eventData);
    }
    else
    {
        OIC_LOG_V(ERROR, ES_ENROLLEE_TAG, "CloudDataProvCb is NULL");
        return;
    }
}

void ESDevconfRsrcallback(ESResult esResult, ESDevConfProvData *eventData)
{
    OIC_LOG_V(DEBUG, ES_ENROLLEE_TAG, "ESDevconfRsrcallback IN");

    if(esResult != ES_OK)
    {
        OIC_LOG_V(ERROR, ES_ENROLLEE_TAG, "ESDevconfRsrcallback Error Occured");
        return;
    }

    if(gESProvisioningCb.DevConfProvCb != NULL)
    {
        gESProvisioningCb.DevConfProvCb(eventData);
    }
    else
    {
        OIC_LOG_V(ERROR, ES_ENROLLEE_TAG, "DevConfProvCb is NULL");
        return;
    }
}

ESResult ESInitEnrollee(bool isSecured, ESResourceMask resourceMask, ESProvisioningCallbacks callbacks)
{
    OIC_LOG(INFO, ES_ENROLLEE_TAG, "ESInitEnrollee IN");

    gIsSecured = isSecured;

    if((resourceMask & ES_WIFI_RESOURCE) == ES_WIFI_RESOURCE)
    {
        if(callbacks.WiFiProvCb != NULL)
        {
            gESProvisioningCb.WiFiProvCb = callbacks.WiFiProvCb;
            RegisterWifiRsrcEventCallBack(ESWiFiRsrcCallback);
        }
        else
        {
            OIC_LOG(ERROR, ES_ENROLLEE_TAG, "WiFiProvCb NULL");
            return ES_ERROR;
        }
    }
    if((resourceMask & ES_DEVCONF_RESOURCE) == ES_DEVCONF_RESOURCE)
    {
        if(callbacks.DevConfProvCb != NULL)
        {
            gESProvisioningCb.DevConfProvCb = callbacks.DevConfProvCb;
            RegisterDevConfRsrcEventCallBack(ESDevconfRsrcallback);
        }
        else
        {
            OIC_LOG(ERROR, ES_ENROLLEE_TAG, "DevConfProvCb NULL");
            return ES_ERROR;
        }
    }
    if((resourceMask & ES_CLOUD_RESOURCE) == ES_CLOUD_RESOURCE)
    {
        if(callbacks.CloudDataProvCb != NULL)
        {
            gESProvisioningCb.CloudDataProvCb = callbacks.CloudDataProvCb;
            RegisterCloudRsrcEventCallBack(ESCloudRsrcCallback);
        }
        else
        {
            OIC_LOG(ERROR, ES_ENROLLEE_TAG, "CloudDataProvCb NULL");
            return ES_ERROR;
        }
    }

    if(CreateEasySetupResources(gIsSecured, resourceMask) != OC_STACK_OK)
    {
        UnRegisterResourceEventCallBack();

        if (DeleteEasySetupResources() != OC_STACK_OK)
        {
            OIC_LOG(ERROR, ES_ENROLLEE_TAG, "Deleting prov resource error!!");
        }

        return ES_ERROR;
    }


    OIC_LOG(INFO, ES_ENROLLEE_TAG, "ESInitEnrollee OUT");
    return ES_OK;
}

ESResult ESSetDeviceProperty(ESDeviceProperty *deviceProperty)
{
    OIC_LOG(INFO, ES_ENROLLEE_TAG, "ESSetDeviceProperty IN");

    if(SetDeviceProperty(deviceProperty) != OC_STACK_OK)
    {
        OIC_LOG(ERROR, ES_ENROLLEE_TAG, "ESSetDeviceProperty Error");
        return ES_ERROR;
    }

    int modeIdx = 0;
    while((deviceProperty->WiFi).mode[modeIdx] != WiFi_EOF)
    {
        (gESDeviceProperty.WiFi).mode[modeIdx] = (deviceProperty->WiFi).mode[modeIdx];
        OIC_LOG_V(INFO, ES_ENROLLEE_TAG, "WiFi Mode : %d", (gESDeviceProperty.WiFi).mode[modeIdx]);
        modeIdx ++;
    }
    (gESDeviceProperty.WiFi).freq = (deviceProperty->WiFi).freq;
    OIC_LOG_V(INFO, ES_ENROLLEE_TAG, "WiFi Freq : %d", (gESDeviceProperty.WiFi).freq);

    OICStrcpy((gESDeviceProperty.DevConf).deviceName, OIC_STRING_MAX_VALUE, (deviceProperty->DevConf).deviceName);
    OIC_LOG_V(INFO, ES_ENROLLEE_TAG, "Device Name : %s", (gESDeviceProperty.DevConf).deviceName);

    OIC_LOG(INFO, ES_ENROLLEE_TAG, "ESSetDeviceProperty OUT");
    return ES_OK;
}

ESResult ESSetState(ESEnrolleeState esState)
{
    OIC_LOG(INFO, ES_ENROLLEE_TAG, "ESSetState IN");

    if(esState < ES_STATE_INIT || esState >= ES_STATE_EOF)
    {
        OIC_LOG_V(ERROR, ES_ENROLLEE_TAG, "Invalid ESEnrolleeState : %d", esState);
        return ES_ERROR;
    }

    if(SetEnrolleeState(esState) != OC_STACK_OK)
    {
        OIC_LOG(ERROR, ES_ENROLLEE_TAG, "ESSetState ES_ERROR");
        return ES_ERROR;
    }

    OIC_LOG_V(INFO, ES_ENROLLEE_TAG, "Set ESState succesfully : %d", esState);
    OIC_LOG(INFO, ES_ENROLLEE_TAG, "ESSetState OUT");
    return ES_OK;
}

ESResult ESSetErrorCode(ESErrorCode esErrCode)
{
    OIC_LOG(INFO, ES_ENROLLEE_TAG, "ESSetErrorCode IN");

    if(esErrCode < ES_ERRCODE_NO_ERROR || esErrCode > ES_ERRCODE_UNKNOWN)
    {
        OIC_LOG_V(ERROR, ES_ENROLLEE_TAG, "Invalid ESSetErrorCode : %d", esErrCode);
            return ES_ERROR;
    }

    if(SetEnrolleeErrCode(esErrCode) != OC_STACK_OK)
    {
        OIC_LOG(ERROR, ES_ENROLLEE_TAG, "ESSetErrorCode ES_ERROR");
        return ES_ERROR;
    }

    OIC_LOG_V(DEBUG, ES_ENROLLEE_TAG, "Set ESErrorCode succesfully : %d", esErrCode);
    OIC_LOG(INFO, ES_ENROLLEE_TAG, "ESSetErrorCode OUT");
    return ES_OK;
}

ESResult ESTerminateEnrollee()
{
    OIC_LOG(INFO, ES_ENROLLEE_TAG, "ESTerminateEnrollee IN");

    UnRegisterResourceEventCallBack();

    //Delete Prov resource
    if (DeleteEasySetupResources() != OC_STACK_OK)
    {
        OIC_LOG(ERROR, ES_ENROLLEE_TAG, "Deleting prov resource error!!");
        return ES_ERROR;
    }

    OIC_LOG(ERROR, ES_ENROLLEE_TAG, "ESTerminateEnrollee success");
    return ES_OK;
}

ESResult ESSetCallbackForUserdata(ESReadUserdataCb readCb, ESWriteUserdataCb writeCb)
{
    if(!readCb && !writeCb)
    {
        OIC_LOG(INFO, ES_ENROLLEE_TAG, "Both of callbacks for user data are null");
        return ES_ERROR;
    }

    SetCallbackForUserData(readCb, writeCb);
    return ES_OK;
}