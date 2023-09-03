#include "bsp_leds.hpp"

#include "bsp_internals.hpp"

namespace
{

void set_pin(hal::GpioPin& pin, bool state)
{
	if (state)
	{
		pin.set_output_high();
	}
	else
	{
		pin.set_output_low();
	}
}

}  // namespace

namespace bsp
{

void set_ld1(bool state)
{
	set_pin(ld1, !state);
}

void set_ld2(bool state)
{
	set_pin(ld2, !state);
}

void set_ld3(bool state)
{
	set_pin(ld3, !state);
}

void set_ld4(bool state)
{
	set_pin(ld4, !state);
}

}  // namespace bsp
