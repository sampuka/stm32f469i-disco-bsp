#ifndef INCLUDE_BSP_DISPLAY_HPP
#define INCLUDE_BSP_DISPLAY_HPP

namespace bsp
{

class Display
{
public:
	Display() = default;

	void init();

private:
	void configure_ltdc();
};

}  // namespace bsp

#endif  // INCLUDE_BSP_DISPLAY_HPP
