#ifndef TASK_2_H
#define TASK_2_H

#include <stdint.h>

typedef enum
{
    TASK2_RESULT_RUNNING = 0,

    TASK2_RESULT_FINISHED
} Task2RunResult;

void Task2_Enter(void);
Task2RunResult Task2_Run(uint32_t current_tick);
void Task2_Exit(void);
void Task2_Draw(void);



#endif