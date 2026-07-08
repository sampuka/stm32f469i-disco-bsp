#include "bsp_sdram.hpp"

#include "hal_fmc.hpp"
#include "hal_gpio.hpp"
#include "hal_rcc.hpp"

#include <cstdint>

namespace bsp
{

namespace
{

constexpr uint8_t fmc_alternate_function = 12;

// UM1932 identifies the external memory as a 128-Mbit MT48LC4M32B2B5-7 SDRAM
// on FMC bank 1. ST's STM32F469I Discovery BSP drives it with a 90 MHz SDCLK;
// these timing fields are the corresponding FMC cycle counts.
constexpr hal::SdramTiming sdram_timing = {
	.load_to_active_delay = 2,
	.exit_self_refresh_delay = 7,
	.self_refresh_time = 4,
	.row_cycle_delay = 7,
	.write_recovery_time = 2,
	.row_precharge_delay = 2,
	.row_to_column_delay = 2,
};

// ST's board BSP uses this FMC refresh counter for the 90 MHz SDRAM clock.
constexpr uint32_t sdram_refresh_count = 0x0569;

// MT48LC4M32B2B5 mode register: burst length 1, sequential burst, CAS latency
// 3, standard operation, and single-location write burst.
constexpr uint32_t sdram_mode_register = 0x0230;

// After the clock-enable command, the SDRAM needs at least 100 us before the
// next command. This loop is intentionally conservative during board bring-up.
constexpr uint32_t sdram_power_up_delay_cycles = 180000;

void busy_wait_cycles(uint32_t cycles)
{
	while (cycles > 0)
	{
		asm volatile("" ::: "memory");
		cycles--;
	}
}

void configure_fmc_pin(hal::Gpio& gpio, uint8_t pin)
{
	gpio.set_mode(pin, hal::Gpio::Mode::ALTFUNC);
	gpio.set_output_type(pin, hal::Gpio::OutputType::PUSHPULL);
	// ST's BSP uses GPIO_SPEED_FAST for these FMC pins; in this codebase that
	// maps to the HIGH drive setting rather than the absolute maximum.
	gpio.set_speed(pin, hal::Gpio::Speed::HIGH);
	gpio.set_pull(pin, hal::Gpio::Pull::NOPULLUP_PULLDOWN);
	gpio.set_alternate_function(pin, fmc_alternate_function);
}

template <typename... Pins>
void configure_fmc_pins(hal::Gpio& gpio, Pins... pins)
{
	(configure_fmc_pin(gpio, pins), ...);
}

void configure_sdram_pins()
{
	hal::rcc.enable_gpio_c();
	hal::rcc.enable_gpio_d();
	hal::rcc.enable_gpio_e();
	hal::rcc.enable_gpio_f();
	hal::rcc.enable_gpio_g();
	hal::rcc.enable_gpio_h();
	hal::rcc.enable_gpio_i();

	// UM1932 routes the SDRAM through the FMC alternate function. The concrete
	// pin groups below match ST's STM32469I Discovery SDRAM MSP init.
	configure_fmc_pins(hal::gpio_c, 0);
	configure_fmc_pins(hal::gpio_d, 0, 1, 8, 9, 10, 14, 15);
	configure_fmc_pins(hal::gpio_e, 0, 1, 7, 8, 9, 10, 11, 12, 13, 14, 15);
	configure_fmc_pins(hal::gpio_f, 0, 1, 2, 3, 4, 5, 11, 12, 13, 14, 15);
	configure_fmc_pins(hal::gpio_g, 0, 1, 4, 5, 8, 15);
	configure_fmc_pins(hal::gpio_h, 2, 3, 8, 9, 10, 11, 12, 13, 14, 15);
	configure_fmc_pins(hal::gpio_i, 0, 1, 2, 3, 4, 5, 6, 7, 9, 10);
}

}  // namespace

void init_sdram()
{
	configure_sdram_pins();

	hal::rcc.enable_fmc();
	hal::rcc.reset_fmc();

	hal::fmc.configure_sdram_bank1(sdram_timing);

	// Standard SDRAM power-up sequence from the FMC reference flow:
	// clock enable, delay, precharge all, auto-refresh, load mode register,
	// then program the periodic refresh counter.
	hal::fmc.send_sdram_bank1_command(hal::Fmc::SdramCommand::CLOCK_ENABLE, 1, 0);
	busy_wait_cycles(sdram_power_up_delay_cycles);
	hal::fmc.send_sdram_bank1_command(hal::Fmc::SdramCommand::PRECHARGE_ALL, 1, 0);
	hal::fmc.send_sdram_bank1_command(hal::Fmc::SdramCommand::AUTO_REFRESH, 8, 0);
	hal::fmc.send_sdram_bank1_command(hal::Fmc::SdramCommand::LOAD_MODE_REGISTER, 1, sdram_mode_register);
	hal::fmc.set_sdram_refresh_rate(sdram_refresh_count);
}

}  // namespace bsp
