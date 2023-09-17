#include "bsp_devtemp.hpp"

#include "hal_timer.hpp"

namespace bsp
{

std::uint32_t get_tim2_counter()
{
	return hal::tim2.get_counter();
}

void reset_tim2_interrupt()
{
	hal::tim2.reset_interrupt();
	// tim2.disable();
}

}  // namespace bsp