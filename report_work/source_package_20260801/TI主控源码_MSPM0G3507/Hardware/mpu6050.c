#include "ti_msp_dl_config.h"
#include "mpu6050.h"
#include "board.h"

#define I2C_TIME_OUT 100000U


uint8_t MPU6050_InitStage = 0;
uint8_t MPU6050_GyroConfigReadback = 0;

uint8_t MPU6050_ConfigReadback = 0xFF;
uint8_t MPU6050_DividerReadback = 0xFF;


//高低字节拼接函数
static int16_t MPU6050_CombineBytes(uint8_t high_byte,
                                    uint8_t low_byte)
{
    uint16_t combined;

    combined = ((uint16_t)high_byte << 8) | low_byte;

    return (int16_t)combined;
}


//// 辅助函数：等待 I2C 总线空闲；static 表示仅 mpu6050.c 可调用
static bool MPU6050_I2CStatusJugement(void)
{
	uint32_t time_out=I2C_TIME_OUT;
	//当I2C繁忙时,进行超时判定
	while((DL_I2C_getControllerStatus(I2C_0_INST)&DL_I2C_CONTROLLER_STATUS_IDLE)==0)
	{
		time_out=time_out-1;
		if(time_out==0)
		{
			return false;
		}
	}



	return true;
}

//向指定寄存器reg写入data中length长度的字节
bool MPU6050_WriteBytes(
    uint8_t reg,
    const uint8_t *data,
    uint8_t length)
{
    uint8_t tx_buffer[8];
    uint8_t i;
    uint16_t total_length;
    uint16_t filled;
    uint32_t status;
    uint32_t time_out = I2C_TIME_OUT;

    /*
     * TX FIFO只有8字节。
     * 其中1字节保存寄存器地址，因此数据最多7字节。
     */
    if ((data == 0) || (length == 0) || (length > 7))
    {
        return false;
    }

    /*
     * 按I2C发送顺序准备完整数据包：
     * tx_buffer[0] 是寄存器地址；
     * 后面的元素是真正要写入的数据。
     */
    tx_buffer[0] = reg;

    for (i = 0; i < length; i++)
    {
        tx_buffer[i + 1] = data[i];
    }

    total_length = (uint16_t)length + 1U;

    /* 确认上一笔I2C通信已经结束 */
    if (!MPU6050_I2CStatusJugement())
    {
        return false;
    }

    /*
     * 关闭读取模式，并清理上一笔通信可能留下的数据。
     * 此时总线已经空闲，所以可以安全清理。
     */
    DL_I2C_resetControllerTransfer(I2C_0_INST);
    DL_I2C_flushControllerTXFIFO(I2C_0_INST);
    DL_I2C_flushControllerRXFIFO(I2C_0_INST);

    DL_I2C_clearInterruptStatus(
        I2C_0_INST,
        DL_I2C_INTERRUPT_CONTROLLER_TX_DONE |
        DL_I2C_INTERRUPT_CONTROLLER_NACK);

    /* 启动传输前，将完整数据包装入TX FIFO */
    filled = DL_I2C_fillControllerTXFIFO(
        I2C_0_INST,
        tx_buffer,
        total_length);

    if (filled != total_length)
    {
        DL_I2C_flushControllerTXFIFO(I2C_0_INST);
        return false;
    }

    /* FIFO准备完成后，才正式启动I2C发送 */
    DL_I2C_startControllerTransfer(
        I2C_0_INST,
        MPU6050_ADDR,
        DL_I2C_CONTROLLER_DIRECTION_TX,
        total_length);

				/* 等待整笔I2C通信结束 */
				/* 等待总线传输结束 */
				while ((DL_I2C_getControllerStatus(I2C_0_INST) &
								DL_I2C_CONTROLLER_STATUS_BUSY_BUS) != 0U)
				{
						if (time_out == 0U)
						{
								DL_I2C_flushControllerTXFIFO(I2C_0_INST);
								return false;
						}

						time_out--;
				}

				/* 初步检查通信错误 */
				status = DL_I2C_getControllerStatus(I2C_0_INST);

				if ((status & DL_I2C_CONTROLLER_STATUS_ERROR) != 0U)
				{
						DL_I2C_flushControllerTXFIFO(I2C_0_INST);
						return false;
				}

				/* 等待控制器完全进入空闲状态 */
				while ((DL_I2C_getControllerStatus(I2C_0_INST) &
								DL_I2C_CONTROLLER_STATUS_IDLE) == 0U)
				{
						status = DL_I2C_getControllerStatus(I2C_0_INST);

						if ((status & DL_I2C_CONTROLLER_STATUS_ERROR) != 0U)
						{
								DL_I2C_flushControllerTXFIFO(I2C_0_INST);
								return false;
						}

						if (time_out == 0U)
						{
								DL_I2C_flushControllerTXFIFO(I2C_0_INST);
								return false;
						}

						time_out--;
				}

				DL_I2C_flushControllerTXFIFO(I2C_0_INST);
				return true;
}

