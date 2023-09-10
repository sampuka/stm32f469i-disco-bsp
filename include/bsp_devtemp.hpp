#ifndef INCLUDE_BSP_DEVTEMP_HPP
#define INCLUDE_BSP_DEVTEMP_HPP

#include <cstdint>

namespace bsp
{

std::uint32_t get_tim2_counter();

void reset_tim2_interrupt();

}  // namespace bsp

#endif  // INCLUDE_BSP_DEVTEMP_HPP
