#ifndef LINE_FOLLOW_H
#define LINE_FOLLOW_H

#include <stdbool.h>
#include <stdint.h>

typedef struct
{
    int right_speed;
    int left_speed;
} LineFollowOutput;

typedef struct
{
    int kp;
    int kd;
    int max_correction;

    int16_t last_error;
    bool derivative_ready;
} LineFollowController;

void LineFollow_Init(
    LineFollowController *controller,
    int kp,
    int kd,
    int max_correction);

void LineFollow_Update(
    LineFollowController *controller,
    int16_t error,
    bool line_found,
    int base_speed,
    LineFollowOutput *output);

#endif