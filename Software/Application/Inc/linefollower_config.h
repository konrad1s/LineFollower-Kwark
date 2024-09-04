#ifndef __LINEFOLLOWER_CONFIG_H__
#define __LINEFOLLOWER_CONFIG_H__

#include "pid.h"
#include "scp.h"

#define NVM_SECTOR_USED  FLASH_SECTOR_7
#define SCP_BUFFER_SIZE  128U

typedef struct
{
    PID_Settings_T pidStgSensor;
    PID_Settings_T pidStgMotorLeft;
    PID_Settings_T pidStgMotorRight;
} NVM_Layout_T;

extern const NVM_Layout_T NvmDefaultData;
extern const SCP_Config_T ScpConfig;

#endif /* __LINEFOLLOWER_CONFIG_H__ */
