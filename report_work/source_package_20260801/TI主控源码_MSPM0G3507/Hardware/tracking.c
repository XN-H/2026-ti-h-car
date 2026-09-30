#include "ti_msp_dl_config.h"
#include "tracking.h"

void Tracking_ReadRaw(TrackingRaw *raw)
{
    raw->level[0] =
        (DL_GPIO_readPins(TRACK_S2_PORT, TRACK_S2_PIN) != 0U);

    raw->level[1] =
        (DL_GPIO_readPins(TRACK_S3_PORT, TRACK_S3_PIN) != 0U);

    raw->level[2] =
        (DL_GPIO_readPins(TRACK_S4_PORT, TRACK_S4_PIN) != 0U);

    raw->level[3] =
        (DL_GPIO_readPins(TRACK_S5_PORT, TRACK_S5_PIN) != 0U);

    raw->level[4] =
        (DL_GPIO_readPins(TRACK_S6_PORT, TRACK_S6_PIN) != 0U);

    raw->level[5] =
        (DL_GPIO_readPins(TRACK_S7_PORT, TRACK_S7_PIN) != 0U);
	
		raw->s8_level =
    (DL_GPIO_readPins(
        TRACK_S8_PORT,
        TRACK_S8_PIN) != 0U);
}

bool Tracking_CalculateError(
    const TrackingRaw *raw,
    int16_t *error,
    uint8_t *active_count)
{
    static const int16_t sensor_position[TRACK_SENSOR_COUNT] =
    {
        -2500, -1500, -500, 500, 1500, 2500
    };

    int32_t weighted_sum = 0;
    uint8_t count = 0U;
    uint8_t i;

    if ((raw == 0) || (error == 0) || (active_count == 0))
    {
        return false;
    }

		//遍历所有传感器如果这一传感器检测到黑线，就执行以下误差计算
    for (i = 0U; i < TRACK_SENSOR_COUNT; i++)
    {
        if (raw->level[i] != 0U)
        {
            weighted_sum += sensor_position[i];
            count++;
        }
    }

    *active_count = count;

    if (count == 0U)
    {
        return false;
    }

    *error = (int16_t)(weighted_sum / (int32_t)count);

    return true;
}