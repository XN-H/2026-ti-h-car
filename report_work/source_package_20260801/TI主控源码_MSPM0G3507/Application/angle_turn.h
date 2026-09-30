#ifndef ANGLE_TURN_H
#define ANGLE_TURN_H

#include <stdint.h>
#include <stdbool.h>

typedef enum
{
    ANGLE_TURN_IDLE = 0,
    ANGLE_TURN_WAITING_IMU,
    ANGLE_TURN_RUNNING,
    ANGLE_TURN_SETTLING,
    ANGLE_TURN_DONE,
    ANGLE_TURN_TIMEOUT,
    ANGLE_TURN_SENSOR_ERROR
} AngleTurnState;

void AngleTurn_Init(void);

bool AngleTurn_Start(float target_angle_deg);

void AngleTurn_Update(uint32_t current_tick);

void AngleTurn_Cancel(void);

bool AngleTurn_IsBusy(void);
bool AngleTurn_IsFinished(void);

AngleTurnState AngleTurn_GetState(void);

float AngleTurn_GetTargetAngle(void);
float AngleTurn_GetCurrentAngle(void);
float AngleTurn_GetError(void);
float AngleTurn_GetTurnRate(void);
int AngleTurn_GetOutput(void);

#endif