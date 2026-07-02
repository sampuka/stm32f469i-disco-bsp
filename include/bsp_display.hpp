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

	void init();

private:
	void configure_ltdc(const hal::DisplayTimings& timings);

	FrameBuffer buffer;
};

inline Display display;

}  // namespace bsp

#endif  // INCLUDE_BSP_DISPLAY_HPP
