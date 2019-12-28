#include <stdio.h>
#include <unistd.h>
#include "i2c.h"
#include "ads111x.h"
#include <math.h>

static struct i2c_dev i2c;
static uint8_t i2c_err = 0;

static uint8_t i2c_wr(uint8_t i2c_addr, uint8_t reg, uint8_t *buf, uint16_t size)
{
	i2c_err |= i2c.wr_reg(NULL, i2c_addr, reg, buf, size);
	printf("0x%.2X: 0x%.2X <- 0x%.4X\n", i2c_addr, reg, *(uint16_t*)buf);
	return i2c_err;
}

static uint8_t i2c_rd(uint8_t i2c_addr, uint8_t reg, uint8_t *buf, uint16_t size)
{
	i2c_err |= i2c.rd_reg(NULL, i2c_addr, reg, buf, size);
	if (size == 2)
		printf("0x%.2X: 0x%.2X -> 0x%.4X\n", i2c_addr, reg, *(uint16_t*)buf);
	else
		printf("0x%.2X: 0x%.2X -> 0x%.2X\n", i2c_addr, reg, *(uint8_t*)buf);
	return i2c_err;
}

struct ads111x_t adc = {
	.i2c_addr = ADS111X_I2C_ADDR_GND,
	.wr = i2c_wr,
	.rd = i2c_rd,
	.gain = FSR_1_024V,
	.mode = SINGLE_SHOT,
	.rate = RATE_8_HZ,
	.comp = TRADITIONAL,
	.cpol = COMP_ACTIVE_HIGH,
	.comp_latch = COMP_LATCH_DISABLE,
	.comp_mode = COMP_DISABLED,
};

int main(void)
{
	uint8_t ret, i;

	ret = i2c_init(&i2c, "/dev/i2c-1");
	if (ret) {
		printf("Error %d\n", ret);
		return ret;
	}

	if (!ads111x_init(&adc)) {
		int16_t res;
		printf("ADC works: %u\n", i2c_err);
		for (i = 0; i < 10; i++) {
			ads111x_start(&adc, IN0_GND);
			while (ads111x_read(&adc, &res));
			printf("ADC result: %d\n", res);
		}
	}

	i2c_close(&i2c);
	return 0;
}
