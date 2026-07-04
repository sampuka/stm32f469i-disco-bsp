#include "bsp_buttons.hpp"

#include "bsp_internals.hpp"

namespace bsp
{

bool user_button_state()
{
	return user_button.is_high();
}

}  // namespace bsp
