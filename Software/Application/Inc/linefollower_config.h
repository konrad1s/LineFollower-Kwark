#ifndef __LINEFOLLOWER_CONFIG_H__
#define __LINEFOLLOWER_CONFIG_H__

/******************************************************************************************
 *                                        INCLUDES                                        *
 ******************************************************************************************/
#include <stdint.h>
#include <stdbool.h>
#include "pid.h"
#include "scp.h"
#include "tb6612_motor.h"
#include "encoder.h"

/******************************************************************************************
 *                                         DEFINES                                        *
 ******************************************************************************************/
#define NVM_SECTOR_USED  FLASH_SECTOR_2
#define SCP_BUFFER_SIZE  512U
#define SENSORS_NUMBER   (12U)

/******************************************************************************************
 *                                        TYPEDEFS                                        *
 ******************************************************************************************/
typedef enum
{
    LF_TIMER_NO_LINE_DETECTED,
    LF_TIMER_REDUCED_SPEED,
    LF_TIMER_SENSORS_STABILIZE,
    LF_TIMER_CALIBRATION,
    LF_TIMER_NB
} LF_TimetId_T;

typedef struct
{
    int8_t weights[SENSORS_NUMBER];
    uint16_t thresholds[SENSORS_NUMBER];
    float errorThreshold;
    float fallbackErrorPositive;
    float fallbackErrorNegative;
} NVM_Sensors_T;

typedef struct
{
    PID_Settings_T pidStgSensor;
    PID_Settings_T pidStgEncoderLeft;
    PID_Settings_T pidStgEncoderRight;
    NVM_Sensors_T sensors;
    float targetSpeed;
    uint32_t timerTimeout[LF_TIMER_NB];
} NVM_Layout_T;

/******************************************************************************************
 *                                    GLOBAL VARIABLES                                    *
 ******************************************************************************************/
extern const NVM_Layout_T NvmDefaultData;
extern const TB6612MotorDriver_T LeftMotor;
extern const TB6612MotorDriver_T RightMotor;
extern const Encoder_Settings_T encoderSettings;

/******************************************************************************************
 *                                   FUNCTION PROTOTYPES                                  *
 ******************************************************************************************/

#endif /* __LINEFOLLOWER_CONFIG_H__ */
