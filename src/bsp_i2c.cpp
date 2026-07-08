#include "bsp_i2c.hpp"

#include "hal_rcc.hpp"

namespace bsp
{

namespace
{

constexpr uint32_t i2c1_peripheral_clock_hz = 45'000'000;
constexpr uint32_t i2c1_bus_clock_hz = 100'000;
constexpr uint8_t i2c1_scl_pin = 8;
constexpr uint8_t i2c1_sda_pin = 9;
constexpr uint8_t i2c1_alternate_function = 4;

}  // namespace

I2cBus::I2cBus() : scl(hal::gpio_b, i2c1_scl_pin), sda(hal::gpio_b, i2c1_sda_pin), i2c(hal::i2c1)
{
}

void I2cBus::init()
{
	hal::rcc.enable_gpio_b();
	hal::rcc.enable_i2c1();
	hal::rcc.reset_i2c1();

	scl.setup_for_alternate_function(i2c1_alternate_function);
	sda.setup_for_alternate_function(i2c1_alternate_function);

	i2c.reset();
	i2c.configure_standard_mode(i2c1_peripheral_clock_hz, i2c1_bus_clock_hz);
	i2c.enable();
}

bool I2cBus::write(uint8_t address_7bit, const uint8_t* data, uint32_t size)
{
	return i2c.write(address_7bit, data, size);
}

bool I2cBus::read(uint8_t address_7bit, uint8_t* data, uint32_t size)
{
	return i2c.read(address_7bit, data, size);
}

bool I2cBus::write_read(uint8_t address_7bit, const uint8_t* write_data, uint32_t write_size, uint8_t* read_data, uint32_t read_size)
{
	return i2c.write_read(address_7bit, write_data, write_size, read_data, read_size);
}

}  // namespace bsp
