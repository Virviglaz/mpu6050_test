#include <stdio.h>
#include <unistd.h>
#include "i2c.h"
#include "MPU6050.h"
#include "apds9960.h"
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

static uint8_t i2c_wr_adps9930(uint8_t Reg, uint8_t Value)
{
	i2c_err |= i2c.wr_reg(NULL, 0x39, Reg, &Value, 1);
	return 0;
}

static uint8_t i2c_rd_adps9930(uint8_t Reg, uint8_t * buf, uint16_t size)
{
	i2c_err |= i2c.rd_reg(NULL, 0x39, Reg, buf, size);
	return 0;
}

static void delay_func(void)
{
	sleep(1);
}

static struct mpu_real_values acc_data;
struct rgbs_data crgb;

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

static struct apds9960 apds = {
	.write_reg = i2c_wr_adps9930,
	.read_reg = i2c_rd_adps9930,
	.check_irq = NULL,
	.wait_for_irq = NULL,
	.ctrl1 = { .again = AGAIN_8x, .pgain = PGAIN_1x, .led_c = LED_12mA },
	.conf2 = { .led_boost = BOOST_100 },
};

int main(void)
{
	uint8_t ret;
	if (ret = i2c_init(&i2c, "/dev/i2c-1")) {
		printf("Error %d\n", ret);
		return ret;
	}
	printf("Struct size: %u\n", sizeof(apds));

	if (!apds9960_init(&apds)) {
		printf("ADPS9960 init done!\n");
		apds9960_meas_crgb(&apds, &crgb);
		printf("RED:	%u\n", crgb.r);
		printf("GREEN:	%u\n", crgb.g);
		printf("BLUE:	%u\n", crgb.b);
		printf("CLEAR:	%u\n", crgb.c);
	}
	
	while (++ret < 10) {
		if (apds9960_proximity(&apds, 0xFFF0))
			printf("Proxy pass!\n");
	}
	
	return 0;
	mpu6050_init(&accel, delay_func);
	printf("i2c result: %u\n", i2c_err);

	while(1) {
		if (!mpu6050_get_result(&accel)) {
			double lxy, lyz, lzx;
			double sum = acc_data.x * acc_data.x;
			sum += acc_data.y * acc_data.y;
			sum += acc_data.z * acc_data.z;
			sum = sqrt(sum);
			lxy = atan2(acc_data.y, acc_data.x) * 180 / 3.1415;
			lyz = atan2(acc_data.z, acc_data.y) * 180 / 3.1415;
			lzx = atan2(acc_data.x, acc_data.z) * 180 / 3.1415;

			printf("x = %.3f, y = %.3f, z = %.3f s = %.3f, t = %.2f\r",
				acc_data.x, acc_data.y, acc_data.z, sum, acc_data.temp);
			//printf("lxy: %.0f, lyz: %.0f, lzx: %.0f\r", lxy, lyz, lzx);
		}
	}

	i2c_close(&i2c);
	return 0;
}
