#include "pid.h"

static inline float PID_CalculateIntegralTerm(const PID_T *const pid, const float error, const float dt)
{
    float integral = pid->integral + error * dt;

    if (integral > pid->settings.integral_max)
    {
        integral = pid->settings.integral_max;
    }
    else if (integral < pid->settings.integral_min)
    {
        integral = pid->settings.integral_min;
    }

    return integral;
}

static inline float PID_CalculateDerivativeTerm(const PID_T *const pid, const float error, const float dt)
{
    return (error - pid->error_previous) / dt;
}

static inline float PID_CalculateProportionalTerm(const PID_T *const pid, const float error)
{
    return pid->settings.kp * error;
}

static inline float PID_LimitOutput(const PID_T *const pid, const float output)
{
    if (output > pid->settings.output_max)
    {
        return pid->settings.output_max;
    }
    else if (output < pid->settings.output_min)
    {
        return pid->settings.output_min;
    }

    return output;
}

void PID_Init(PID_T *const pid, const PID_Settings_T *const settings)
{
    pid->settings = *settings;
    pid->setpoint = 0.0f;
    pid->error_previous = 0.0f;
    pid->integral = 0.0f;
}

float PID_Update(PID_T *const pid, const float measured, const float dt)
{
    float derivative, proportional;
    float output;
    float error = pid->setpoint - measured;

    pid->integral = PID_CalculateIntegralTerm(pid, error, dt);
    derivative = PID_CalculateDerivativeTerm(pid, error, dt);
    proportional = PID_CalculateProportionalTerm(pid, error);

    output = proportional + pid->settings.ki * pid->integral + pid->settings.kd * derivative;
    output = PID_LimitOutput(pid, output);

    pid->error_previous = error;

    return output;
}
