#ifndef BSP_INCLUDE_BSP_I2C_HPP
#define BSP_INCLUDE_BSP_I2C_HPP

#include "hal_gpio_pin.hpp"
#include "hal_i2c.hpp"

#include <cstdint>

namespace bsp
{

class I2cBus
{
public:
	I2cBus();

	void init();

	bool write(uint8_t address_7bit, const uint8_t* data, uint32_t size);
	bool read(uint8_t address_7bit, uint8_t* data, uint32_t size);
	bool write_read(uint8_t address_7bit, const uint8_t* write_data, uint32_t write_size, uint8_t* read_data, uint32_t read_size);

private:
	hal::GpioPin scl;
	hal::GpioPin sda;
	hal::I2c& i2c;
};

inline I2cBus i2c1;

}  // namespace bsp

#endif  // BSP_INCLUDE_BSP_I2C_HPP
