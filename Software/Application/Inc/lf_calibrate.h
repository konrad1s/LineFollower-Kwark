#ifndef __LF_CALIBRATE_H__
#define __LF_CALIBRATE_H__

typedef enum
{
    LF_CALIBRATION_IN_PROGRESS,
    LF_CALIBRATION_COMPLETE
} LF_CalibrationStatus_T;

void LF_StartCalibration(void);
LF_CalibrationStatus_T LF_CalibrateSensors(void);

#endif /* __LF_CALIBRATE_H__ */
