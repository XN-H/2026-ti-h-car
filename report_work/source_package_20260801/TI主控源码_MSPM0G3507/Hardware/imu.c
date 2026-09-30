#include "imu.h"
#include "mpu6050.h"

//编写互补滤波
#include <math.h>

#define IMU_RAD_TO_DEG              57.2957795f
#define IMU_COMPLEMENTARY_ALPHA     0.98f
#define IMU_SAMPLE_PERIOD_S         0.01f

//imuZ轴校准数据
#define IMU_Z_OFFSET_MG    169
#define IMU_Z_SPAN_MG      1025

static volatile uint32_t imu_data_ready_count = 0;
static uint32_t imu_last_data_ready_count = 0;
static uint32_t imu_processed_count = 0;
static uint32_t imu_missed_count = 0;
static uint32_t imu_read_error_count = 0;
static bool imu_timing_started = false;

static int16_t imu_gyro_bias_gx_raw = 0;
static int16_t imu_gyro_bias_gy_raw = 0;
static int16_t imu_gyro_bias_gz_raw = 0;

static IMU_Data imu_latest_data = {0};
static bool imu_latest_valid = false;
static uint32_t imu_latest_sequence = 0U;

bool IMU_Init(void)
{
    imu_data_ready_count = 0;
    imu_last_data_ready_count = 0;
    imu_processed_count = 0;
    imu_missed_count = 0;
    imu_read_error_count = 0;
		imu_timing_started = false;
		imu_latest_data = (IMU_Data){0};
		imu_latest_valid = false;
		imu_latest_sequence = 0U;

    return MPU6050_Init();
}

bool IMU_CalibrateGyro(uint16_t sample_count)
{
    MPU6050_RawData raw_average;

    if (!MPU6050_ReadAverageRawData(
            &raw_average,
            sample_count))
    {
        return false;
    }

    imu_gyro_bias_gx_raw = raw_average.gx;
    imu_gyro_bias_gy_raw = raw_average.gy;
    imu_gyro_bias_gz_raw = raw_average.gz;

    imu_last_data_ready_count = imu_data_ready_count;
    imu_timing_started = true;

    return true;
}


//实时采样六轴加速度 角速度数据
bool IMU_Read(IMU_Data *data)
{
    MPU6050_RawData raw_data;
    int16_t z_mg;

    if (data == 0)
    {
        return false;
    }

    if (!MPU6050_ReadRawData(&raw_data))
    {
        return false;
    }

    data->ax_mg =
        MPU6050_AccelRawToMg(raw_data.ax);

    data->ay_mg =
        MPU6050_AccelRawToMg(raw_data.ay);

    z_mg =
        MPU6050_AccelRawToMg(raw_data.az);

    data->az_mg =
        MPU6050_AccelCorrectMg(
            z_mg,
            IMU_Z_OFFSET_MG,
            IMU_Z_SPAN_MG);
		//陀螺仪角速度采集
		data->gx_dps =
				MPU6050_GyroRawToDps(
						raw_data.gx - imu_gyro_bias_gx_raw);

		data->gy_dps =
				MPU6050_GyroRawToDps(
						raw_data.gy - imu_gyro_bias_gy_raw);

		data->gz_dps =
				MPU6050_GyroRawToDps(
						raw_data.gz - imu_gyro_bias_gz_raw);

    return true;
}

bool IMU_ReadAverage(
    IMU_Data *data,
    uint16_t sample_count)
{
    MPU6050_RawData raw_average;
    int16_t z_mg;

    if (data == 0)
    {
        return false;
    }

    if (!MPU6050_ReadAverageRawData(
            &raw_average,
            sample_count))
    {
        return false;
    }

    data->ax_mg =
        MPU6050_AccelRawToMg(raw_average.ax);
    data->ay_mg =
        MPU6050_AccelRawToMg(raw_average.ay);

    z_mg = MPU6050_AccelRawToMg(raw_average.az);
    data->az_mg = MPU6050_AccelCorrectMg(
        z_mg,
        IMU_Z_OFFSET_MG,
        IMU_Z_SPAN_MG);

    data->gx_dps = MPU6050_GyroRawToDps(
        raw_average.gx - imu_gyro_bias_gx_raw);
    data->gy_dps = MPU6050_GyroRawToDps(
        raw_average.gy - imu_gyro_bias_gy_raw);
    data->gz_dps = MPU6050_GyroRawToDps(
        raw_average.gz - imu_gyro_bias_gz_raw);

    return true;
}

