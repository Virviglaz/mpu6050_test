#include <stdio.h>
#include <unistd.h>
#include "i2c.h"
#include "MPU6050.h"
#include <math.h>

static struct i2c_dev i2c;
static uint8_t i2c_err = 0;

static uint8_t i2c_wr(uint8_t I2C_Adrs, uint8_t Reg, uint8_t Value)
{
	i2c_err |= i2c.wr_reg(NULL, 0x68, Reg, &Value, 1);
	return 0;
}

static uint8_t i2c_rd(uint8_t I2C_Adrs, uint8_t Reg, uint8_t * buf, uint16_t size)
{
	i2c_err |= i2c.rd_reg(NULL, 0x68, Reg, buf, size);
	return 0;
}

static void delay_func(unsigned int ms)
{
	sleep(1);
}

static MPU6050_ResultTypeDef result;
static MPU6050_StructTypeDef accel = {
	.MPU6050_Result = &result,
	.WriteReg = i2c_wr,
	.ReadReg = i2c_rd,
	.delay_func = delay_func,
	.GyroSampleRateHz = 10,
	.FilterOrder = 6,
	.GyroScale = GYRO_0250d_s,
	.AccelScale = Scale_2g,
	.CheckRDY_pin = NULL,
};


int main(void)
{
	uint8_t res;
	float x,y,z;
	int ret = i2c_init(&i2c, NULL);
	if (ret) {
		printf("Error %d\n", ret);
		return ret;
	}

	MPU6050_Init(&accel);
	printf("i2c result: %u\n", i2c_err);
	
	while(1) {
		res = MPU6050_GetResult(&accel);
		if (!res) {
			x = (float)accel.MPU6050_Result->X / 16384.0;
			y = (float)accel.MPU6050_Result->Y / 16384.0;
			z = (float)accel.MPU6050_Result->Z / 16384.0;
			printf("x = %.3f, y = %.3f, z = %.3f s = %.3f\t\t\n",
				x,y,z, sqrtf(x*x+y*y+z*z));
		}
	}
	
	i2c_close(&i2c);
	return 0;
}