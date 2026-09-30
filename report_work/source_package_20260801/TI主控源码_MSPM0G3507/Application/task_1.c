#include "task_1.h"
#include "tracking.h"
#include "line_follow.h"
#include "encoder.h"
#include "encoder_pid.h"
#include "oled.h"

#include "imu.h"
#include "angle_turn.h"
#include <math.h>
#include "motion.h"

//电赛任务一开始

#define TASK1_START_CLEAR_SAMPLES     3U
#define TASK1_FINISH_CONFIRM_SAMPLES  3U
#define TASK1_MIN_RUN_TICKS         300U
#define TASK1_MARKER_MIN_ACTIVE       3U

#define TASK1_RUN_SPEED                  20

#define TASK1_CD_ARM_TICKS          400U

#define TASK1_CD_STABLE_SAMPLES      20U
#define TASK1_CD_MAX_GZ_DPS          5.0f

#define TASK1_BRAKE_SETTLE_TICKS       20U
#define TASK1_CD_TO_A_ANGLE_DEG      180.0f
#define TASK1_TURN_IGNORE_DEG          2.0f

#define TASK1_TURN_EXTRA_DEG 5.0f

static uint32_t brake_start_tick = 0U;
static float final_turn_angle_deg = 0.0f;


static uint8_t cd_stable_count = 0U;


static bool cd_heading_valid = false;

static float cd_heading_deg = 0.0f;
static IMU_Data task1_imu = {0};






/*
 * S8触发后，在300ms内等待456或567。
 * system_tick_10ms每次代表10ms。
 */
#define TASK1_S8_WINDOW_TICKS 10U

typedef enum
{
    TASK1_STATE_FOLLOW = 0,
    TASK1_STATE_BRAKE_SETTLE,
    TASK1_STATE_TURNING
} Task1State;

/*
 * 记录上一次S8状态，用于检测0->1的上升过程。
 */
static bool s8_last_black = false;

/*
 * S8触发后的短时间窗口是否打开。
 */
static bool s8_window_open = false;

/*
 * S8触发时的10ms系统节拍。
 */
static uint32_t s8_trigger_tick = 0U;

static bool finish_detection_armed = false;
static uint8_t start_clear_count = 0U;
static uint8_t finish_confirm_count = 0U;
static uint32_t task1_start_tick = 0U;

static TrackingRaw track_raw = {0};
static int16_t track_error = 0;
static uint8_t track_active_count = 0U;
static bool track_line_found = false;

static LineFollowController line_controller = {0};
static LineFollowOutput line_output = {0};

static int line_base_speed =
    TASK1_RUN_SPEED;
static uint32_t last_tracking_tick = 0U;

static Task1State task1_state =
    TASK1_STATE_FOLLOW;





static void Task1_ShowText(
    uint8_t x,
    uint8_t y,
    const char *text)
{
    OLED_ShowString(
        x, y, (const uint8_t *)text);
}
void Task1_Enter(void)
	
{
	

	
	

	
		task1_state = TASK1_STATE_FOLLOW;
		line_base_speed = TASK1_RUN_SPEED;
	
	

		cd_stable_count = 0U;


		cd_heading_valid = false;

		cd_heading_deg = 0.0f;
		task1_imu = (IMU_Data){0};
		brake_start_tick = 0U;
		final_turn_angle_deg = 0.0f;

		AngleTurn_Cancel();


	
		finish_detection_armed = false;
		start_clear_count = 0U;
		finish_confirm_count = 0U;
		task1_start_tick = system_tick_10ms;
    track_raw = (TrackingRaw){0};
    track_error = 0;
    track_active_count = 0U;
    track_line_found = false;
		
		s8_last_black = false;
		s8_window_open = false;
		s8_trigger_tick = 0U;

    line_output.right_speed = 0;
    line_output.left_speed = 0;

    last_tracking_tick = system_tick_10ms;

    LineFollow_Init(
        &line_controller,
        5,
        2,
        10);

    CarSpeedPIDSet(0, 0);
}


static float Task1_WrapAngle180(float angle_deg)
{
    while (angle_deg > 180.0f)
    {
        angle_deg -= 360.0f;
    }

    while (angle_deg < -180.0f)
    {
        angle_deg += 360.0f;
    }

    return angle_deg;
}

