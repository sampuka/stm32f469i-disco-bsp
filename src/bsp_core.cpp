#include "bsp_core.hpp"

#include "bsp_internals.hpp"
#include "hal_exceptions.hpp"

namespace bsp
{

void (*callback_1ms)(void) = nullptr;

void register_1ms_callback(void (*new_callback_1ms)(void))
{
	callback_1ms = new_callback_1ms;
}

}  // namespace bsp

void SysTick_Handler()
{
	if (bsp::callback_1ms != nullptr)
	{
		bsp::callback_1ms();
	}
}