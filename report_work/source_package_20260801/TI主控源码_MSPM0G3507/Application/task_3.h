#ifndef TASK_3_H
#define TASK_3_H

#include <stdint.h>

typedef enum
{
    TASK3_RESULT_RUNNING = 0,

    TASK3_RESULT_FINISHED
} Task3RunResult;

void Task3_Enter(void);
Task3RunResult Task3_Run(uint32_t current_tick);
void Task3_Exit(void);
void Task3_Draw(void);



#endif