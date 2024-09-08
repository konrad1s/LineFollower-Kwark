#include "nvm.h"
#include "linefollower_config.h"
#include "scp.h"
#include "pid.h"
#include "sensors.h"

NVM_T NvmInstance;
SCP_Instance_T ScpInstance;
PID_T PidSensorInstance;
Sensors_Manager_T SensorsManager;

NVM_Layout_T NVM_Block;

void Linefollower_Init(void)
{
    NVM_Init(&NvmInstance, &NVM_Block, &NvmDefaultData, sizeof(NVM_Layout_T), NVM_SECTOR_USED);
    (void)NVM_Read(&NvmInstance);

    PID_Init(&PidSensorInstance, &NVM_Block.pidStgSensor);

    (void)SCP_Init(&ScpInstance, &ScpConfig);

    Sensors_Config_Init(&SensorsManager, &hadc1, NVM_Block.sensorWeights);
}
