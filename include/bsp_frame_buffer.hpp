#ifndef INCLUDE_BSP_FRAME_BUFFER_HPP
#define INCLUDE_BSP_FRAME_BUFFER_HPP

#include <cstdint>

namespace bsp
{

using pixel_t = uint32_t;

class FrameBuffer
{
public:
	uintptr_t get_start_address() const;
	pixel_t* data();
	const pixel_t* data() const;
	void fill(pixel_t color);
	void set_pixel(uint32_t x, uint32_t y, pixel_t color);

	static constexpr uint32_t width = 800;
	static constexpr uint32_t height = 480;

private:
	// UM1932 maps the board's 128-Mbit SDRAM on FMC bank 1 from 0xC0000000
	// through 0xC0FFFFFF. The LCD framebuffer lives at the start of that range.
	static constexpr uintptr_t external_sdram_start_address = 0xC0000000;
};

}  // namespace bsp

#endif  // INCLUDE_BSP_FRAME_BUFFER_HPP
