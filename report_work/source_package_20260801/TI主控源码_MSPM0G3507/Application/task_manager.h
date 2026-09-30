#ifndef TASK_MANAGER_H
#define TASK_MANAGER_H

#include <stdint.h>

typedef enum
{
    APP_TASK_1 = 0,
    APP_TASK_2,
    APP_TASK_3,
    APP_TASK_4,
    APP_TASK_COUNT
} AppTaskId;

typedef enum
{
    APP_STATE_SELECT = 0,
    APP_STATE_RUNNING,
    APP_STATE_FINISHED
} AppState;

void TaskManager_Init(void);
void TaskManager_Update10ms(uint32_t current_tick);
void TaskManager_Run(uint32_t current_tick);
void TaskManager_Draw100ms(void);

AppTaskId TaskManager_GetSelectedTask(void);
AppState TaskManager_GetState(void);

#endif