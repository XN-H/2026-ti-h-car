#include "imu_uart_link.h"
#include "ti_msp_dl_config.h"
#include <stdio.h>

void IMU_UART_SendAy(int16_t ay_mg)
{
    char frame[12];
    int length;
    int i;

    length = snprintf(
        frame,
        sizeof(frame),
        "%d\n",
        (int)ay_mg);

    if (length <= 0)
    {
        return;
    }

    for (i = 0; i < length; i++)
    {
        DL_UART_Main_transmitDataBlocking(
            UART_OPENMV_INST,
            (uint8_t)frame[i]);
    }
}