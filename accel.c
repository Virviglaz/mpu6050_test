#include <stdio.h>
#include <unistd.h>
#include "i2c.h"
#include "MPU6050.h"
#include "apds9960.h"
#include "BME280.h"
#include "PCA9685.h"
#include "ads111x.h"
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

static uint8_t bme280_write(uint8_t reg, uint8_t value)
{
	i2c_err |= i2c.wr_reg(NULL, BME280_DEFAULT_I2C_ADDRESS, reg, &value, 1);
	return 0;
}

static uint8_t bme280_read(uint8_t reg, uint8_t *buf, uint16_t size)
{
	i2c_err |= i2c.rd_reg(NULL, BME280_DEFAULT_I2C_ADDRESS, reg, buf, size);
	return 0;
}

static void delay_func(void)
{
	sleep(1);
}

static uint8_t wr(uint8_t i2c_addr, uint8_t reg, uint8_t *buf, uint8_t size)
{
	i2c_err |= i2c.wr_reg(NULL, i2c_addr, reg, buf, size);
	return 0;
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
};

static struct bme280_t bme280 = {
	.write_reg = bme280_write,
	.read_reg = bme280_read,
	.humidity_oversampling = BME280_OVS_X4,
	.pressure_oversampling = BME280_OVS_X4,
	.temperature_oversampling = BME280_OVS_X4,
};

static struct pca9685_t pwm = {
	.wr = wr,
	.i2c_addr = 0x40,
	.ext_clk = PCA9685_EXT_CLK_DISABLED,
	.psc = PCA9685_50HZ_PSC_VALUE,
};

static void set_pwm(uint16_t value)
{
	int i;
	for (i = 0; i < 15; i++)
		pca9685_set(&pwm, i, value);
}

int main(void)
{
	uint8_t ret, i = 10;

	if (ret = i2c_init(&i2c, "/dev/i2c-1")) {
		printf("Error %d\n", ret);
		return ret;
	}

	if (!pca9685_init(&pwm)) {
		for (i = 0; i != 10; i++) {
			printf("PWM works %u\n", i2c_err);
			set_pwm(i * 100);
			sleep(1);
		}
	}

	/*if (!bme280_init(&bme280)) {
		while (bme280_calibrate_sea_level(&bme280));
		while (bme280_get_result(&bme280));
		printf("Temperature: %f\n", bme280.temperature);
		printf("Humidity: %f\n", bme280.humidity);
		printf("Pressure: %u\n", bme280.pressure);
		printf("Altitude: %f\n", bme280_altitude(&bme280));
		printf("mmHg: %u\n", bme280_mmHg(&bme280));
	}*/

	printf("Struct size: %u\n", sizeof(apds));

	if (!apds9960_init(&apds, false, true)) {
		printf("ADPS9960 init done!\n");
		apds9960_meas_crgb(&apds, &crgb);
		printf("RED:	%u\n", crgb.red);
		printf("GREEN:	%u\n", crgb.green);
		printf("BLUE:	%u\n", crgb.blue);
		printf("CLEAR:	%u\n", crgb.clear);
	}
		
	while (--i) {
		if (ret = apds9960_proximity(&apds))
			printf("Proxy pass: %u\n", ret);
	}
	
	i = 100;
	while (--i) {
		enum apds_gesture gesture = apds9960_gesture(&apds);
		switch (gesture) {
		case NO_ACTIVITY:
			printf("No activity\n");
			break;
		case DIR_UP:
			printf("Direction up\n");
			break;
		case DIR_DOWN:
			printf("Direction down\n");
			break;
		case DIR_LEFT:
			printf("Direction left\n");
			break;
		case DIR_RIGHT:
			printf("Direction right\n");
			break;
		case ERR_DATA_INVALID:
			printf("Data invalid\n");
			break;
		default:
			printf("Undefined value\n");
		}		
	}
	
	i2c_close(&i2c);
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
