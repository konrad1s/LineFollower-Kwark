#ifndef __LF_CALIBRATE_H__
#define __LF_CALIBRATE_H__

#include "lf_main.h"

typedef enum
{
    LF_CALIBRATION_IN_PROGRESS,
    LF_CALIBRATION_ERROR,
    LF_CALIBRATION_COMPLETE,
} LF_CalibrationStatus_T;

void LF_StartCalibration(LineFollower_T *const me);
void LF_UpdateCalibrationData(LineFollower_T *const me);
LF_CalibrationStatus_T LF_UpdateCalibrationTimer(LineFollower_T *const me);

#endif /* __LF_CALIBRATE_H__ */