//向指定地址的寄存器reg接收一个字节存储在value里
bool MPU6050_ReadReg(uint8_t reg, uint8_t *value)
{
    if (value == 0)
    {
        return false;
    }

    return MPU6050_ReadBytes(
        reg,
        value,
        1U);
}

//向某个reg地址的寄存器写入一个字节
bool MPU6050_WriteReg(uint8_t reg, uint8_t value)
{
    return MPU6050_WriteBytes(
        reg,
        &value,
        1U);
}

//从指定地址的寄存器reg中接收length长度字节函数

bool MPU6050_ReadBytes(uint8_t start_reg,
                       uint8_t *buffer,
                       uint8_t length)
{
    uint8_t index = 0;
    uint32_t status;
    uint32_t time_out = I2C_TIME_OUT;

    if ((buffer == 0) || (length == 0))
    {
        return false;
    }

    if (!MPU6050_I2CStatusJugement())
    {
        return false;
    }
		DL_I2C_resetControllerTransfer(I2C_0_INST);

		DL_I2C_flushControllerTXFIFO(I2C_0_INST);
		DL_I2C_flushControllerRXFIFO(I2C_0_INST);

    DL_I2C_clearInterruptStatus(
        I2C_0_INST,
        DL_I2C_INTERRUPT_CONTROLLER_RX_DONE |
        DL_I2C_INTERRUPT_CONTROLLER_NACK);

/* 下面先要发送寄存器地址，因此当前方向必须是TX */
DL_I2C_setControllerDirection(
    I2C_0_INST,
    DL_I2C_CONTROLLER_DIRECTION_TX);

DL_I2C_transmitControllerData(
    I2C_0_INST,
    start_reg);

/* 寄存器地址发送完成后，需要重复起始并转为读取 */
DL_I2C_setControllerDirection(
    I2C_0_INST,
    DL_I2C_CONTROLLER_DIRECTION_RX);

/*
 * 直接赋值，只保留RD_ON_TXEMPTY，
 * 避免把上一笔传输的BURSTRUN等控制位重新写入。
 */
I2C_0_INST->MASTER.MCTR =
    I2C_MCTR_RD_ON_TXEMPTY_ENABLE;

/* 避开MSPM0G3507连续写MCTR可能导致START不生效的问题 */
delay_cycles(8);

DL_I2C_startControllerTransfer(
    I2C_0_INST,
    MPU6050_ADDR,
    DL_I2C_CONTROLLER_DIRECTION_RX,
    length);
		
    while (1)
    {
        status = DL_I2C_getRawInterruptStatus(
            I2C_0_INST,
            DL_I2C_INTERRUPT_CONTROLLER_NACK |
            DL_I2C_INTERRUPT_CONTROLLER_RX_DONE);

        if (status & DL_I2C_INTERRUPT_CONTROLLER_NACK)
        {
            DL_I2C_resetControllerTransfer(I2C_0_INST);
            DL_I2C_flushControllerTXFIFO(I2C_0_INST);
            DL_I2C_flushControllerRXFIFO(I2C_0_INST);
            return false;
        }

        while ((!DL_I2C_isControllerRXFIFOEmpty(I2C_0_INST)) &&
               (index < length))
        {
            buffer[index] =
                DL_I2C_receiveControllerData(I2C_0_INST);

            index++;
            time_out = I2C_TIME_OUT;
        }

        if (status & DL_I2C_INTERRUPT_CONTROLLER_RX_DONE)
        {
            break;
        }

        time_out--;

        if (time_out == 0)
        {
            DL_I2C_resetControllerTransfer(I2C_0_INST);
            DL_I2C_flushControllerTXFIFO(I2C_0_INST);
            DL_I2C_flushControllerRXFIFO(I2C_0_INST);
            return false;
        }
    }

    while ((!DL_I2C_isControllerRXFIFOEmpty(I2C_0_INST)) &&
           (index < length))
    {
        buffer[index] =
            DL_I2C_receiveControllerData(I2C_0_INST);

        index++;
    }

    DL_I2C_resetControllerTransfer(I2C_0_INST);
    DL_I2C_flushControllerTXFIFO(I2C_0_INST);

    return (index == length);
}

