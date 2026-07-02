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

void FrameBuffer::fill_test_pattern()
{
	for (uint32_t y = 0; y < height; y++)
	{
		for (uint32_t x = 0; x < width; x++)
		{
			const uint8_t red = static_cast<uint8_t>((x * 255) / (width - 1));
			const uint8_t green = static_cast<uint8_t>((y * 255) / (height - 1));
			const uint8_t blue = ((x / 10 + y / 10) % 2) ? 0x20 : 0xFF;

			buffer[y][x] = (0xFFu << 24) | (static_cast<uint32_t>(red) << 16) | (static_cast<uint32_t>(green) << 8) | blue;
		}
	}
}

}  // namespace bsp
