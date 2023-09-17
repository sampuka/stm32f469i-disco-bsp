#include "bsp_init.hpp"

#include "bsp_internals.hpp"
#include "hal_rcc.hpp"
#include "hal_system_control.hpp"
#include "hal_timer.hpp"

namespace bsp
{

void init()
{
	hal::system_control.enable_systick();

	hal::rcc.enable_tim2();

	hal::tim2.set_interrupt();

	hal::tim2.enable();

	hal::rcc.enable_gpio_d();
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
}

}  // namespace bsp