#include "nvm.h"
#include "linefollower_config.h"
#include "scp.h"
#include "pid.h"

static NVM_T NvmInstance;
static SCP_Instance_T ScpInstance;
static PID_T PidSensorInstance;

static NVM_Layout_T NVM_Block;

void Linefollower_Init(void)
{
    NVM_Init(&NvmInstance, &NVM_Block, &NvmDefaultData, sizeof(NVM_Layout_T), NVM_SECTOR_USED);
    (void)NVM_Read(&NvmInstance);

    PID_Init(&PidSensorInstance, &NVM_Block.pidStgSensor);

    (void)SCP_Init(&ScpInstance, &ScpConfig);
}
