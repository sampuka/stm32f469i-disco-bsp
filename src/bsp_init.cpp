#include "bsp_init.hpp"

#include "bsp_internals.hpp"

namespace bsp
{

void init()
{
	*((uint32_t*)0x40023830) |= ((1 << 3) | (1 << 6) | (1 << 10));  // Enable GPIO D, G, K in RCC register

	ld1.setup_for_output();
	ld2.setup_for_output();
	ld3.setup_for_output();
	ld4.setup_for_output();
}

}  // namespace bsp