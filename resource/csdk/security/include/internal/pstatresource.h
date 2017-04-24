//******************************************************************
//
// Copyright 2015 Intel Mobile Communications GmbH All Rights Reserved.
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

#ifndef IOTVT_SRM_PSTATR_H
#define IOTVT_SRM_PSTATR_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Initialize Pstat resource by loading data from persistent storage.
 *
 * @return ::OC_STACK_OK for Success, otherwise some error value.
 */
OCStackResult InitPstatResource();

/**
 * Perform cleanup for Pstat resources.
 *
 * @return ::OC_STACK_OK for Success, otherwise some error value.
 */
OCStackResult DeInitPstatResource();

/**
 * This method converts PSTAT into the cbor payload.
 *
 * @param pstat pointer to the initialized pstat structure.
 * @param cborPayload pointer to pstat cbor payload.
 * @param cborSize of the cbor payload converted. It is 0 in case of error,
 * else a positive value if succcessful.
 * @param writableOnly indicates whether it is writable only or not.
 *
 * @return ::OC_STACK_OK for Success, otherwise some error value.
 */
 OCStackResult PstatToCBORPayload(const OicSecPstat_t *pstat, uint8_t **cborPayload,
                                  size_t *cborSize, bool writableOnly);

/**
 * This method converts cbor into PSTAT data.
 *
 * @param cborPayload is the pstat data in cbor format.
 * @param cborSize of the cborPayload. In case 0 is provided it assigns CBOR_SIZE (255) value.
 * @param pstat pointer to @ref OicSecPstat_t.
  *
  * @return ::OC_STACK_OK for Success, otherwise some error value.
 */
 OCStackResult CBORPayloadToPstat(const uint8_t *cborPayload, const size_t cborSize,
                                  OicSecPstat_t **pstat);

/** This function deallocates the memory for OicSecPstat_t.
 *
 * @param pstat is the pointer to @ref OicSecPstat_t.
 */
void DeletePstatBinData(OicSecPstat_t* pstat);

/**
 * Function to restore pstat resurce to initial status.
 * This function will use in case of error while ownership transfer
 */
void RestorePstatToInitState();

/**
 * Get the pstat.rowneruuid value for this device.
 *
 * @param[out] rowneruuid ptr to contain a copy of pstat.rowneruuid value.
 * @return ::OC_STACK_OK if value is copied successfully, else ::OC_STACK_ERROR.
 */
OCStackResult GetPstatRownerId(OicUuid_t *rowneruuid);

/**
 * Set the pstat.rowneruuid value for this device.
 *
 * @param[in] rowneruuid ptr to value to be copied into pstat.rowneruuid.
 * @return ::OC_STACK_OK if value is copied successfully, else ::OC_STACK_ERROR.
 */
OCStackResult SetPstatRownerId(const OicUuid_t* rowneruuid);

/**
 * Get the pstat.dos.s value for this device.
 *
 * @param[out] s ptr to contain a copy of pstat.dos.s value.
 * @return ::OC_STACK_OK if value is copied successfully, else ::OC_STACK_ERROR.
 */
OCStackResult GetPstatDosS(OicSecDeviceOnboardingState_t *s);

/**
 * Set the pstat.dos.s value for this device.
 *
 * @param[in] s value to be copied into pstat.dos.s.
 * @return ::OC_STACK_OK if value is copied successfully, else ::OC_STACK_ERROR.
 */
OCStackResult SetPstatDosS(const OicSecDeviceOnboardingState_t s);

/**
 * Get the pstat.dos.p value for this device.
 *
 * @param[out] p ptr to contain a copy of pstat.dos.p value.
 * @return ::OC_STACK_OK if value is copied successfully, else ::OC_STACK_ERROR.
 */
OCStackResult GetPstatDosP(bool *p);

/**
 * Set the pstat.dos.p value for this device.
 *
 * @param[in] p value to be copied into pstat.dos.p.
 * @return ::OC_STACK_OK if value is copied successfully, else ::OC_STACK_ERROR.
 */
OCStackResult SetPstatDosP(const bool p);

/**
 * Get the pstat.isop value for this device.
 *
 * @param[out] isop ptr to contain a copy of pstat.isop value.
 * @return ::OC_STACK_OK if value is copied successfully, else ::OC_STACK_ERROR.
 */
OCStackResult GetPstatIsop(bool *isop);

/**
 * Set the pstat.isop value for this device.
 *
 * @param[in] isop value to be copied into pstat.isop.
 * @return ::OC_STACK_OK if value is copied successfully, else ::OC_STACK_ERROR.
 */
OCStackResult SetPstatIsop(const bool isop);

/**
 * Get the pstat.cm value for this device.
 *
 * @param[out] cm ptr to contain a copy of pstat.cm value.
 * @return ::OC_STACK_OK if value is copied successfully, else ::OC_STACK_ERROR.
 */
OCStackResult GetPstatCm(OicSecDpm_t *cm);

/**
 * Set the pstat.cm value for this device.
 *
 * @param[in] cm value to be copied into pstat.cm.
 * @return ::OC_STACK_OK if value is copied successfully, else ::OC_STACK_ERROR.
 */
OCStackResult SetPstatCm(const OicSecDpm_t cm);

/**
 * Get the pstat.tm value for this device.
 *
 * @param[out] tm ptr to contain a copy of pstat.tm value.
 * @return ::OC_STACK_OK if value is copied successfully, else ::OC_STACK_ERROR.
 */
OCStackResult GetPstatTm(OicSecDpm_t *tm);

/**
 * Set the pstat.tm value for this device.
 *
 * @param[in] tm value to be copied into pstat.tm.
 * @return ::OC_STACK_OK if value is copied successfully, else ::OC_STACK_ERROR.
 */
OCStackResult SetPstatTm(const OicSecDpm_t tm);

/**
 * Internal function to change pastat resource to Ready for Normal Operation.
 *
 * @param newROwner new owner
 *
 * @retval ::OC_STACK_OK for Success, otherwise some error value
 */
OCStackResult SetPstatSelfOwnership(const OicUuid_t* newROwner);

#ifdef __cplusplus
}
#endif

#endif //IOTVT_SRM_PSTATR_H