// mpu6050初始化函数
/*通信失败             -> false
身份值错误           -> false
唤醒写入失败         -> false
读回失败             -> false
读回值不是 0x00      -> false
全部通过             -> true*/
bool MPU6050_Init(void)
{
	
		MPU6050_InitStage = 1;
	
    uint8_t who_am_i = 0;
    uint8_t pwr_mgmt_1 = 0xFF;
	
		//加速度计掩码检查
	  uint8_t accel_config = 0xFF;
		//陀螺仪掩码检查
		uint8_t gyro_config = 0xFF;
		uint8_t int_enable = 0xFFU;
	


    delay_ms(100);

		MPU6050_InitStage = 2;
    if (!MPU6050_ReadReg(MPU6050_REG_WHO_AM_I, &who_am_i))
    {
        return false;
    }

		MPU6050_InitStage = 3;
    if (who_am_i != MPU6050_WHO_AM_I_VALUE)
    {
        return false;
    }
		

		//唤醒mpu6050
		MPU6050_InitStage = 4;

		if (!MPU6050_WriteReg(
						MPU6050_PWR_MGMT_1,
						MPU6050_PWR_DEVICE_RESET))
		{
				return false;
		}

		/* 复位后，MPU6050需要重新启动内部电路 */
		delay_ms(100);

		MPU6050_InitStage = 5;

		if (!MPU6050_WriteReg(
						MPU6050_PWR_MGMT_1,
						MPU6050_PWR_WAKE_UP))
		{
				return false;
		}

		delay_ms(100);

		MPU6050_InitStage = 6;

		if (!MPU6050_ReadReg(
						MPU6050_PWR_MGMT_1,
						&pwr_mgmt_1))
		{
				return false;
		}

MPU6050_InitStage = 16;

/* 临时把PWR_MGMT_1读回值送到OLED */


if ((pwr_mgmt_1 & MPU6050_PWR_SLEEP_MASK) != 0x00U)
{
    return false;
}
		
		//启用启用约42Hz的陀螺仪低通滤波
		
		
		MPU6050_InitStage = 20;

		
		
				if (!MPU6050_WriteReg(
						MPU6050_REG_CONFIG,
						MPU6050_DLPF_42HZ))
		{
				return false;
		}
				MPU6050_InitStage = 21;

				if (!MPU6050_ReadReg(
								MPU6050_REG_CONFIG,
								&MPU6050_ConfigReadback))
				{
						return false;
					
				}
				MPU6050_InitStage = 22;

				if ((MPU6050_ConfigReadback & 0x07U) !=
								MPU6050_DLPF_42HZ)
				{
						return false;
				}
				
				MPU6050_InitStage = 23;

				if (!MPU6050_WriteReg(
								MPU6050_REG_SMPLRT_DIV,
								MPU6050_DIV_100HZ))
				{
						return false;
				}

				MPU6050_InitStage = 24;

				if (!MPU6050_ReadReg(
								MPU6050_REG_SMPLRT_DIV,
								&MPU6050_DividerReadback))
				{
						return false;
				}

				MPU6050_InitStage = 25;

				if (MPU6050_DividerReadback != MPU6050_DIV_100HZ)
				{
						return false;
				}
		
		
		
		
		
		//检验写入寄存器设置加速度量程
		MPU6050_InitStage = 7;
		if (!MPU6050_WriteReg(
						MPU6050_REG_ACCEL_CONFIG,
						MPU6050_ACCEL_FS_2G))
		{
				return false;
		}

		MPU6050_InitStage = 8;
		if (!MPU6050_ReadReg(
						MPU6050_REG_ACCEL_CONFIG,
						&accel_config))
		{
				return false;
		}

		MPU6050_InitStage = 9;
		if ((accel_config & MPU6050_ACCEL_FS_MASK) !=
				MPU6050_ACCEL_FS_2G)
		{
				return false;
		}
		//检查是否正确写入,读取到陀螺仪寄存器
/* 主动设置陀螺仪量程 */
			MPU6050_InitStage = 10;

			if (!MPU6050_WriteReg(
							MPU6050_REG_GYRO_CONFIG,
							MPU6050_GYRO_FS_SETTING))
			{
					return false;
			}

			/* 读回GYRO_CONFIG寄存器 */
			MPU6050_InitStage = 11;

			if (!MPU6050_ReadReg(
							MPU6050_REG_GYRO_CONFIG,
							&gyro_config))
			{
					return false;
			}

			MPU6050_GyroConfigReadback = gyro_config;

			/* 只检查FS_SEL对应的bit4和bit3 */
			MPU6050_InitStage = 12;

			if ((gyro_config & MPU6050_GYRO_FS_MASK) !=
							MPU6050_GYRO_FS_SETTING)
			{	
					return false;
			}
			
			MPU6050_InitStage = 26;

			if (!MPU6050_WriteReg(
							MPU6050_REG_INT_PIN_CFG,
							MPU6050_INT_PIN_DEFAULT))
			{
					return false;
			}

			MPU6050_InitStage = 27;

			if (!MPU6050_WriteReg(
							MPU6050_REG_INT_ENABLE,
							MPU6050_INT_DATA_READY))
			{
					return false;
			}

			MPU6050_InitStage = 28;

			if (!MPU6050_ReadReg(
							MPU6050_REG_INT_ENABLE,
							&int_enable))
			{
					return false;
			}

			MPU6050_InitStage = 29;

			if ((int_enable & MPU6050_INT_DATA_READY) == 0U)
			{
					return false;
			}
			
			

			MPU6050_InitStage = 13;

			return true;


}

