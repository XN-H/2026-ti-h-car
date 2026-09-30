#include "ti_msp_dl_config.h"
#include "motion.h"


#define MOTOR_A_DEADZONE 412
#define MOTOR_B_DEADZONE 435

//速度保护
static int Limit_PWM(int value)
{
    if (value > 7999) return 7999;
    if (value < -7999) return -7999;
    return value;
}

//死区修正
static int Apply_Deadzone(int speed, int deadzone)
{
    if (speed > 0)
    {
        speed += deadzone;
    }
    else if (speed < 0)
    {
        speed -= deadzone;
    }

    return Limit_PWM(speed);
}

// 右轮速度控制
void SpeedSetA(int speed)
{
	speed=Limit_PWM(speed);
	speed = Apply_Deadzone(speed, MOTOR_A_DEADZONE);
	//右轮前进
	if(speed>0)
	{
		DL_GPIO_clearPins(AIN_PORT,AIN_AIN2_PIN);
		DL_GPIO_setPins(AIN_PORT,AIN_AIN1_PIN);
		DL_Timer_setCaptureCompareValue(PWM_0_INST,speed,GPIO_PWM_0_C0_IDX);

	}
	//右轮后退
	else if(speed<0)
	{
		DL_GPIO_clearPins(AIN_PORT,AIN_AIN1_PIN);
		DL_GPIO_setPins(AIN_PORT,AIN_AIN2_PIN);
		DL_Timer_setCaptureCompareValue(PWM_0_INST,-speed,GPIO_PWM_0_C0_IDX);
		
	}
	else
	{
		DL_GPIO_clearPins(AIN_PORT,AIN_AIN2_PIN);
		DL_GPIO_clearPins(AIN_PORT,AIN_AIN1_PIN);
		DL_Timer_setCaptureCompareValue(PWM_0_INST,0,GPIO_PWM_0_C0_IDX);
	
	}

}
//右轮停止
void SpeedStopA(void)
{
		DL_GPIO_clearPins(AIN_PORT,AIN_AIN2_PIN);
		DL_GPIO_clearPins(AIN_PORT,AIN_AIN1_PIN);
		DL_Timer_setCaptureCompareValue(PWM_0_INST,0,GPIO_PWM_0_C0_IDX);

}

void SpeedBrakeA(void)
{
    /*
     * TB6612：
     * AIN1=1、AIN2=1时进入短路制动。
     */
    DL_Timer_setCaptureCompareValue(
        PWM_0_INST,
        0,
        GPIO_PWM_0_C0_IDX);

    DL_GPIO_setPins(
        AIN_PORT,
        AIN_AIN1_PIN |
        AIN_AIN2_PIN);
}


// 左轮速度控制
void SpeedSetB(int speed)
{
	speed=Limit_PWM(speed);
	speed = Apply_Deadzone(speed, MOTOR_B_DEADZONE);
	//左轮前进
	if(speed>0)
	{
		DL_GPIO_clearPins(BIN_PORT,BIN_BIN1_PIN);
		DL_GPIO_setPins(BIN_PORT,BIN_BIN2_PIN);
		DL_Timer_setCaptureCompareValue(PWM_0_INST,speed,GPIO_PWM_0_C1_IDX);

	}
	//左轮后退
	else if(speed<0)
	{
		DL_GPIO_clearPins(BIN_PORT,BIN_BIN2_PIN);
		DL_GPIO_setPins(BIN_PORT,BIN_BIN1_PIN);
		DL_Timer_setCaptureCompareValue(PWM_0_INST,-speed,GPIO_PWM_0_C1_IDX);
		
	}
	else
	{
		DL_GPIO_clearPins(BIN_PORT,BIN_BIN2_PIN);
		DL_GPIO_clearPins(BIN_PORT,BIN_BIN1_PIN);
		DL_Timer_setCaptureCompareValue(PWM_0_INST,0,GPIO_PWM_0_C1_IDX);
	
	}

}
//左轮停止
void SpeedStopB(void)
{
		DL_GPIO_clearPins(BIN_PORT,BIN_BIN1_PIN);
		DL_GPIO_clearPins(BIN_PORT,BIN_BIN2_PIN);
		DL_Timer_setCaptureCompareValue(PWM_0_INST,0,GPIO_PWM_0_C1_IDX);

}

void SpeedBrakeB(void)
{
    DL_Timer_setCaptureCompareValue(
        PWM_0_INST,
        0,
        GPIO_PWM_0_C1_IDX);

    DL_GPIO_setPins(
        BIN_PORT,
        BIN_BIN1_PIN |
        BIN_BIN2_PIN);
}


//前进函数
void CarForward(int speed)
{
		SpeedSetA(speed);
	  SpeedSetB(speed);

}
//后退函数
void CarBack(int speed)
{
		SpeedSetA(-speed);
	  SpeedSetB(-speed);

}
//左转弯函数
void CarLeft(int speed)
{
		SpeedSetA(speed);
	  SpeedSetB(100);

}
//右转弯函数
void CarRight(int speed)
{
		SpeedSetB(speed);
	  SpeedSetA(100);

}
//小车停止函数
void CarStop(void)
{
	SpeedStopA();
	SpeedStopB();

}
void CarBrake(void)
{
    SpeedBrakeA();
    SpeedBrakeB();
}