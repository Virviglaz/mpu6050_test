#include <stdio.h>
#include <unistd.h>
#include "i2c.h"
#include "MPU6050.h"
#include <math.h>

static struct i2c_dev i2c;
static uint8_t i2c_err = 0;

static uint8_t i2c_wr(uint8_t Reg, uint8_t Value)
{
	i2c_err |= i2c.wr_reg(NULL, 0x68, Reg, &Value, 1);
	return 0;
}

static uint8_t i2c_rd(uint8_t Reg, uint8_t * buf, uint16_t size)
{
	i2c_err |= i2c.rd_reg(NULL, 0x68, Reg, buf, size);
	return 0;
}

static void delay_func(void)
{
	sleep(1);
}

static struct mpu_real_values acc_data;

static struct mpu_conf accel = {
	.write_reg = i2c_wr,
	.read_reg = i2c_rd,
	.sample_rate_hz = 50,
	.filter_order = 1,
	.gyro_scale = GYRO_0250DS,
	.acc_scale = SCALE_2G,
	.check_ready_pin = NULL,
	.zero_point = NULL,
	.real_values = &acc_data,
};

int main(void)
{
	uint8_t ret;
	if (ret = i2c_init(&i2c, NULL)) {
		printf("Error %d\n", ret);
		return ret;
	}
	printf("Struct size: %u\n", sizeof(struct mpu_measdata));

	mpu6050_init(&accel, delay_func);
	printf("i2c result: %u\n", i2c_err);
	
	while(1) {
		if (!mpu6050_get_result(&accel)) {
			float sum = acc_data.x * acc_data.x;
			sum += acc_data.y * acc_data.y;
			sum += acc_data.z * acc_data.z;
			sum = sqrt(sum);

			printf("x = %.3f, y = %.3f, z = %.3f s = %.3f, t = %.2f\n",
				acc_data.x, acc_data.y, acc_data.z, sum, acc_data.temp);
		}
	}
	
	i2c_close(&i2c);
	return 0;
}