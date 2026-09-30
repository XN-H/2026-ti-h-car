#ifndef TASK_1_H
#define TASK_1_H

#include <stdint.h>
#include <stdbool.h>

void Task1_Enter(void);
bool Task1_Run(uint32_t current_tick);
void Task1_Exit(void);
void Task1_Draw(void);

#endif