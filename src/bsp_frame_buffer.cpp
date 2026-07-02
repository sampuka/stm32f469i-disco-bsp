#include "bsp_frame_buffer.hpp"

namespace bsp
{

uintptr_t FrameBuffer::get_start_address() const
{
	return reinterpret_cast<uintptr_t>(&buffer[0][0]);
}

void FrameBuffer::fill(pixel_t color)
{
	for (auto& row : buffer)
	{
		row.fill(color);
	}
}

void FrameBuffer::set_pixel(uint32_t x, uint32_t y, pixel_t color)
{
	if (x < width && y < height)
	{
		buffer[y][x] = color;
	}
}

}  // namespace bsp
