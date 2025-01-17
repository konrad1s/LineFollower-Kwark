#ifndef __PID__H__
#define __PID__H__

/******************************************************************************************
 *                                        INCLUDES                                        *
 ******************************************************************************************/
#include <stdint.h>

/******************************************************************************************
 *                                         DEFINES                                        *
 ******************************************************************************************/

/******************************************************************************************
 *                                        TYPEDEFS                                        *
 ******************************************************************************************/
typedef struct
{
    float kp; /* Proportional gain */
    float ki; /* Integral gain */
    float kd; /* Derivative gain */

    float integral_max; /* Maximum value for integral term (anti-windup) */
    float integral_min; /* Minimum value for integral term (anti-windup) */
    float output_max;   /* Maximum value for output */
    float output_min;   /* Minimum value for output */
} PID_Settings_T;

typedef struct
{
    PID_Settings_T *settings;

    float integral;       /* Integral term */
    float setpoint;       /* Desired value */
    float error_previous; /* Error at previous step */
} PID_Instance_T;

/******************************************************************************************
 *                                    GLOBAL VARIABLES                                    *
 ******************************************************************************************/

/******************************************************************************************
 *                                   FUNCTION PROTOTYPES                                  *
 ******************************************************************************************/
int PID_Init(PID_Instance_T *const pid);
float PID_Update(PID_Instance_T *const pid, const float measured, const float dt);

#endif /* __PID__H__ */
