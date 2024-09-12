#include "nvm.h"
#include "linefollower_config.h"
#include "scp.h"
#include "pid.h"
#include "sensors.h"
#include "tb6612_motor.h"
#include "lf_calibrate.h"
#include "linefollower_commands.h"

Nvm_Instance_T NvmInstance;
NVM_Layout_T NVM_Block;
SCP_Instance_T ScpInstance;
PID_Instance_T PidSensorInstance;
Sensor_Instance_T Sensors[SENSORS_NUMBER];

extern const Sensor_Led_T sensorLeds[SENSORS_NUMBER];

static bool LF_IsDataUpdated = false;

void Linefollower_DataUpdateCallback(void)
{
    LF_IsDataUpdated = true;
    LF_CalibrateSensors();
}

void Linefollower_Init(void)
{
    NvmInstance = (Nvm_Instance_T){.data = (uint8_t *)&NVM_Block,
                                   .defaultData = (const uint8_t *)&NvmDefaultData,
                                   .size = sizeof(NVM_Layout_T),
                                   .sector = NVM_SECTOR_USED};
    (void)NVM_Init(&NvmInstance);
    (void)NVM_Read(&NvmInstance);

    PidSensorInstance = (PID_Instance_T){.settings = &NVM_Block.pidStgSensor};
    (void)PID_Init(&PidSensorInstance);

    static uint8_t scpBuffer[SCP_BUFFER_SIZE];
    ScpInstance = (SCP_Instance_T){.buffer = scpBuffer,
                                   .size = SCP_BUFFER_SIZE,
                                   .huart = &huart4,
                                   .commands = lineFollowerCommands,
                                   .numCommands = sizeof(lineFollowerCommands) / sizeof(lineFollowerCommands[0]),
                                   .errorHandler = NULL};
    (void)SCP_Init(&ScpInstance);


    for (uint16_t i = 0U; i < SENSORS_NUMBER; i++)
    {
        Sensors[i].positionWeight = NVM_Block.sensorWeights[i];
        Sensors[i].isActive = false;
    }
    (void)Sensors_Init(&hadc1, sensorLeds, Sensors, Linefollower_DataUpdateCallback);

    (void)TB6612Motor_Init(&LeftMotor);
    (void)TB6612Motor_Init(&RightMotor);

    LF_StartCalibration();
}

void Linefollower_Main(void)
{
    if (LF_IsDataUpdated)
    {
        float error = Sensors_CalculateError(&NVM_Block);
        int16_t output = PID_Update(&PidSensorInstance, error, 1.0);

        TB6612Motor_SetSpeed(&LeftMotor, 100U - output);
        TB6612Motor_SetSpeed(&RightMotor, 100U + output);

        Sensors_UpdateLeds();
        LF_IsDataUpdated = false;
    }
}
