#ifndef TRACKING_H
#define TRACKING_H

#include <stdbool.h>
#include <stdint.h>

#define TRACK_SENSOR_COUNT 6U

typedef struct
{
    /*
     * S2～S7用于正常循迹和位置计算。
     */
    uint8_t level[TRACK_SENSOR_COUNT];

    /*
     * S8只作为终点辅助检测，不参与循迹误差计算。
     */
    uint8_t s8_level;
} TrackingRaw;

void Tracking_ReadRaw(TrackingRaw *raw);

bool Tracking_CalculateError(
    const TrackingRaw *raw,
    int16_t *error,
    uint8_t *active_count);

#endif