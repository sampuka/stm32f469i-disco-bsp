#ifndef INCLUDE_BSP_DISPLAY_HPP
#define INCLUDE_BSP_DISPLAY_HPP

#include "bsp_frame_buffer.hpp"

namespace hal
{
struct DisplayTimings;
}

namespace bsp
{

class Display
{
public:
	Display() = default;

	FrameBuffer& get_buffer();

	void set_background_color(uint8_t red, uint8_t green, uint8_t blue);
	void init();

private:
	void configure_ltdc(const hal::DisplayTimings& timings);

	FrameBuffer buffer;
	uint8_t background_red = 0;
	uint8_t background_green = 0;
	uint8_t background_blue = 0;
	bool ltdc_initialized = false;
};

inline Display display;

}  // namespace bsp

#endif  // INCLUDE_BSP_DISPLAY_HPP
