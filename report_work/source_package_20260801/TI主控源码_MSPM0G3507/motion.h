#ifndef MOTION_H
#define MOTION_H



void SpeedSetA(int speed);
void SpeedStopA(void);

void SpeedSetB(int speed);
void SpeedStopB(void);


void CarForward(int speed);
void CarBack(int speed);
void CarLeft(int speed);
void CarRight(int speed);
void CarStop(void);

void SpeedBrakeA(void);
void SpeedBrakeB(void);
void CarBrake(void);

#endif
