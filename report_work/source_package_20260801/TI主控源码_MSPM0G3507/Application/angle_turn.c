#include "angle_turn.h"
#include "imu.h"
#include "motion.h"
#include <math.h>

#define ANGLE_TURN_TOLERANCE_DEG       2.0f
#define ANGLE_TURN_RATE_TOLERANCE_DPS  5.0f
#define ANGLE_TURN_SETTLE_SAMPLES      20U
#define ANGLE_TURN_TIMEOUT_TICKS       1000U

#define ANGLE_TURN_KP             10.0f
#define ANGLE_TURN_OUTPUT_MAX       900
#define ANGLE_TURN_OUTPUT_MIN        500

static AngleTurnState turn_state =
    ANGLE_TURN_IDLE;

static float target_angle_deg = 0.0f;
static float start_angle_deg = 0.0f;
static float current_angle_deg = 0.0f;
static float angle_error_deg = 0.0f;
static float turn_rate_dps = 0.0f;

static int turn_output = 0;
static uint32_t last_imu_sequence = 0U;
static IMU_Data turn_imu_data = {0};

static uint16_t settle_samples = 0U;
static uint32_t turn_start_tick = 0U;
static bool turn_timing_started = false;

static int AngleTurn_ApplyMinimumOutput(
    int output)
{
    if ((output > 0) &&
        (output < ANGLE_TURN_OUTPUT_MIN))
    {
        return ANGLE_TURN_OUTPUT_MIN;
    }

    if ((output < 0) &&
        (output > -ANGLE_TURN_OUTPUT_MIN))
    {
        return -ANGLE_TURN_OUTPUT_MIN;
    }

    return output;
}

static int AngleTurn_LimitOutput(int output)
{
    if (output > ANGLE_TURN_OUTPUT_MAX)
    {
        return ANGLE_TURN_OUTPUT_MAX;
    }

    if (output < -ANGLE_TURN_OUTPUT_MAX)
    {
        return -ANGLE_TURN_OUTPUT_MAX;
    }

    return output;
}

static void AngleTurn_ApplyOutput(int output)
{
    output =
        AngleTurn_ApplyMinimumOutput(output);

    turn_output =
        AngleTurn_LimitOutput(output);

    SpeedSetA(turn_output);
    SpeedSetB(-turn_output);
}

static void AngleTurn_ClearRuntime(void)
{
    target_angle_deg = 0.0f;
    start_angle_deg = 0.0f;
    current_angle_deg = 0.0f;
    angle_error_deg = 0.0f;
    turn_rate_dps = 0.0f;

    turn_output = 0;
    last_imu_sequence = 0U;
		turn_imu_data = (IMU_Data){0};

		settle_samples = 0U;
		turn_start_tick = 0U;
		turn_timing_started = false;
}

void AngleTurn_Init(void)
{
    AngleTurn_ClearRuntime();

    turn_state = ANGLE_TURN_IDLE;

    CarStop();
}

bool AngleTurn_IsBusy(void)
{
    return
        (turn_state == ANGLE_TURN_WAITING_IMU) ||
        (turn_state == ANGLE_TURN_RUNNING) ||
        (turn_state == ANGLE_TURN_SETTLING);
}

bool AngleTurn_Start(float requested_angle_deg)
{
    if (AngleTurn_IsBusy())
    {
        return false;
    }

    AngleTurn_ClearRuntime();

    target_angle_deg = requested_angle_deg;

    last_imu_sequence =
        IMU_GetLatestSequence();

    turn_state = ANGLE_TURN_WAITING_IMU;

    CarStop();

    return true;
}

void AngleTurn_Update(uint32_t current_tick)
{
    uint32_t current_sequence;

    if (!AngleTurn_IsBusy())
    {
        return;
    }

    if (!turn_timing_started)
    {
        turn_start_tick = current_tick;
        turn_timing_started = true;
    }

    if ((uint32_t)(current_tick - turn_start_tick) >=
        ANGLE_TURN_TIMEOUT_TICKS)
    {
				AngleTurn_ApplyOutput(0);
        turn_state = ANGLE_TURN_TIMEOUT;
        return;
    }

    current_sequence = IMU_GetLatestSequence();

    if (current_sequence == last_imu_sequence)
    {
        return;
    }

    last_imu_sequence = current_sequence;

    if (!IMU_GetLatestData(&turn_imu_data))
    {
        CarStop();
        turn_output = 0;
        turn_state = ANGLE_TURN_SENSOR_ERROR;
        return;
    }

    turn_rate_dps = turn_imu_data.gz_dps;

    if (turn_state == ANGLE_TURN_WAITING_IMU)
    {
        start_angle_deg =
            turn_imu_data.angle_z_deg;

        current_angle_deg = 0.0f;
        angle_error_deg = target_angle_deg;

        turn_state = ANGLE_TURN_RUNNING;
        return;
    }

    current_angle_deg =
        turn_imu_data.angle_z_deg -
        start_angle_deg;

    angle_error_deg =
        target_angle_deg -
        current_angle_deg;

		if (fabsf(angle_error_deg) <=
				ANGLE_TURN_TOLERANCE_DEG)
		{
				/*
				 * 已进入目标角度范围，停止继续驱动，
				 * 等待车身角速度降下来。
				 */
				AngleTurn_ApplyOutput(0);

				if (fabsf(turn_rate_dps) <=
						ANGLE_TURN_RATE_TOLERANCE_DPS)
				{
						turn_state =
								ANGLE_TURN_SETTLING;

						settle_samples++;

						if (settle_samples >=
								ANGLE_TURN_SETTLE_SAMPLES)
						{
								AngleTurn_ApplyOutput(0);
								turn_state =
										ANGLE_TURN_DONE;
						}
				}
				else
				{
						settle_samples = 0U;
						turn_state =
								ANGLE_TURN_RUNNING;
				}
		}
		
		else
		{
				settle_samples = 0U;
				turn_state = ANGLE_TURN_RUNNING;

				turn_output = (int)(
						ANGLE_TURN_KP *
						angle_error_deg);

				AngleTurn_ApplyOutput(turn_output);
		}
}

void AngleTurn_Cancel(void)
{
    AngleTurn_ApplyOutput(0);

    AngleTurn_ClearRuntime();
    turn_state = ANGLE_TURN_IDLE;
}

bool AngleTurn_IsFinished(void)
{
    return
        (turn_state == ANGLE_TURN_DONE) ||
        (turn_state == ANGLE_TURN_TIMEOUT) ||
        (turn_state == ANGLE_TURN_SENSOR_ERROR);
}

AngleTurnState AngleTurn_GetState(void)
{
    return turn_state;
}

float AngleTurn_GetTargetAngle(void)
{
    return target_angle_deg;
}

float AngleTurn_GetCurrentAngle(void)
{
    return current_angle_deg;
}

float AngleTurn_GetError(void)
{
    return angle_error_deg;
}

float AngleTurn_GetTurnRate(void)
{
    return turn_rate_dps;
}

int AngleTurn_GetOutput(void)
{
    return turn_output;
}