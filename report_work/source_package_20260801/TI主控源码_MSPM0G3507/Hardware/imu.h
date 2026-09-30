#ifndef IMU_H
#define IMU_H

#include <stdint.h>
#include <stdbool.h>

typedef struct
{
    int16_t ax_mg;
    int16_t ay_mg;
    int16_t az_mg;
	
		float gx_dps;
		float gy_dps;
		float gz_dps;
	
	  float angle_x_deg;
    float angle_y_deg;
    float angle_z_deg;
	
		bool angle_initialized;
	
} IMU_Data;

typedef enum
{
    IMU_UPDATE_NO_DATA = 0,
    IMU_UPDATE_OK,
    IMU_UPDATE_ERROR
} IMU_UpdateResult;

typedef struct
{
    uint32_t data_ready_count;
    uint32_t processed_count;
    uint32_t missed_count;
    uint32_t read_error_count;
} IMU_RuntimeStats;



bool IMU_Init(void);

bool IMU_Read(IMU_Data *data);

bool IMU_ReadAverage(
    IMU_Data *data,
    uint16_t sample_count);

//让 IMU 连续读取 sample_count 次，,计算三轴陀螺仪平均原始值，并保存为零偏。
bool IMU_CalibrateGyro(uint16_t sample_count);

void IMU_NotifyDataReadyFromISR(void);

IMU_UpdateResult IMU_Update(IMU_Data *data);

void IMU_GetRuntimeStats(IMU_RuntimeStats *stats);

void IMU_UpdateGyroAngle(IMU_Data *data, float dt_s);

void IMU_UpdateComplementaryAngle(IMU_Data *data, float dt_s);


/*IMU_Process()：有新中断数据时完成I2C读取和角度更新。
IMU_GetLatestData()：复制一份最新数据给调用者。
IMU_GetLatestSequence()：告诉任务数据更新到了第几次*/

IMU_UpdateResult IMU_Process(void);

bool IMU_GetLatestData(IMU_Data *data);

uint32_t IMU_GetLatestSequence(void);

#endif