void IMU_NotifyDataReadyFromISR(void)
{
    imu_data_ready_count++;
}

IMU_UpdateResult IMU_Update(IMU_Data *data)
{
    uint32_t current_count;
    uint32_t elapsed_samples;
    uint8_t interrupt_status;

    if (data == 0)
    {
        imu_read_error_count++;
        return IMU_UPDATE_ERROR;
    }

    current_count = imu_data_ready_count;

    if (!imu_timing_started)
    {
        imu_last_data_ready_count = current_count;
        imu_timing_started = true;
        return IMU_UPDATE_NO_DATA;
    }

    elapsed_samples =
        current_count - imu_last_data_ready_count;

    if (elapsed_samples == 0U)
    {
        return IMU_UPDATE_NO_DATA;
    }

    imu_last_data_ready_count = current_count;

    if (elapsed_samples > 1U)
    {
        imu_missed_count += elapsed_samples - 1U;
    }

    if (!MPU6050_ReadReg(
            MPU6050_REG_INT_STATUS,
            &interrupt_status))
    {
        imu_read_error_count++;
        return IMU_UPDATE_ERROR;
    }

    if ((interrupt_status & MPU6050_INT_DATA_READY) == 0U)
    {
        imu_read_error_count++;
        return IMU_UPDATE_ERROR;
    }

    if (!IMU_Read(data))
    {
        imu_read_error_count++;
        return IMU_UPDATE_ERROR;
    }

    IMU_UpdateComplementaryAngle(
        data,
        (float)elapsed_samples * IMU_SAMPLE_PERIOD_S);

    imu_processed_count++;

    return IMU_UPDATE_OK;
}

void IMU_GetRuntimeStats(IMU_RuntimeStats *stats)
{
    if (stats == 0)
    {
        return;
    }

    stats->data_ready_count = imu_data_ready_count;
    stats->processed_count = imu_processed_count;
    stats->missed_count = imu_missed_count;
    stats->read_error_count = imu_read_error_count;
}

void IMU_UpdateGyroAngle(IMU_Data *data, float dt_s)
{
    if ((data == 0) || (dt_s <= 0.0f))
    {
        return;
    }

    data->angle_x_deg += data->gx_dps * dt_s;
    data->angle_y_deg += data->gy_dps * dt_s;
    data->angle_z_deg += data->gz_dps * dt_s;
}

//互补滤波
void IMU_UpdateComplementaryAngle(
    IMU_Data *data,
    float dt_s)
{
    float ax;
    float ay;
    float az;
    float accel_angle_x;
    float accel_angle_y;

    if ((data == 0) || (dt_s <= 0.0f))
    {
        return;
    }

    ax = (float)data->ax_mg;
    ay = (float)data->ay_mg;
    az = (float)data->az_mg;

    accel_angle_x =
        atan2f(ay, az) * IMU_RAD_TO_DEG;

    accel_angle_y =
        atan2f(-ax, sqrtf(ay * ay + az * az))
        * IMU_RAD_TO_DEG;

    if (!data->angle_initialized)
    {
        data->angle_x_deg = accel_angle_x;
        data->angle_y_deg = accel_angle_y;
        data->angle_z_deg = 0.0f;
        data->angle_initialized = true;
        return;
    }

    data->angle_x_deg =
        IMU_COMPLEMENTARY_ALPHA *
        (data->angle_x_deg + data->gx_dps * dt_s) +
        (1.0f - IMU_COMPLEMENTARY_ALPHA) * accel_angle_x;

    data->angle_y_deg =
        IMU_COMPLEMENTARY_ALPHA *
        (data->angle_y_deg + data->gy_dps * dt_s) +
        (1.0f - IMU_COMPLEMENTARY_ALPHA) * accel_angle_y;

    data->angle_z_deg += data->gz_dps * dt_s;
}

IMU_UpdateResult IMU_Process(void)
{
    IMU_UpdateResult result;

    result = IMU_Update(&imu_latest_data);

    if (result == IMU_UPDATE_OK)
    {
        imu_latest_valid = true;
        imu_latest_sequence++;
    }

    return result;
}

bool IMU_GetLatestData(IMU_Data *data)
{
    if ((data == 0) || !imu_latest_valid)
    {
        return false;
    }

    *data = imu_latest_data;
    return true;
}

uint32_t IMU_GetLatestSequence(void)
{
    return imu_latest_sequence;
}