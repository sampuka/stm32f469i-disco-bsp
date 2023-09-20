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

	static constexpr uint32_t width = 100;
	static constexpr uint32_t height = 100;

private:
	std::array<std::array<pixel_t, height>, width> buffer;
};

}  // namespace bsp

#endif  // INCLUDE_BSP_FRAME_BUFFER_HPP
