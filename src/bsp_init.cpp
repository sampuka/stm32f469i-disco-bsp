#include "bsp_init.hpp"

#include "bsp_core.hpp"
#include "bsp_i2c.hpp"
#include "bsp_internals.hpp"
#include "bsp_sdram.hpp"
#include "hal_rcc.hpp"
#include "hal_system_control.hpp"

namespace bsp
{

void init()
{
	hal::rcc.configure_system_clock_180mhz_from_hse();

	register_1ms_callback(nullptr);
	hal::system_control.enable_systick(hal::Rcc::system_clock_hz);

	hal::rcc.enable_gpio_d();
	hal::rcc.enable_gpio_c();
	hal::rcc.enable_gpio_g();
	hal::rcc.enable_gpio_k();

	ld1.setup_for_output();
	ld2.setup_for_output();
	ld3.setup_for_output();
	ld4.setup_for_output();
	ld1.set_output_high();
	ld2.set_output_high();
	ld3.set_output_high();
	ld4.set_output_high();
	user_button.setup_for_input(hal::Gpio::Pull::PULLDOWN);
	i2c1.init();

	init_sdram();
}

}  // namespace bsp
