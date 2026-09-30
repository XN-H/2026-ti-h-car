#include "ti_msp_dl_config.h"
#include "encoder.h"
#include "encoder_pid.h"
#include "motion.h"



#define KP_A 100
#define KI_A 5


#define KP_B 100
#define KI_B 5

//速度保护函数
static int Limit_Speed(int value,int min,int max)
{
	if(value>max) value=max;
	if(value<min) value=min;
	return value;
	
}

//车轮A的PI控制
static int error_Speed_A=0;

static int pwm_A=0;
static int last_error_Speed_A=0;




void PID_Speed_A(int target_speed)
{
	   if (target_speed == 0)
    {
        pwm_A = 0;
        error_Speed_A = 0;
        last_error_Speed_A = 0;
        SpeedStopA();
        return;
    }
	error_Speed_A=target_speed-speed_A;
	pwm_A+=KI_A*error_Speed_A+KP_A*(error_Speed_A-last_error_Speed_A);
	
	pwm_A=Limit_Speed(pwm_A,0,7000);
	SpeedSetA(pwm_A);
	last_error_Speed_A=error_Speed_A;
	

}
//车轮B的PI控制
static int error_Speed_B=0;

static int pwm_B=0;
static int last_error_Speed_B=0;




void PID_Speed_B(int target_speed)
{
	   if (target_speed == 0)
    {
        pwm_B = 0;
        error_Speed_B = 0;
        last_error_Speed_B = 0;
        SpeedStopB();
        return;
    }
	error_Speed_B=target_speed-speed_B;
	pwm_B+=KI_B*error_Speed_B+KP_B*(error_Speed_B-last_error_Speed_B);
	
	pwm_B=Limit_Speed(pwm_B,0,7000);
	SpeedSetB(pwm_B);
	last_error_Speed_B=error_Speed_B;
	

}

//两轮速度控制 闭环
void CarSpeedPIDSet(int right_speed,int left_speed)
{
	PID_Speed_A(right_speed);
	PID_Speed_B(left_speed);

}

//直行速度闭环
#define K_straight 2
#define MAX_CORR 10
void CarStraightPIDSet(int right_speed,int left_speed)
{
	int correction_Straight=(total_A-total_B)/K_straight;
	correction_Straight=Limit_Speed(correction_Straight,-MAX_CORR,
	MAX_CORR);
	PID_Speed_A(right_speed-correction_Straight);
	PID_Speed_B(left_speed+correction_Straight);

}