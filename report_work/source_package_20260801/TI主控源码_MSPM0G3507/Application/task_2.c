#include "task_2.h"
#include "tracking.h"
#include "line_follow.h"
#include "encoder.h"
#include "encoder_pid.h"
#include "oled.h"

#define TASK2_CRUISE_SPEED           15
#define TASK2_ACCEL_PERIOD_TICKS     35U
#define TASK2_DECEL_PERIOD_TICKS     35U

#define TASK2_DECEL_DISTANCE_COUNTS 4500

typedef enum
{
    TASK2_STATE_ACCEL = 0,
    TASK2_STATE_CRUISE,
    TASK2_STATE_DECEL
} Task2State;




static Task2State task2_state = TASK2_STATE_ACCEL;

static TrackingRaw task2_track_raw = {0};
static int16_t task2_track_error = 0;
static uint8_t task2_active_count = 0U;
static bool task2_line_found = false;

static LineFollowController task2_line_controller = {0};
static LineFollowOutput task2_line_output = {0};

static int task2_base_speed = 0;
static uint32_t task2_last_tracking_tick = 0U;
static uint32_t task2_last_ramp_tick = 0U;

static int32_t task2_start_total_a = 0;
static int32_t task2_start_total_b = 0;
static int32_t task2_travel_counts = 0;

static void Task2_LimitLineOutput(void)
{
    int max_target_speed = task2_base_speed * 2;

    if (task2_line_output.right_speed < 0)
    {
        task2_line_output.right_speed = 0;
    }
    else if (task2_line_output.right_speed > max_target_speed)
    {
        task2_line_output.right_speed = max_target_speed;
    }

    if (task2_line_output.left_speed < 0)
    {
        task2_line_output.left_speed = 0;
    }
    else if (task2_line_output.left_speed > max_target_speed)
    {
        task2_line_output.left_speed = max_target_speed;
    }
}

static void Task2_UpdateTravelCounts(void)
{
    int32_t right_counts =
        (int32_t)total_A - task2_start_total_a;

    int32_t left_counts =
        (int32_t)total_B - task2_start_total_b;

    if (right_counts < 0)
    {
        right_counts = 0;
    }

    if (left_counts < 0)
    {
        left_counts = 0;
    }

    task2_travel_counts =
        (right_counts + left_counts) / 2;
}

//测试代码


//测试结束区





void Task2_Enter(void)

{

    task2_state = TASK2_STATE_ACCEL;
    task2_base_speed = 0;

    task2_track_raw = (TrackingRaw){0};
    task2_track_error = 0;
    task2_active_count = 0U;
    task2_line_found = false;

    task2_line_output.right_speed = 0;
    task2_line_output.left_speed = 0;

    task2_start_total_a = (int32_t)total_A;
    task2_start_total_b = (int32_t)total_B;
    task2_travel_counts = 0;

    task2_last_tracking_tick = system_tick_10ms;
    task2_last_ramp_tick = system_tick_10ms;

    LineFollow_Init(
        &task2_line_controller,
        4,
        2,
        10);

    CarSpeedPIDSet(0, 0);
}

Task2RunResult Task2_Run(uint32_t current_tick)
{
    Task2_UpdateTravelCounts();

		Task2RunResult task2_result = TASK2_RESULT_RUNNING;

		/* 只在匀速状态判断是否到达B点 */
		/* 到达计时位置：只向TaskManager报告一次。 */


		/* 到达减速位置：开始逐渐减速，不停止计时事件。 */
		if ((task2_state != TASK2_STATE_DECEL) &&
				(task2_travel_counts >=
				 TASK2_DECEL_DISTANCE_COUNTS))
		{
				task2_state = TASK2_STATE_DECEL;
				task2_last_ramp_tick = current_tick;
		}
		
		if ((task2_state == TASK2_STATE_ACCEL) &&
    ((uint32_t)(current_tick - task2_last_ramp_tick) >=
     TASK2_ACCEL_PERIOD_TICKS))
		{
				task2_last_ramp_tick = current_tick;

				task2_base_speed++;

				if (task2_base_speed >= TASK2_CRUISE_SPEED)
				{
						task2_base_speed = TASK2_CRUISE_SPEED;
						task2_state = TASK2_STATE_CRUISE;
				}
		}
		else if ((task2_state == TASK2_STATE_DECEL) &&
         ((uint32_t)(current_tick - task2_last_ramp_tick) >=
          TASK2_DECEL_PERIOD_TICKS))
		{
				task2_last_ramp_tick = current_tick;

				if (task2_base_speed > 0)
				{
						task2_base_speed--;
				}

				if (task2_base_speed == 0)
				{
						task2_line_output.right_speed = 0;
						task2_line_output.left_speed = 0;

						return TASK2_RESULT_FINISHED;
				}
		}



    if ((uint32_t)(
            current_tick -
            task2_last_tracking_tick) >= 1U)
    {
        task2_last_tracking_tick = current_tick;

        Tracking_ReadRaw(&task2_track_raw);

        task2_line_found =
            Tracking_CalculateError(
                &task2_track_raw,
                &task2_track_error,
                &task2_active_count);

        LineFollow_Update(
            &task2_line_controller,
            task2_track_error,
            task2_line_found,
            task2_base_speed,
            &task2_line_output);
			
			Task2_LimitLineOutput();
    }

    if (Encoder_TakeSpeedUpdate())
    {
        CarSpeedPIDSet(
            task2_line_output.right_speed,
            task2_line_output.left_speed);
    }

    return task2_result;
}

void Task2_Exit(void)
{
    task2_base_speed = 0;
    task2_line_output.right_speed = 0;
    task2_line_output.left_speed = 0;

    CarSpeedPIDSet(0, 0);
}

void Task2_Draw(void)
{
    OLED_ShowString(0, 16, (const uint8_t *)"S:");
    OLED_ShowNumber(16, 16, task2_state, 1, 12);

    OLED_ShowString(40, 16, (const uint8_t *)"V:");
    OLED_ShowNumber(56, 16, task2_base_speed, 2, 12);

    OLED_ShowString(0, 32, (const uint8_t *)"D:");
    OLED_ShowNumber(
        16, 32,
        (uint32_t)task2_travel_counts,
        5, 12);

    OLED_ShowString(0, 48, (const uint8_t *)"A:");
    OLED_ShowNumber(16, 48, speed_A, 3, 12);

    OLED_ShowString(64, 48, (const uint8_t *)"B:");
    OLED_ShowNumber(80, 48, speed_B, 3, 12);
}





