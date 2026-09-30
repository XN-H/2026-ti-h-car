#ifndef MPU6050_H
#define MPU6050_H

#include <stdint.h>
#include <stdbool.h>

#define MPU6050_ADDR             0x68U  // I2C 从机地址
#define MPU6050_WHO_AM_I_VALUE   0x68U  // 身份寄存器正确值
#define MPU6050_REG_WHO_AM_I 0x75U


#define MPU6050_PWR_MGMT_1 0x6BU
#define MPU6050_PWR_WAKE_UP 0x00U

#define MPU6050_PWR_SLEEP_MASK    0x40U

#define MPU6050_REG_ACCEL_XOUT_H  0x3BU
#define MPU6050_RAW_DATA_LEN      14U

//设置mpu6050寄存器中加速度量程为+-G
#define MPU6050_REG_ACCEL_CONFIG  0x1CU
#define MPU6050_ACCEL_FS_MASK     0x18U
#define MPU6050_ACCEL_FS_2G       0x00U

//±2g 模式下，原始值每变化 16384，代表变化 1g
#define MPU6050_ACCEL_LSB_PER_G  16384
//1g 等于 1000mg
#define MPU6050_MG_PER_G         1000

//mpu6050陀螺仪参数
#define MPU6050_REG_GYRO_CONFIG     0x1BU
#define MPU6050_GYRO_FS_MASK        0x18U

#define MPU6050_GYRO_FS_250DPS      0x00U
#define MPU6050_GYRO_FS_500DPS      0x08U
#define MPU6050_GYRO_FS_1000DPS     0x10U
#define MPU6050_GYRO_FS_2000DPS     0x18U

/* 当前实际使用的量程，只需要修改这里 */
#define MPU6050_GYRO_FS_SETTING     MPU6050_GYRO_FS_500DPS

#if MPU6050_GYRO_FS_SETTING == MPU6050_GYRO_FS_250DPS

#define MPU6050_GYRO_LSB_PER_DPS 131.0f

#elif MPU6050_GYRO_FS_SETTING == MPU6050_GYRO_FS_500DPS

#define MPU6050_GYRO_LSB_PER_DPS 65.5f

#elif MPU6050_GYRO_FS_SETTING == MPU6050_GYRO_FS_1000DPS

#define MPU6050_GYRO_LSB_PER_DPS 32.8f

#elif MPU6050_GYRO_FS_SETTING == MPU6050_GYRO_FS_2000DPS

#define MPU6050_GYRO_LSB_PER_DPS 16.4f

#else

#error "Unsupported MPU6050 gyro range"

#endif

//mpu6050安全配置寄存器
#define MPU6050_REG_SMPLRT_DIV  0x19U
#define MPU6050_REG_CONFIG      0x1AU

#define MPU6050_DLPF_42HZ       0x03U
#define MPU6050_DIV_200HZ       0x04U
#define MPU6050_DIV_100HZ 			0x09U


/*0x80 = 1000 0000，bit7置1，复位芯片
0x01 = 0000 0001，退出睡眠并选择X轴陀螺仪PLL
0x47 = 0100 0111，只检查睡眠位和时钟源位*/
#define MPU6050_PWR_DEVICE_RESET  0x80U
#define MPU6050_PWR_CLOCK_PLL_X   0x01U
#define MPU6050_PWR_CHECK_MASK    0x47U

#define MPU6050_REG_INT_PIN_CFG  0x37U
#define MPU6050_REG_INT_ENABLE   0x38U
#define MPU6050_REG_INT_STATUS   0x3AU

#define MPU6050_INT_DATA_READY   0x01U
#define MPU6050_INT_PIN_DEFAULT  0x00U


extern uint8_t MPU6050_ConfigReadback;
extern uint8_t MPU6050_DividerReadback;






//mpu6050的数据结构体
typedef struct
{
    int16_t ax;
    int16_t ay;
    int16_t az;

    int16_t temperature;

    int16_t gx;
    int16_t gy;
    int16_t gz;
} MPU6050_RawData;

bool MPU6050_ReadBytes(uint8_t start_reg,
                       uint8_t *buffer,
                       uint8_t length);




//读取寄存器reg中的值,并赋值到指定value地址上,最终检验返回bool值判断是否成功
bool MPU6050_ReadReg(uint8_t reg,uint8_t *value);

bool MPU6050_WriteReg(uint8_t reg,uint8_t value);

bool MPU6050_Init(void);

//数据读取函数
bool MPU6050_ReadRawData(MPU6050_RawData *data);

//加速度换算函数
int16_t MPU6050_AccelRawToMg(int16_t raw_value);

////连续读取多组原始数据并计算平均值
bool MPU6050_ReadAverageRawData(
    MPU6050_RawData *average_data,
    uint16_t sample_count);

//mpu6050的Z轴校准函数
int16_t MPU6050_AccelCorrectMg(
    int16_t measured_mg,
    int16_t offset_mg,
    int16_t one_g_span_mg);

//mpu6050陀螺仪数据获取
float MPU6050_GyroRawToDps(int16_t raw_value);
//INIT调试索引
extern uint8_t MPU6050_InitStage;

//检查陀螺仪寄存器的掩码
extern uint8_t MPU6050_GyroConfigReadback;

//mpu6050写入函数
bool MPU6050_WriteBytes(
    uint8_t reg,
    const uint8_t *data,
    uint8_t length);



#endif