static void Task1_StartBrakeSettle(
    uint32_t current_tick)
{
    task1_state = TASK1_STATE_BRAKE_SETTLE;
    brake_start_tick = current_tick;

    line_output.right_speed = 0;
    line_output.left_speed = 0;

    CarSpeedPIDSet(0, 0);
    CarBrake();
}



static void Task1_UpdateCdHeading(
    uint32_t current_tick)
{
    /*
     * 保存成功后不再更新CD航向。
     */
    if (cd_heading_valid)
    {
        return;
    }

    /*
     * 运行未满4秒时，不寻找CD直线。
     */
    if ((uint32_t)(current_tick -
                   task1_start_tick) <
        TASK1_CD_ARM_TICKS)
    {
        cd_stable_count = 0U;
        return;
    }

    /*
     * 获取IMU模块已经积分好的最新数据。
     */
    if (!IMU_GetLatestData(&task1_imu))
    {
        cd_stable_count = 0U;
        return;
    }

    /*
     * Z轴角速度小，说明车正在接近直线行驶。
     */
    if (fabsf(task1_imu.gz_dps) <=
        TASK1_CD_MAX_GZ_DPS)
    {
        if (cd_stable_count <
            TASK1_CD_STABLE_SAMPLES)
        {
            cd_stable_count++;
        }
    }
    else
    {
        cd_stable_count = 0U;
    }

    /*
     * 连续20次成立，即稳定200ms，保存CD航向。
     */
    if (cd_stable_count >=
        TASK1_CD_STABLE_SAMPLES)
    {
        cd_heading_deg =
            task1_imu.angle_z_deg;

        cd_heading_valid = true;
    }
}





static bool Task1_IsMarkerPattern(void)
{
    bool s4_black =
        track_raw.level[2] != 0U;

    bool s5_black =
        track_raw.level[3] != 0U;

    bool s6_black =
        track_raw.level[4] != 0U;

    bool s7_black =
        track_raw.level[5] != 0U;

    return
        (s4_black && s5_black && s6_black) ||
        (s5_black && s6_black && s7_black);
}

static bool Task1_CheckFinish(uint32_t current_tick)
{
    bool marker_pattern =
        Task1_IsMarkerPattern();

    bool s8_black =
        track_raw.s8_level != 0U;

    bool s8_rising =
        s8_black && !s8_last_black;

    /*
     * 保存本次S8状态，供下一次判断使用。
     */
    s8_last_black = s8_black;

    /*
     * 还没有离开起点时，不允许保存S8触发状态。
     */
    if (!finish_detection_armed)
    {
        s8_window_open = false;
        finish_confirm_count = 0U;

        if (!marker_pattern)
        {
            if (start_clear_count <
                TASK1_START_CLEAR_SAMPLES)
            {
                start_clear_count++;
            }

            if (start_clear_count >=
                TASK1_START_CLEAR_SAMPLES)
            {
                finish_detection_armed = true;
            }
        }
        else
        {
            start_clear_count = 0U;
        }

        return false;
    }

    /*
     * 启动后的最短保护时间内，也不允许打开窗口。
     */
    if ((uint32_t)(current_tick - task1_start_tick) <
        TASK1_MIN_RUN_TICKS)
    {
        s8_window_open = false;
        finish_confirm_count = 0U;
        return false;
    }

    /*
     * 检测到S8从白变黑，打开300ms窗口。
     */
    if (s8_rising)
    {
        s8_window_open = true;
        s8_trigger_tick = current_tick;
        finish_confirm_count = 0U;
    }

    /*
     * S8还没有触发，不进行三路横线判断。
     */
    if (!s8_window_open)
    {
        finish_confirm_count = 0U;
        return false;
    }

    /*
     * S8触发后超过300ms，仍未找到横线：
     * 认为这次S8是弯道或干扰，关闭窗口。
     */
    if ((uint32_t)(current_tick - s8_trigger_tick) >
        TASK1_S8_WINDOW_TICKS)
    {
        s8_window_open = false;
        finish_confirm_count = 0U;
        return false;
    }

    /*
     * 窗口内检测456或567，并进行连续采样确认。
     */
    if (marker_pattern)
    {
        if (finish_confirm_count <
            TASK1_FINISH_CONFIRM_SAMPLES)
        {
            finish_confirm_count++;
        }
    }
    else
    {
        finish_confirm_count = 0U;
    }

    return finish_confirm_count >=
           TASK1_FINISH_CONFIRM_SAMPLES;
}







