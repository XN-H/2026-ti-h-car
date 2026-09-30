/*
 * Copyright (c) 2021, Texas Instruments Incorporated
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * *  Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * *  Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * *  Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "ti_msp_dl_config.h"
#include "motion.h"
#include "encoder.h"
#include <stdio.h>
#include "encoder_pid.h"
#include "oled.h"
#include <string.h>
#include "board.h"
#include "mpu6050.h"
#include "imu.h"
#include "tracking.h"
#include "line_follow.h"

#include "task_manager.h"
#include "app_config.h"
#include "angle_turn.h"
#include "imu_uart_link.h"


/* printf retarget support for the Arm C library. */
#if !defined(__MICROLIB)
#if (__ARMCLIB_VERSION <= 6000000)
struct __FILE
{
    int handle;
};
#endif

FILE __stdout;

void _sys_exit(int x)
{
    (void)x;
}
#endif

int fputc(int ch, FILE *stream)
{
    (void)stream;

    while (DL_UART_isBusy(UART_0_INST) == true)
    {
    }

    DL_UART_Main_transmitData(
        UART_0_INST,
        (uint8_t)ch);

    return ch;
}

static void OLED_ShowText(
    uint8_t x,
    uint8_t y,
    const char *text)
{
    OLED_ShowString(
        x,
        y,
        (const uint8_t *)text);
}




//主程序
int main(void)
{
	
	SYSCFG_DL_init();
	
	//OLED初始化
	OLED_Init();
	OLED_Clear();
	OLED_ShowText(0, 0, "OLED BOOT");
	OLED_Refresh_Gram();


	
	//启用GPIOA中断
NVIC_ClearPendingIRQ(GPIO_MULTIPLE_GPIOA_INT_IRQN);
NVIC_EnableIRQ(GPIO_MULTIPLE_GPIOA_INT_IRQN);
	//启用编码器B中断
	NVIC_ClearPendingIRQ(ENCODERB_INT_IRQN );
	NVIC_EnableIRQ(ENCODERB_INT_IRQN);
	//启用TIM_0定时器中断开始测量小车速度
	NVIC_ClearPendingIRQ(TIMER_0_INST_INT_IRQN);
	NVIC_EnableIRQ(TIMER_0_INST_INT_IRQN);
	







// 通过IMU层初始化MPU6050
bool imu_init_ok = IMU_Init();

OLED_Clear();
OLED_ShowText(0, 0, "IMU DONE");
OLED_Refresh_Gram();

bool imu_gyro_cal_ok = false;
IMU_UpdateResult imu_update_result = IMU_UPDATE_NO_DATA;
IMU_UpdateResult imu_last_update = IMU_UPDATE_NO_DATA;
IMU_RuntimeStats imu_stats = {0};

if (imu_init_ok)
{
    OLED_Clear();
    OLED_ShowText(0, 0, "KEEP STILL");
    OLED_ShowText(0, 16, "GYRO CAL...");
    OLED_Refresh_Gram();

    delay_ms(1000);

    imu_gyro_cal_ok = IMU_CalibrateGyro(500);
		OLED_Clear();
		OLED_ShowText(0, 0, "CAL DONE");
		OLED_Refresh_Gram();
}


uint32_t current_tick;
uint32_t last_oled_tick= system_tick_10ms;
//角度转向初始化
AngleTurn_Init();

//任务树
TaskManager_Init();

uint32_t last_task_manager_tick =
    system_tick_10ms;

bool task1_running = false;
bool previous_task1_running = false;

	

IMU_Data imu_uart_data = {0};

while (1)
{
    /*
     * 读取当前10ms系统节拍。
     * system_tick_10ms在定时器中断中每10ms加1。
     */
    current_tick = system_tick_10ms;
		if ((uint32_t)(
						current_tick -
						last_task_manager_tick) >=
						APP_KEY_PERIOD_TICKS)
		{
				last_task_manager_tick = current_tick;
				TaskManager_Update10ms(current_tick);
		}




    /* IMU_Update只在PA7报告新数据后进行I2C读取。 */
    if (imu_init_ok && imu_gyro_cal_ok)
    {
        imu_update_result = IMU_Process();

        if (imu_update_result != IMU_UPDATE_NO_DATA)
        {
            imu_last_update = imu_update_result;
        }
				if (imu_update_result == IMU_UPDATE_OK)
				{
							if (IMU_GetLatestData(&imu_uart_data))
							{
									IMU_UART_SendAy(imu_uart_data.ay_mg);
							}
				}
    }
		
		TaskManager_Run(current_tick);

    /*
     * 两次OLED刷新之间经过10个节拍，
     * 也就是10 × 10ms = 100ms。
     */
			if ((uint32_t)(
							current_tick -
							last_oled_tick) >=
							APP_DISPLAY_PERIOD_TICKS)
			{
					last_oled_tick = current_tick;
					TaskManager_Draw100ms();
			}
}

		
		}
