#ifndef PID_H
#define PID_H


void PID_Speed_A(int target_speed);
void PID_Speed_B(int target_speed);
void CarSpeedPIDSet(int right_speed,int left_speed);
void CarStraightPIDSet(int right_speed,int left_speed);

#endif
