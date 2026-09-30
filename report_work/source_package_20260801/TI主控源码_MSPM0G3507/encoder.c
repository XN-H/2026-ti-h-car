#include "ti_msp_dl_config.h"
#include "encoder.h"
#include "imu.h"


	//初始化编码器计数
volatile int encoderA_count;
volatile int encoderB_count;

//初始化速度
volatile int speed_A;
volatile int speed_B;

//路程计量
volatile int total_A;
volatile int total_B;


static volatile uint32_t encoder_speed_sequence = 0U;




volatile uint32_t system_tick_10ms = 0;

void GROUP1_IRQHandler(void)
{
    uint32_t gpioA_status =
        DL_GPIO_getEnabledInterruptStatus(
            GPIOA,
            ENCODERA_E1A_PIN |
            ENCODERA_E1B_PIN |
            MPU_INT_INT_PIN);

    uint32_t gpioB_status =
        DL_GPIO_getEnabledInterruptStatus(
            GPIOB,
            ENCODERB_E2A_PIN |
            ENCODERB_E2B_PIN);

    if (gpioA_status & ENCODERA_E1A_PIN)
    {
        encoderA_count++;
    }

    if (gpioB_status & ENCODERB_E2A_PIN)
    {
        encoderB_count++;
    }

    if (gpioA_status & MPU_INT_INT_PIN)
    {
        IMU_NotifyDataReadyFromISR();
    }

    if (gpioA_status != 0U)
    {
        DL_GPIO_clearInterruptStatus(GPIOA, gpioA_status);
    }

    if (gpioB_status != 0U)
    {
        DL_GPIO_clearInterruptStatus(GPIOB, gpioB_status);
    }
}
//速度读取函数
void TIMER_0_INST_IRQHandler(void)
{
    static uint8_t encoder_divider = 0;

    uint32_t timer_status =
        DL_TimerA_getPendingInterrupt(TIMER_0_INST);

    if (timer_status == DL_TIMERA_IIDX_ZERO)
    {
        /* 每次加1，表示又过去了10ms */
        system_tick_10ms++;

        /* 每两次10ms中断，计算一次20ms编码器速度 */
        encoder_divider++;

        if (encoder_divider >= 2)
        {
            encoder_divider = 0;

            speed_A = encoderA_count;
            speed_B = encoderB_count;

            total_A += speed_A;
            total_B += speed_B;

            encoderA_count = 0;
            encoderB_count = 0;
					
						encoder_speed_sequence++;
        }
    }
}

bool Encoder_TakeSpeedUpdate(void)
{
    static uint32_t last_sequence = 0U;
    uint32_t current_sequence;

    current_sequence = encoder_speed_sequence;

    if (current_sequence == last_sequence)
    {
        return false;
    }

    last_sequence = current_sequence;

    return true;
}