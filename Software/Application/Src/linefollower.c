#include "nvm.h"
#include "linefollower_config.h"
#include "scp.h"
#include "pid.h"
#include "sensors.h"
#include "tb6612_motor.h"

NVM_T NvmInstance;
SCP_Instance_T ScpInstance;
PID_T PidSensorInstance;

NVM_Layout_T NVM_Block;

void Linefollower_Init(void)
{
    NVM_Init(&NvmInstance, &NVM_Block, &NvmDefaultData, sizeof(NVM_Layout_T), NVM_SECTOR_USED);
    (void)NVM_Read(&NvmInstance);

    PID_Init(&PidSensorInstance, &NVM_Block.pidStgSensor);

    (void)SCP_Init(&ScpInstance, &ScpConfig);

    Sensors_Config_Init(&SensorsManager, &hadc1, NVM_Block.sensorWeights);
    Sensors_Config_Init(&hadc1, NVM_Block.sensorWeights);
    TB6612Motor_Init(&LeftMotor);
    TB6612Motor_Init(&RightMotor);
}
