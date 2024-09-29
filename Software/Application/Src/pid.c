#include "pid.h"
#include <stddef.h>

/**
 * @brief Calculates the integral term for the PID controller.
 *
 * @param[in] pid Pointer to the PID instance.
 * @param[in] error The current error value.
 * @param[in] dt The time step between PID updates.
 *
 * @return The calculated integral term, clamped within the integral limits.
 */
static inline float PID_CalculateIntegralTerm(const PID_Instance_T *const pid, const float error, const float dt)
{
    float integral = pid->integral + error * dt;

    if (integral > pid->settings->integral_max)
    {
        integral = pid->settings->integral_max;
    }
    else if (integral < pid->settings->integral_min)
    {
        integral = pid->settings->integral_min;
    }

    return integral;
}

/**
 * @brief Calculates the derivative term for the PID controller.
 *
 * @param[in] pid Pointer to the PID instance.
 * @param[in] error The current error value.
 * @param[in] dt The time step between PID updates.
 *
 * @return The calculated derivative term.
 */
static inline float PID_CalculateDerivativeTerm(const PID_Instance_T *const pid, const float error, const float dt)
{
    return (error - pid->error_previous) / dt;
}

/**
 * @brief Calculates the proportional term for the PID controller.
 *
 * @param[in] pid Pointer to the PID instance.
 * @param[in] error The current error value.
 *
 * @return The calculated proportional term.
 */
static inline float PID_CalculateProportionalTerm(const PID_Instance_T *const pid, const float error)
{
    return pid->settings->kp * error;
}

/**
 * @brief Limits the output of the PID controller.
 *
 * @param[in] pid Pointer to the PID instance.
 * @param[in] output The calculated output before clamping.
 *
 * @return The clamped output, limited within the output bounds.
 */
static inline float PID_LimitOutput(const PID_Instance_T *const pid, const float output)
{
    if (output > pid->settings->output_max)
    {
        return pid->settings->output_max;
    }
    else if (output < pid->settings->output_min)
    {
        return pid->settings->output_min;
    }

    return output;
}

/**
 * @brief Initializes the PID controller instance.
 *
 * @param[in,out] pid Pointer to the PID instance to initialize.
 *
 * @return
 * - 0 on success.
 * - -1 on failure.
 */
int PID_Init(PID_Instance_T *const pid)
{
    if (pid == NULL)
    {
        return -1;
    }

    pid->integral = 0.0f;
    pid->error_previous = 0.0f;
    pid->setpoint = 0.0f;

    return 0;
}

/**
 * @brief Updates the PID controller with the latest measurement.
 *
 * @param[in,out] pid Pointer to the PID instance.
 * @param[in] measured The current measured value.
 * @param[in] dt The time step between PID updates.
 *
 * @return The PID controller's output after applying the control algorithm.
 */
float PID_Update(PID_Instance_T *const pid, const float measured, const float dt)
{
    float derivative, proportional;
    float output;
    float error = pid->setpoint - measured;

    pid->integral = PID_CalculateIntegralTerm(pid, error, dt);
    derivative = PID_CalculateDerivativeTerm(pid, error, dt);
    proportional = PID_CalculateProportionalTerm(pid, error);

    output = proportional + pid->settings->ki * pid->integral + pid->settings->kd * derivative;
    output = PID_LimitOutput(pid, output);

    pid->error_previous = error;

    return output;
}
