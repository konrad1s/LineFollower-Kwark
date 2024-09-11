#include "nvm.h"
#include "linefollower_config.h"
#include "scp.h"
#include "pid.h"
#include "sensors.h"
#include "tb6612_motor.h"

NVM_T NvmInstance;
NVM_Layout_T NVM_Block;
SCP_Instance_T ScpInstance;
PID_T PidSensorInstance;

static bool LF_IsUpdateRequired = false;

void Linefollower_DataUpdateCallback(void)
{
    LF_IsUpdateRequired = true;
}

void Linefollower_Init(void)
{
    NVM_Init(&NvmInstance, &NVM_Block, &NvmDefaultData, sizeof(NVM_Layout_T), NVM_SECTOR_USED);
    (void)NVM_Read(&NvmInstance);

    PID_Init(&PidSensorInstance, &NVM_Block.pidStgSensor);

    (void)SCP_Init(&ScpInstance, &ScpConfig);

    Sensors_Config_Init(&hadc1, NVM_Block.sensorWeights, Linefollower_DataUpdateCallback);
    TB6612Motor_Init(&LeftMotor);
    TB6612Motor_Init(&RightMotor);
}

void Linefollower_Main(void)
{
    if (LF_IsUpdateRequired)
    {
        float error = Sensors_CalculateError(&NVM_Block);
        int16_t output = PID_Update(&PidSensorInstance, error, 1.0);

        TB6612Motor_SetSpeed(&LeftMotor, 100U - output);
        TB6612Motor_SetSpeed(&RightMotor, 100U + output);

        Sensors_UpdateLeds();
        LF_IsUpdateRequired = false;
    }
}