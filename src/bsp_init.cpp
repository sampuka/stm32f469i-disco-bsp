#include "bsp_init.hpp"

#include "bsp_internals.hpp"

namespace bsp
{

void init()
{
	system_control.enable_systick();

	rcc.enable_tim2();

	tim2.set_interrupt();

	tim2.enable();

	rcc.enable_gpio_d();
	rcc.enable_gpio_g();
	rcc.enable_gpio_k();

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