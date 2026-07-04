#include "bsp_frame_buffer.hpp"

namespace bsp
{

uintptr_t FrameBuffer::get_start_address() const
{
	return external_sdram_start_address;
}

pixel_t* FrameBuffer::data()
{
	return reinterpret_cast<pixel_t*>(external_sdram_start_address);
}

const pixel_t* FrameBuffer::data() const
{
	return reinterpret_cast<const pixel_t*>(external_sdram_start_address);
}

void FrameBuffer::fill(pixel_t color)
{
	pixel_t* pixels = data();

	for (uint32_t i = 0; i < width * height; i++)
	{
		pixels[i] = color;
	}
}

void FrameBuffer::set_pixel(uint32_t x, uint32_t y, pixel_t color)
{
	if (x < width && y < height)
	{
		data()[y * width + x] = color;
	}
}

}  // namespace bsp
