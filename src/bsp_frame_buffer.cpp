#include "bsp_frame_buffer.hpp"

namespace bsp
{

uintptr_t FrameBuffer::get_start_address() const
{
	return reinterpret_cast<uintptr_t>(&buffer[0][0]);
}

}  // namespace bsp