bool Task1_Run(uint32_t current_tick)
{
    if ((uint32_t)(
            current_tick -
            last_tracking_tick) >= 1U)
    {
        last_tracking_tick = current_tick;

        Tracking_ReadRaw(&track_raw);

        track_line_found =
            Tracking_CalculateError(
                &track_raw,
                &track_error,
                &track_active_count);
			
			
			    /*
         * 每读取一次新的传感器数据，
         * 只进行一次终点判断。
         */
			if (task1_state == TASK1_STATE_FOLLOW)
			{
					Task1_UpdateCdHeading(current_tick);

					if (Task1_CheckFinish(current_tick))
					{
							Task1_StartBrakeSettle(current_tick);
					}
					else
					{
							LineFollow_Update(
									&line_controller,
									track_error,
									track_line_found,
									line_base_speed,
									&line_output);
					}
			}
			else if (task1_state == TASK1_STATE_BRAKE_SETTLE)
			{
					if ((uint32_t)(current_tick - brake_start_tick) >=
							TASK1_BRAKE_SETTLE_TICKS)
					{
							if (!cd_heading_valid ||
									!IMU_GetLatestData(&task1_imu))
							{
									return true;
							}

							final_turn_angle_deg =
									Task1_WrapAngle180(
											cd_heading_deg +
											TASK1_CD_TO_A_ANGLE_DEG -
											task1_imu.angle_z_deg);

							if (fabsf(final_turn_angle_deg) <=
									TASK1_TURN_IGNORE_DEG)
							{
									return true;
							}
							/*
						 * Increase the correction magnitude while keeping its direction.
						 */
						if (final_turn_angle_deg > 0.0f)
						{
								final_turn_angle_deg +=
										TASK1_TURN_EXTRA_DEG;
						}
						else
						{
								final_turn_angle_deg -=
										TASK1_TURN_EXTRA_DEG;
						}

						final_turn_angle_deg =
								Task1_WrapAngle180(
										final_turn_angle_deg);

							if (AngleTurn_Start(final_turn_angle_deg))
							{
									task1_state = TASK1_STATE_TURNING;
							}
							else
							{
									return true;
							}
					}
			}
			else if (task1_state == TASK1_STATE_TURNING)
			{
					AngleTurn_Update(current_tick);

					if (AngleTurn_IsFinished())
					{
							return true;
					}
			}

    }

			if (Encoder_TakeSpeedUpdate())
			{
					if (task1_state == TASK1_STATE_FOLLOW)
					{
							CarSpeedPIDSet(
									line_output.right_speed,
									line_output.left_speed);
					}
			}

		return false;

}
void Task1_Exit(void)
{
	
		AngleTurn_Cancel();
    line_output.right_speed = 0;
    line_output.left_speed = 0;

    CarSpeedPIDSet(0, 0);
}

void Task1_Draw(void)
{
    uint8_t i;



		Task1_ShowText(0, 16, "S:");

		for (i = 0U; i < TRACK_SENSOR_COUNT; i++)
		{
				OLED_ShowChar(
						16U + i * 8U,
						16,
						track_raw.level[i] ? '1' : '0',
						12,
						1);
		}
		OLED_ShowChar(
    64,
    16,
    track_raw.s8_level ? '1' : '0',
    12,
    1);

		Task1_ShowText(80, 16, "N:");
		OLED_ShowNumber(
				96, 16, track_active_count, 1, 12);

    Task1_ShowText(0, 32, "R:");
    OLED_ShowNumber(
        16, 32, line_output.right_speed, 3, 12);

    Task1_ShowText(64, 32, "L:");
    OLED_ShowNumber(
        80, 32, line_output.left_speed, 3, 12);
Task1_ShowText(48, 48, "ST:");
OLED_ShowNumber(72, 48, task1_state, 1, 12);
}