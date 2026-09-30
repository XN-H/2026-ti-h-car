#include "task_3.h"
#include "tracking.h"
#include "line_follow.h"
#include "encoder.h"
#include "encoder_pid.h"
#include "oled.h"




#define TASK3_CRUISE_SPEED             10
#define TASK3_ACCEL_PERIOD_TICKS       20U
#define TASK3_DECEL_PERIOD_TICKS       20U

#define TASK3_DECEL_DISTANCE_COUNTS 12000

typedef enum
{
    TASK3_STATE_ACCEL = 0,
    TASK3_STATE_CRUISE,
    TASK3_STATE_DECEL
} Task3State;




static Task3State task3_state = TASK3_STATE_ACCEL;

static TrackingRaw task3_track_raw = {0};
static int16_t task3_track_error = 0;
static uint8_t task3_active_count = 0U;
static bool task3_line_found = false;

static LineFollowController task3_line_controller = {0};
static LineFollowOutput task3_line_output = {0};

static int task3_base_speed = 0;
static uint32_t task3_last_tracking_tick = 0U;
static uint32_t task3_last_ramp_tick = 0U;

static int32_t task3_start_total_a = 0;
static int32_t task3_start_total_b = 0;
static int32_t task3_travel_counts = 0;

static void Task3_LimitLineOutput(void)
{
    int max_target_speed = task3_base_speed * 2;

    if (task3_line_output.right_speed < 0)
    {
        task3_line_output.right_speed = 0;
    }
    else if (task3_line_output.right_speed > max_target_speed)
    {
        task3_line_output.right_speed = max_target_speed;
    }

    if (task3_line_output.left_speed < 0)
    {
        task3_line_output.left_speed = 0;
    }
    else if (task3_line_output.left_speed > max_target_speed)
    {
        task3_line_output.left_speed = max_target_speed;
    }
}

static void Task3_UpdateTravelCounts(void)
{
    int32_t right_counts =
        (int32_t)total_A - task3_start_total_a;

    int32_t left_counts =
        (int32_t)total_B - task3_start_total_b;

    if (right_counts < 0)
    {
        right_counts = 0;
    }

    if (left_counts < 0)
    {
        left_counts = 0;
    }

    task3_travel_counts =
        (right_counts + left_counts) / 2;
}

//测试代码


//测试结束区





void Task3_Enter(void)
{

    task3_state = TASK3_STATE_ACCEL;
    task3_base_speed = 0;

    task3_track_raw = (TrackingRaw){0};
    task3_track_error = 0;
    task3_active_count = 0U;
    task3_line_found = false;

    task3_line_output.right_speed = 0;
    task3_line_output.left_speed = 0;

    task3_start_total_a = (int32_t)total_A;
    task3_start_total_b = (int32_t)total_B;
    task3_travel_counts = 0;

    task3_last_tracking_tick = system_tick_10ms;
    task3_last_ramp_tick = system_tick_10ms;

    LineFollow_Init(
        &task3_line_controller,
        4,
        2,
        10);

    CarSpeedPIDSet(0, 0);
}

Task3RunResult Task3_Run(uint32_t current_tick)
{
    Task3RunResult task3_result = TASK3_RESULT_RUNNING;

    /*
     * 每次执行Task3_Run，都更新从起点开始的编码器距离。
     * 如果没有这一句，task3_travel_counts永远是0。
     */
    Task3_UpdateTravelCounts();

		/* 到达计时位置：冻结成绩，但车辆继续运行。 */


		/* 到达减速位置后才开始减速。 */
		if ((task3_state == TASK3_STATE_CRUISE) &&
				(task3_travel_counts >=
				 TASK3_DECEL_DISTANCE_COUNTS))
		{
				task3_state = TASK3_STATE_DECEL;
				task3_last_ramp_tick = current_tick;
		}

		/* 加速：每100ms，目标速度加1 */
		if ((task3_state == TASK3_STATE_ACCEL) &&
				((uint32_t)(current_tick - task3_last_ramp_tick) >=
				 TASK3_ACCEL_PERIOD_TICKS))
		{
				task3_last_ramp_tick = current_tick;
				task3_base_speed++;

				if (task3_base_speed >= TASK3_CRUISE_SPEED)
				{
						task3_base_speed = TASK3_CRUISE_SPEED;
						task3_state = TASK3_STATE_CRUISE;
				}
		}
		/* 减速：每100ms，目标速度减1 */
		else if ((task3_state == TASK3_STATE_DECEL) &&
						 ((uint32_t)(current_tick - task3_last_ramp_tick) >=
							TASK3_DECEL_PERIOD_TICKS))
		{
				task3_last_ramp_tick = current_tick;

				if (task3_base_speed > 0)
				{
						task3_base_speed--;
				}

				if (task3_base_speed == 0)
				{
						task3_line_output.right_speed = 0;
						task3_line_output.left_speed = 0;

						return TASK3_RESULT_FINISHED;
				}
		}

    if ((uint32_t)(
            current_tick -
            task3_last_tracking_tick) >= 1U)
    {
        task3_last_tracking_tick = current_tick;

        Tracking_ReadRaw(&task3_track_raw);

        task3_line_found =
            Tracking_CalculateError(
                &task3_track_raw,
                &task3_track_error,
                &task3_active_count);

        LineFollow_Update(
            &task3_line_controller,
            task3_track_error,
            task3_line_found,
            task3_base_speed,
            &task3_line_output);
			
			Task3_LimitLineOutput();
    }

    if (Encoder_TakeSpeedUpdate())
    {
        CarSpeedPIDSet(
            task3_line_output.right_speed,
            task3_line_output.left_speed);
    }

    return task3_result;
}

void Task3_Exit(void)
{
    task3_base_speed = 0;
    task3_line_output.right_speed = 0;
    task3_line_output.left_speed = 0;

    CarSpeedPIDSet(0, 0);
}

void Task3_Draw(void)
{
    OLED_ShowString(0, 16, (const uint8_t *)"S:");
    OLED_ShowNumber(16, 16, task3_state, 1, 12);

    OLED_ShowString(40, 16, (const uint8_t *)"V:");
    OLED_ShowNumber(56, 16, task3_base_speed, 2, 12);

    OLED_ShowString(0, 32, (const uint8_t *)"D:");
    OLED_ShowNumber(
        16, 32,
        (uint32_t)task3_travel_counts,
        5, 12);

    OLED_ShowString(0, 48, (const uint8_t *)"A:");
    OLED_ShowNumber(16, 48, speed_A, 3, 12);

    OLED_ShowString(64, 48, (const uint8_t *)"B:");
    OLED_ShowNumber(80, 48, speed_B, 3, 12);
}





