#ifndef SRC_BSP_INTERNALS_HPP
#define SRC_BSP_INTERNALS_HPP

#include "hal_gpio_pin.hpp"

namespace bsp
{

inline hal::GpioPin ld1(hal::gpio_g, 6);
inline hal::GpioPin ld2(hal::gpio_d, 4);
inline hal::GpioPin ld3(hal::gpio_d, 5);
inline hal::GpioPin ld4(hal::gpio_k, 3);

}  // namespace bsp

#endif  // SRC_BSP_INTERNALS_HPP
