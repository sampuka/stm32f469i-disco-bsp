#include "bsp_devtemp.hpp"

#include "bsp_internals.hpp"

namespace bsp
{

std::uint32_t get_tim2_counter()
{
	return tim2.get_counter();
}

void reset_tim2_interrupt()
{
	tim2.reset_interrupt();
	// tim2.disable();
}

}  // namespace bsp