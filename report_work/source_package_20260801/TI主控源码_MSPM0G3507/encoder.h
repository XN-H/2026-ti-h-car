#ifndef ENCODER_H
#define ENCODER_H

#include <stdint.h>
#include <stdbool.h>

//这两个变量在别的 .c 文件里定义；include encoder.h，谁就可以使用它们。
extern volatile int encoderA_count;
extern volatile int encoderB_count;

extern volatile int speed_A;
extern volatile int speed_B;

extern volatile int total_A;
extern volatile int total_B;

extern volatile uint32_t system_tick_10ms;


void GROUP1_IRQHandler(void);
void TIMER_0_INST_IRQHandler (void);

bool Encoder_TakeSpeedUpdate(void);



#endif
