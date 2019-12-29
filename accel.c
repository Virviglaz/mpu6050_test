#include <stdio.h>
#include <unistd.h>
#include "i2c.h"
#include "ina3221.h"

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

struct ina3221_t ina = {
	.i2c_addr = INA3221_I2C_ADDRESS_GND,
	.wr = i2c_wr,
	.rd = i2c_rd,
	.config = {
		.mode = SHUNT_AND_BUS_CONTIUOUS,
		.shunt_voltage_conv_time = CONV_TIME_1100US,
		.bus_voltage_conv_time = CONV_TIME_1100US,
		.average = AVG_1024,
		.reset = false,
		.ch1_enable = true,
		.ch2_enable = true,
		.ch3_enable = true,
	},
	.shunt[0] = 0.1,
	.shunt[1] = 0.1,
	.shunt[2] = 0.1,
};

int main(void)
{
	uint8_t ret, i = 10;
	double voltage, current;

	ret = i2c_init(&i2c, "/dev/i2c-1");
	if (ret) {
		printf("Error %d\n", ret);
		return ret;
	}

	if (!ina3221_init(&ina)) {
		printf("ina3221 works!\n");
		ina3221_set_warning_current(&ina, 1.0, CH1);
		ina3221_set_critical_current(&ina, 2.0, CH1);
		while (i--) {
			voltage = ina3221_read_bus_voltage(&ina, CH1);
			current = ina3221_read_shunt_current(&ina, CH1);
			//while (!ina3221_read_bus_voltage(&ina, 0, &voltage));
			//while (!ina3221_read_shunt_current(&ina, 0, &current));
			printf("RES: %.3f V\t\t%.3f A\n", voltage, current);
		}
	}

	i2c_close(&i2c);
	return 0;
}
