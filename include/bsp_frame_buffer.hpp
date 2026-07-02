#ifndef INCLUDE_BSP_FRAME_BUFFER_HPP
#define INCLUDE_BSP_FRAME_BUFFER_HPP

#include <array>
#include <cstdint>

namespace bsp
{

using pixel_t = uint32_t;

class FrameBuffer
{
public:
	uintptr_t get_start_address() const;
	void fill(pixel_t color);
	void set_pixel(uint32_t x, uint32_t y, pixel_t color);
	void fill_test_pattern();

	static constexpr uint32_t width = 100;
	static constexpr uint32_t height = 480;

private:
	std::array<std::array<pixel_t, width>, height> buffer;
};

}  // namespace bsp

#endif  // INCLUDE_BSP_FRAME_BUFFER_HPP