//原始数据读取函数
bool MPU6050_ReadRawData(MPU6050_RawData *data)
{
    uint8_t raw_data[MPU6050_RAW_DATA_LEN];

    if (data == 0)
    {
        return false;
    }

    if (!MPU6050_ReadBytes(
            MPU6050_REG_ACCEL_XOUT_H,
            raw_data,
            MPU6050_RAW_DATA_LEN))
    {
        return false;
    }

    data->ax = MPU6050_CombineBytes(raw_data[0], raw_data[1]);
    data->ay = MPU6050_CombineBytes(raw_data[2], raw_data[3]);
    data->az = MPU6050_CombineBytes(raw_data[4], raw_data[5]);

    data->temperature =
        MPU6050_CombineBytes(raw_data[6], raw_data[7]);

    data->gx = MPU6050_CombineBytes(raw_data[8], raw_data[9]);
    data->gy = MPU6050_CombineBytes(raw_data[10], raw_data[11]);
    data->gz = MPU6050_CombineBytes(raw_data[12], raw_data[13]);

    return true;
}

//原始数据换算加速的函数
int16_t MPU6050_AccelRawToMg(int16_t raw_value)
{
    int32_t scaled_value;

    scaled_value =
        (int32_t)raw_value * MPU6050_MG_PER_G;

    return (int16_t)(
        scaled_value / MPU6050_ACCEL_LSB_PER_G);
}

bool MPU6050_ReadAverageRawData(
    MPU6050_RawData *average_data,
    uint16_t sample_count)
{
    MPU6050_RawData sample;

    int32_t sum_ax = 0;
    int32_t sum_ay = 0;
    int32_t sum_az = 0;
    int32_t sum_temperature = 0;
    int32_t sum_gx = 0;
    int32_t sum_gy = 0;
    int32_t sum_gz = 0;

    uint16_t success_count = 0;
    uint32_t try_count = 0;
    uint32_t max_try;

    if ((average_data == 0) || (sample_count == 0))
    {
        return false;
    }

    max_try = (uint32_t)sample_count * 5U;

    while ((success_count < sample_count) &&
           (try_count < max_try))
    {
        try_count++;

        if (MPU6050_ReadRawData(&sample))
        {
            sum_ax += sample.ax;
            sum_ay += sample.ay;
            sum_az += sample.az;
            sum_temperature += sample.temperature;
            sum_gx += sample.gx;
            sum_gy += sample.gy;
            sum_gz += sample.gz;

            success_count++;
        }

        delay_ms(5);
    }

    if (success_count < sample_count)
    {
        return false;
    }

    average_data->ax =
        (int16_t)(sum_ax / success_count);
    average_data->ay =
        (int16_t)(sum_ay / success_count);
    average_data->az =
        (int16_t)(sum_az / success_count);

    average_data->temperature =
        (int16_t)(sum_temperature / success_count);

    average_data->gx =
        (int16_t)(sum_gx / success_count);
    average_data->gy =
        (int16_t)(sum_gy / success_count);
    average_data->gz =
        (int16_t)(sum_gz / success_count);

    return true;
}

int16_t MPU6050_AccelCorrectMg(
    int16_t measured_mg,
    int16_t offset_mg,
    int16_t one_g_span_mg)
{
    int32_t corrected_value;

    if (one_g_span_mg == 0)
    {
        return 0;
    }

    corrected_value =
        ((int32_t)measured_mg - offset_mg) *
        MPU6050_MG_PER_G;

    return (int16_t)(
        corrected_value / one_g_span_mg);
}

//mpu6050陀螺仪
float MPU6050_GyroRawToDps(int16_t raw_value)
{
    return (float)raw_value /
        MPU6050_GYRO_LSB_PER_DPS;
}
