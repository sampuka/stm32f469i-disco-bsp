#ifndef BSP_INCLUDE_BSP_INTERNALS_HPP
#define BSP_INCLUDE_BSP_INTERNALS_HPP

#include "hal/hal_gpio.hpp"

namespace bsp
{

inline hal::Gpio& gpio_d = *new (hal::Gpio::GPIOD) hal::Gpio;
inline hal::Gpio& gpio_g = *new (hal::Gpio::GPIOG) hal::Gpio;
inline hal::Gpio& gpio_k = *new (hal::Gpio::GPIOK) hal::Gpio;

inline hal::GpioPin ld1(gpio_g, 6);
inline hal::GpioPin ld2(gpio_d, 4);
inline hal::GpioPin ld3(gpio_d, 5);
inline hal::GpioPin ld4(gpio_k, 3);

}  // namespace bsp

#endif  // BSP_INCLUDE_BSP_INTERNALS_HPP
