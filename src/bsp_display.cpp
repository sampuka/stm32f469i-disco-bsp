#include "bsp_display.hpp"

#include "hal_gpio.hpp"
#include "hal_ltdc.hpp"
#include "hal_rcc.hpp"

namespace bsp
{

namespace
{

constexpr uint8_t lcd_reset_pin = 7;
constexpr uint32_t lcd_reset_delay_cycles = 320000;

constexpr uint8_t ltdc_background_red = 0x10;
constexpr uint8_t ltdc_background_green = 0x10;
constexpr uint8_t ltdc_background_blue = 0x40;

constexpr hal::DisplayTimings lcd_timings = {.hsync_width = 2,
                                             .vsync_width = 1,
                                             .horizontal_back_porch = 34,
                                             .vertical_back_porch = 15,
                                             .horizontal_front_porch = 34,
                                             .vertical_front_porch = 16,
                                             .active_width = 800,
                                             .active_height = 480};

void busy_wait_cycles(uint32_t cycles)
{
	while (cycles > 0)
	{
		asm volatile("" ::: "memory");
		cycles--;
	}
}

void reset_lcd_panel()
{
	hal::rcc.enable_gpio_h();

	hal::gpio_h.set_mode(lcd_reset_pin, hal::Gpio::Mode::OUTPUT);
	hal::gpio_h.set_output_type(lcd_reset_pin, hal::Gpio::OutputType::OPENDRAIN);
	hal::gpio_h.set_speed(lcd_reset_pin, hal::Gpio::Speed::HIGH);
	hal::gpio_h.set_pull(lcd_reset_pin, hal::Gpio::Pull::NOPULLUP_PULLDOWN);

	hal::gpio_h.set_output_low(lcd_reset_pin);
	busy_wait_cycles(lcd_reset_delay_cycles);

	hal::gpio_h.set_output_high(lcd_reset_pin);
	busy_wait_cycles(lcd_reset_delay_cycles);
}

}  // namespace

FrameBuffer& Display::get_buffer()
{
	return buffer;
}

void Display::init()
{
	buffer.fill_test_pattern();

	hal::rcc.enable_dsi();
	hal::rcc.enable_ltdc();

	hal::rcc.configure_pllsai_for_ltdc_27mhz();
	hal::rcc.enable_pllsai();
	while (!hal::rcc.is_pllsai_ready())
	{
		;
	}

	reset_lcd_panel();

	configure_ltdc();

	// TODO: Initialize the DSI host/wrapper and the LCD controller before
	// expecting the board's MIPI-DSI panel to show the LTDC output.
}

void Display::configure_ltdc()
{
	hal::ltdc.set_timing_registers(lcd_timings);
	hal::ltdc.set_background_color(ltdc_background_red, ltdc_background_green, ltdc_background_blue);

	hal::ltdc.configure_argb8888_layer(lcd_timings, buffer.get_start_address(), FrameBuffer::width, FrameBuffer::height);
	hal::ltdc.enable_layer();
	hal::ltdc.reload();
	hal::ltdc.enable();
}

}  // namespace bsp
