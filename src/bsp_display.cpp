#include "bsp_display.hpp"

#include "hal_dsi.hpp"
#include "hal_ltdc.hpp"
#include "hal_nvic.hpp"
#include "hal_rcc.hpp"

namespace bsp
{

// Based on AN4861 Application Notes
// "LCD-TFT timings extracted from ROCKTECH RK043FN48H datasheet"

constexpr hal::DisplayTimings display_timings = {
	.hsync_width = 1, .vsync_width = 10, .horizontal_back_porch = 43, .vertical_back_porch = 12, .horizontal_front_porch = 8, .vertical_front_porch = 4, .active_width = 800, .active_height = 480

};

FrameBuffer& Display::get_buffer()
{
	return buffer;
}

void Display::init()
{
	// Developed based on Chapter 18.14 of RM0386 Reference Manual

	// Step 1: Configure RCC
	// Step 1.1: Enable clock for DSI and LTDC
	hal::rcc.enable_dsi();
	hal::rcc.enable_ltdc();

	// Step 1.2: Configure LTDC PLL, turn it ON and wait for its lock
	// configure using PLLSAICFGR?
	hal::rcc.enable_pllsai();
	while (!hal::rcc.is_pllsai_ready())
	{
		;
	}

	// Step 2: Optionally configure the GPIO (if tearing effect requires GPIO usage for example)
	// Optionally :))

	// Step 3: Optionally valid the ISR
	hal::nvic.enable_exception(88);  // LTDC global interrupt
	hal::nvic.enable_exception(89);  // LTDC global error interrupt
	hal::nvic.enable_exception(92);  // DSI global interrupt

	// Step 4: Configure the LTDC
	configure_ltdc();

	// Step 5: Turn on the DSI regulator and wait for the regulator ready

	// Step 6: Configure the DSI PLL, turn it ON and wait for its lock

	// Step 7: Configure the D-PHY parameters in the DSI Host and the DSI wrapper to define D-PHY configuration and timing

	// Step 8: Configure the DSI Host timings

	// Step 9: Configure the DSI Host flow control and DBI interface

	// Step 10: Configure the DSI Host LTDC interface

	// Step 11: Configure the DSI Host for video mode or adapted command mode

	// Step 12: Enable the D-PHY setting the DEN bit of the DSI_PCTLR

	// Step 13: Enable the D-PHY clock lane setting the CKEN bit of the DSI_PCTLR

	// Step 14: Enable the DSI Host setting the EN bit of the DSI_CR

	// Step 15: Enable the DSI wrapper setting the DSIEN bit of the DSI_WCR

	// Step 16: Optionally send DCS commands through the APB generic interface to configure the display

	// Step 17: Enable the LTDC in the LTDC

	// Step 18: Start the LTDC flow through the DSI wrapper (CR.LTDCEN = 1)
}

void Display::configure_ltdc()
{
	// Developed based on Chapter 17.6 of RM0386 Reference Manual

	// Step 1: Enable the LTDC clock in the RCC registe
	// Already enabled in DSI init procedure

	// Step 2: Configure the required pixel clock
	// Already done when PLLSAI was configured in DSI init procedure (or at least if it was configured)

	// Step 3: Configure the synchronous timings: VSYNC, HSYNC, vertical and horizontal back porch, active data area and the front porch timings following the panel datasheet
	hal::ltdc.set_timing_registers(display_timings);

	// Step 4: Configure the synchronous signals and clock polarity in the LTDC_GCR register
	// Let's hope defaults are good

	// Step 5: If needed, configure the background color in the LTDC_BCCR register
	// Not feeling very needy

	// Step 6: Configure the needed interrupts in the LTDC_IER and LTDC_LIPCR register
	hal::ltdc.enable_interrupts();

	// Step 7: Configure the layer1/2 parameters
	// Let's just set up one layer
	hal::ltdc.set_up_layer(display_timings, buffer.get_start_address());

	// Step 8: Enable layer1/2 and if needed the CLUT in the LTDC_LxCR register
	hal::ltdc.enable_layer();

	// Step 9: If needed, enable dithering and color keying respectively in the LTDC_GCR and LTDC_LxCKCR registers. They can be also enabled on the fly
	// Not feeling very needy

	// Step 10: Reload the shadow registers to active register through the LTDC_SRCR register
	hal::ltdc.reload();

	// Step 11: Enable the LCD-TFT controller in the LTDC_GCR register
	hal::ltdc.enable();

	// "All layer parameters can be modified on the fly except the CLUT. The new configuration has to be either reloaded immediately or during vertical blanking period by configuring the
	// LTDC_SRCR register"
}

}  // namespace bsp