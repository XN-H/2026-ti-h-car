#include "line_follow.h"

#define LINE_FOLLOW_ERROR_SCALE 1000

static int LineFollow_Limit(
    int value,
    int limit)
{
    if (value > limit)
    {
        value = limit;
    }

    if (value < -limit)
    {
        value = -limit;
    }

    return value;
}

void LineFollow_Init(
    LineFollowController *controller,
    int kp,
    int kd,
    int max_correction)
{
    if (controller == 0)
    {
        return;
    }

    controller->kp = kp;
    controller->kd = kd;
    controller->max_correction = max_correction;

    controller->last_error = 0;
    controller->derivative_ready = false;
}

void LineFollow_Update(
    LineFollowController *controller,
    int16_t error,
    bool line_found,
    int base_speed,
    LineFollowOutput *output)
{
    int error_change = 0;
    int correction;

    if ((controller == 0) || (output == 0))
    {
        return;
    }

    if (!line_found)
    {
        output->right_speed = 0;
        output->left_speed = 0;

        controller->derivative_ready = false;
        return;
    }

    if (controller->derivative_ready)
    {
        error_change =
            (int)error -
            (int)controller->last_error;
    }

    correction =
        ((int32_t)controller->kp * error +
         (int32_t)controller->kd * error_change) /
        LINE_FOLLOW_ERROR_SCALE;

    correction = LineFollow_Limit(
        correction,
        controller->max_correction);

    output->right_speed =
        base_speed - correction;

    output->left_speed =
        base_speed + correction;

    controller->last_error = error;
    controller->derivative_ready = true;
}