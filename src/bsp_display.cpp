#include "bsp_display.hpp"

#include "hal_dsi.hpp"
#include "hal_gpio.hpp"
#include "hal_ltdc.hpp"
#include "hal_rcc.hpp"

#include <cstddef>

namespace bsp
{

namespace
{

constexpr uint8_t lcd_reset_pin = 7;
constexpr uint32_t lcd_reset_delay_cycles = 320000;
constexpr uint32_t lcd_sleep_out_delay_cycles = lcd_reset_delay_cycles * 6;

constexpr uint32_t dsi_virtual_channel_id = 0;
constexpr uint32_t dsi_lane_byte_clock_khz = 62500;
constexpr uint32_t ltdc_pixel_clock_khz = 27429;
constexpr uint32_t dsi_tx_escape_clock_divider = dsi_lane_byte_clock_khz / 15620;

constexpr uint8_t ltdc_background_red = 0x00;
constexpr uint8_t ltdc_background_green = 0x80;
constexpr uint8_t ltdc_background_blue = 0x20;

enum class LcdController
{
	OTM8009A,
	NT35510,
};

constexpr hal::DisplayTimings otm8009a_timings = {.hsync_width = 2,
                                                  .vsync_width = 1,
                                                  .horizontal_back_porch = 34,
                                                  .vertical_back_porch = 15,
                                                  .horizontal_front_porch = 34,
                                                  .vertical_front_porch = 16,
                                                  .active_width = 800,
                                                  .active_height = 480};

constexpr hal::DisplayTimings nt35510_timings = {.hsync_width = 2,
                                                 .vsync_width = 120,
                                                 .horizontal_back_porch = 34,
                                                 .vertical_back_porch = 150,
                                                 .horizontal_front_porch = 34,
                                                 .vertical_front_porch = 150,
                                                 .active_width = 800,
                                                 .active_height = 480};

constexpr hal::DsiPllConfig dsi_pll_config = {.input_divider = 2, .loop_multiplier = 125, .output_divider = 0};
constexpr hal::DsiPhyTimings dsi_phy_timings = {.clock_lane_hs_to_lp = 35,
                                                .clock_lane_lp_to_hs = 35,
                                                .data_lane_hs_to_lp = 35,
                                                .data_lane_lp_to_hs = 35,
                                                .data_lane_max_read = 0,
                                                .stop_wait = 10};

constexpr uint8_t otm8009a_cmd_nop = 0x00;
constexpr uint8_t otm8009a_cmd_read_id1 = 0xDA;
constexpr uint8_t otm8009a_cmd_sleep_out = 0x11;
constexpr uint8_t otm8009a_cmd_display_on = 0x29;
constexpr uint8_t otm8009a_cmd_column_address_set = 0x2A;
constexpr uint8_t otm8009a_cmd_page_address_set = 0x2B;
constexpr uint8_t otm8009a_cmd_memory_write = 0x2C;
constexpr uint8_t otm8009a_cmd_memory_access_control = 0x36;
constexpr uint8_t otm8009a_cmd_color_mode = 0x3A;
constexpr uint8_t otm8009a_cmd_write_brightness = 0x51;
constexpr uint8_t otm8009a_cmd_write_control_display = 0x53;
constexpr uint8_t otm8009a_cmd_write_cabc = 0x55;
constexpr uint8_t otm8009a_cmd_write_cabc_minimum_brightness = 0x5E;

constexpr uint8_t otm8009a_id1 = 0x40;
constexpr uint8_t otm8009a_color_mode_rgb888 = 0x77;
constexpr uint8_t otm8009a_landscape_memory_access = 0x60;

constexpr uint8_t nt35510_id2 = 0x80;
constexpr uint8_t nt35510_cmd_read_id2 = 0xDB;
constexpr uint8_t nt35510_cmd_sleep_out = 0x11;
constexpr uint8_t nt35510_cmd_display_on = 0x29;
constexpr uint8_t nt35510_cmd_column_address_set = 0x2A;
constexpr uint8_t nt35510_cmd_page_address_set = 0x2B;
constexpr uint8_t nt35510_cmd_memory_write = 0x2C;
constexpr uint8_t nt35510_cmd_memory_access_control = 0x36;
constexpr uint8_t nt35510_cmd_color_mode = 0x3A;
constexpr uint8_t nt35510_cmd_read_display_power_mode = 0x0A;
constexpr uint8_t nt35510_cmd_read_display_madctl = 0x0B;
constexpr uint8_t nt35510_cmd_read_display_pixel_format = 0x0C;
constexpr uint8_t nt35510_cmd_write_brightness = 0x51;
constexpr uint8_t nt35510_cmd_write_control_display = 0x53;
constexpr uint8_t nt35510_cmd_write_cabc = 0x55;
constexpr uint8_t nt35510_cmd_write_cabc_minimum_brightness = 0x5E;

constexpr uint8_t nt35510_color_mode_rgb888 = 0x77;
constexpr uint8_t nt35510_landscape_memory_access = 0x60;

constexpr uint8_t lcd_reg_data1[] = {0x80, 0x09, 0x01};
constexpr uint8_t lcd_reg_data2[] = {0x80, 0x09};
constexpr uint8_t lcd_reg_data3[] = {0x00, 0x09, 0x0F, 0x0E, 0x07, 0x10, 0x0B, 0x0A, 0x04, 0x07, 0x0B, 0x08, 0x0F, 0x10, 0x0A, 0x01};
constexpr uint8_t lcd_reg_data4[] = {0x00, 0x09, 0x0F, 0x0E, 0x07, 0x10, 0x0B, 0x0A, 0x04, 0x07, 0x0B, 0x08, 0x0F, 0x10, 0x0A, 0x01};
constexpr uint8_t lcd_reg_data5[] = {0x79, 0x79};
constexpr uint8_t lcd_reg_data6[] = {0x00, 0x01};
constexpr uint8_t lcd_reg_data7[] = {0x85, 0x01, 0x00, 0x84, 0x01, 0x00};
constexpr uint8_t lcd_reg_data8[] = {0x18, 0x04, 0x03, 0x39, 0x00, 0x00, 0x00, 0x18, 0x03, 0x03, 0x3A, 0x00, 0x00, 0x00};
constexpr uint8_t lcd_reg_data9[] = {0x18, 0x02, 0x03, 0x3B, 0x00, 0x00, 0x00, 0x18, 0x01, 0x03, 0x3C, 0x00, 0x00, 0x00};
constexpr uint8_t lcd_reg_data10[] = {0x01, 0x01, 0x20, 0x20, 0x00, 0x00, 0x01, 0x02, 0x00, 0x00};
constexpr uint8_t lcd_reg_data11[10] = {};
constexpr uint8_t lcd_reg_data12[15] = {};
constexpr uint8_t lcd_reg_data13[15] = {};
constexpr uint8_t lcd_reg_data14[10] = {};
constexpr uint8_t lcd_reg_data15[] = {0x00, 0x04, 0x04, 0x04, 0x04, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
constexpr uint8_t lcd_reg_data16[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x04, 0x04, 0x04, 0x04, 0x04, 0x00, 0x00, 0x00, 0x00};
constexpr uint8_t lcd_reg_data17[10] = {};
constexpr uint8_t lcd_reg_data18[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
constexpr uint8_t lcd_reg_data19[] = {0x00, 0x26, 0x09, 0x0B, 0x01, 0x25, 0x00, 0x00, 0x00, 0x00};
constexpr uint8_t lcd_reg_data20[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x26, 0x0A, 0x0C, 0x02};
constexpr uint8_t lcd_reg_data21[] = {0x25, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
constexpr uint8_t lcd_reg_data22[] = {0x00, 0x25, 0x0C, 0x0A, 0x02, 0x26, 0x00, 0x00, 0x00, 0x00};
constexpr uint8_t lcd_reg_data23[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x25, 0x0B, 0x09, 0x01};
constexpr uint8_t lcd_reg_data24[] = {0x26, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
constexpr uint8_t lcd_reg_data25[] = {0xFF, 0xFF, 0xFF};
constexpr uint8_t lcd_reg_data26[] = {0x00, 0x00, 0x03, 0x1F};
constexpr uint8_t lcd_reg_data27[] = {0x00, 0x00, 0x01, 0xDF};

constexpr uint8_t short_reg_data[] = {
    0x00, 0x00, 0x80, 0x30, 0x8A, 0x40, 0xB1, 0xA9, 0x91, 0x34, 0xB4, 0x50, 0x4E, 0x81, 0x66, 0xA1, 0x08,
    0x92, 0x01, 0x95, 0x94, 0x33, 0xA3, 0x1B, 0x82, 0x83, 0x83, 0x0E, 0xA6, 0xA0, 0xB0, 0xC0, 0xD0, 0x90,
    0xE0, 0xF0, 0x00, 0x55, otm8009a_color_mode_rgb888, 0x7F, 0x2C, 0x02, 0xFF, 0x00, 0x00, 0x00, 0x66, 0xB6, 0x06,
    0xB1, 0x06};

constexpr uint8_t nt35510_reg_data1[] = {0x55, 0xAA, 0x52, 0x08, 0x01};
constexpr uint8_t nt35510_reg_data2[] = {0x03, 0x03, 0x03};
constexpr uint8_t nt35510_reg_data3[] = {0x46, 0x46, 0x46};
constexpr uint8_t nt35510_reg_data4[] = {0x36, 0x36, 0x36};
constexpr uint8_t nt35510_reg_data5[] = {0x00, 0x00, 0x02};
constexpr uint8_t nt35510_reg_data6[] = {0x26, 0x26, 0x26};
constexpr uint8_t nt35510_reg_data7[] = {0x01};
constexpr uint8_t nt35510_reg_data8[] = {0x09, 0x09, 0x09};
constexpr uint8_t nt35510_reg_data9[] = {0x08, 0x08, 0x08};
constexpr uint8_t nt35510_reg_data10[] = {0x00, 0x80, 0x00};
constexpr uint8_t nt35510_reg_data11[] = {0x00, 0x50};
constexpr uint8_t nt35510_reg_data12[] = {0x55, 0xAA, 0x52, 0x08, 0x00};
constexpr uint8_t nt35510_reg_data13[] = {0xFC, 0x00};
constexpr uint8_t nt35510_reg_data14[] = {0x03};
constexpr uint8_t nt35510_reg_data15[] = {0x50};
constexpr uint8_t nt35510_reg_data16[] = {0x00, 0x00};
constexpr uint8_t nt35510_reg_data17[] = {0x01, 0x02, 0x02, 0x02};
constexpr uint8_t nt35510_reg_data18[] = {0x00, 0x00, 0x00};
constexpr uint8_t nt35510_reg_data19[] = {0x03, 0x00, 0x00};
constexpr uint8_t nt35510_column_address[] = {0x00, 0x00, 0x03, 0x1F};
constexpr uint8_t nt35510_page_address[] = {0x00, 0x00, 0x01, 0xDF};

volatile uint8_t lcd_detected_controller = 0;
volatile uint8_t lcd_nt35510_read_id2 = 0;
volatile uint8_t lcd_otm8009a_read_id1 = 0;
volatile uint8_t lcd_panel_init_failed = 0;
volatile uint8_t lcd_power_mode = 0;
volatile uint8_t lcd_memory_access = 0;
volatile uint8_t lcd_pixel_format = 0;

hal::DsiVideoConfig make_dsi_video_config(const hal::DisplayTimings& timings)
{
	return {.virtual_channel_id = dsi_virtual_channel_id,
	        .active_width = timings.active_width,
	        .active_height = timings.active_height,
	        .hsync = timings.hsync_width,
	        .horizontal_back_porch = timings.horizontal_back_porch,
	        .horizontal_front_porch = timings.horizontal_front_porch,
	        .vsync = timings.vsync_width,
	        .vertical_back_porch = timings.vertical_back_porch,
	        .vertical_front_porch = timings.vertical_front_porch,
	        .lane_byte_clock_khz = dsi_lane_byte_clock_khz,
	        .pixel_clock_khz = ltdc_pixel_clock_khz};
}

void busy_wait_cycles(uint32_t cycles)
{
	while (cycles > 0)
	{
		asm volatile("" ::: "memory");
		cycles--;
	}
}

void reset_lcd_panel()
{
	hal::rcc.enable_gpio_h();

	hal::gpio_h.set_mode(lcd_reset_pin, hal::Gpio::Mode::OUTPUT);
	hal::gpio_h.set_output_type(lcd_reset_pin, hal::Gpio::OutputType::PUSHPULL);
	hal::gpio_h.set_speed(lcd_reset_pin, hal::Gpio::Speed::HIGH);
	hal::gpio_h.set_pull(lcd_reset_pin, hal::Gpio::Pull::NOPULLUP_PULLDOWN);

	hal::gpio_h.set_output_low(lcd_reset_pin);
	busy_wait_cycles(lcd_reset_delay_cycles);

	hal::gpio_h.set_output_high(lcd_reset_pin);
	busy_wait_cycles(lcd_reset_delay_cycles);
}

bool write_panel_command(uint8_t command, const uint8_t* parameters, uint32_t parameter_count)
{
	if (parameter_count == 0)
	{
		return hal::dsi.dcs_short_write(dsi_virtual_channel_id, command, *parameters);
	}

	return hal::dsi.dcs_long_write(dsi_virtual_channel_id, command, parameters, parameter_count);
}

template <std::size_t N>
bool write_panel_command(uint8_t command, const uint8_t (&parameters)[N])
{
	return write_panel_command(command, parameters, N);
}

bool write_panel_parameter(uint8_t command, uint8_t parameter)
{
	return hal::dsi.dcs_short_write(dsi_virtual_channel_id, command, parameter);
}

LcdController detect_lcd_controller()
{
	uint8_t id = 0;
	if (hal::dsi.dcs_read(dsi_virtual_channel_id, nt35510_cmd_read_id2, &id, 1))
	{
		lcd_nt35510_read_id2 = id;
		if (id == nt35510_id2)
		{
			lcd_detected_controller = 2;
			return LcdController::NT35510;
		}
	}

	id = 0;
	if (hal::dsi.dcs_read(dsi_virtual_channel_id, otm8009a_cmd_read_id1, &id, 1))
	{
		lcd_otm8009a_read_id1 = id;
		if (id == otm8009a_id1)
		{
			lcd_detected_controller = 1;
			return LcdController::OTM8009A;
		}
	}

	lcd_detected_controller = 2;
	return LcdController::NT35510;
}

bool init_otm8009a()
{
	bool ok = true;
	auto write_parameter = [&ok](uint8_t command, uint8_t parameter) { ok = ok && write_panel_parameter(command, parameter); };
	auto write_block = [&ok](uint8_t command, auto const& parameters) { ok = ok && write_panel_command(command, parameters); };

	// Vendor power, timing, and gamma setup from ST's OTM8009A component driver.
	write_parameter(otm8009a_cmd_nop, short_reg_data[1]);
	write_block(0xFF, lcd_reg_data1);
	write_parameter(otm8009a_cmd_nop, short_reg_data[2]);
	write_block(0xFF, lcd_reg_data2);
	write_parameter(otm8009a_cmd_nop, short_reg_data[2]);
	write_parameter(0xC4, short_reg_data[3]);
	busy_wait_cycles(lcd_reset_delay_cycles / 2);
	write_parameter(otm8009a_cmd_nop, short_reg_data[4]);
	write_parameter(0xC4, short_reg_data[5]);
	busy_wait_cycles(lcd_reset_delay_cycles / 2);
	write_parameter(otm8009a_cmd_nop, short_reg_data[6]);
	write_parameter(0xC5, short_reg_data[7]);
	write_parameter(otm8009a_cmd_nop, short_reg_data[8]);
	write_parameter(0xC5, short_reg_data[9]);
	write_parameter(otm8009a_cmd_nop, short_reg_data[10]);
	write_parameter(0xC0, short_reg_data[11]);
	write_parameter(otm8009a_cmd_nop, short_reg_data[1]);
	write_parameter(0xD9, short_reg_data[12]);
	write_parameter(otm8009a_cmd_nop, short_reg_data[13]);
	write_parameter(0xC1, short_reg_data[14]);
	write_parameter(otm8009a_cmd_nop, short_reg_data[15]);
	write_parameter(0xC1, short_reg_data[16]);
	write_parameter(otm8009a_cmd_nop, short_reg_data[17]);
	write_parameter(0xC5, short_reg_data[18]);
	write_parameter(otm8009a_cmd_nop, short_reg_data[19]);
	write_parameter(0xC5, short_reg_data[9]);
	write_parameter(otm8009a_cmd_nop, short_reg_data[1]);
	write_block(0xD8, lcd_reg_data5);
	write_parameter(otm8009a_cmd_nop, short_reg_data[20]);
	write_parameter(0xC5, short_reg_data[21]);
	write_parameter(otm8009a_cmd_nop, short_reg_data[22]);
	write_parameter(0xC0, short_reg_data[23]);
	write_parameter(otm8009a_cmd_nop, short_reg_data[24]);
	write_parameter(0xC5, short_reg_data[25]);
	write_parameter(otm8009a_cmd_nop, short_reg_data[13]);
	write_parameter(0xC4, short_reg_data[26]);
	write_parameter(otm8009a_cmd_nop, short_reg_data[15]);
	write_parameter(0xC1, short_reg_data[27]);
	write_parameter(otm8009a_cmd_nop, short_reg_data[28]);
	write_block(0xB3, lcd_reg_data6);
	write_parameter(otm8009a_cmd_nop, short_reg_data[2]);
	write_block(0xCE, lcd_reg_data7);
	write_parameter(otm8009a_cmd_nop, short_reg_data[29]);
	write_block(0xCE, lcd_reg_data8);
	write_parameter(otm8009a_cmd_nop, short_reg_data[30]);
	write_block(0xCE, lcd_reg_data9);
	write_parameter(otm8009a_cmd_nop, short_reg_data[31]);
	write_block(0xCF, lcd_reg_data10);
	write_parameter(otm8009a_cmd_nop, short_reg_data[32]);
	write_parameter(0xCF, short_reg_data[45]);
	write_parameter(otm8009a_cmd_nop, short_reg_data[2]);
	write_block(0xCB, lcd_reg_data11);
	write_parameter(otm8009a_cmd_nop, short_reg_data[33]);
	write_block(0xCB, lcd_reg_data12);
	write_parameter(otm8009a_cmd_nop, short_reg_data[29]);
	write_block(0xCB, lcd_reg_data13);
	write_parameter(otm8009a_cmd_nop, short_reg_data[30]);
	write_block(0xCB, lcd_reg_data14);
	write_parameter(otm8009a_cmd_nop, short_reg_data[31]);
	write_block(0xCB, lcd_reg_data15);
	write_parameter(otm8009a_cmd_nop, short_reg_data[32]);
	write_block(0xCB, lcd_reg_data16);
	write_parameter(otm8009a_cmd_nop, short_reg_data[34]);
	write_block(0xCB, lcd_reg_data17);
	write_parameter(otm8009a_cmd_nop, short_reg_data[35]);
	write_block(0xCB, lcd_reg_data18);
	write_parameter(otm8009a_cmd_nop, short_reg_data[2]);
	write_block(0xCC, lcd_reg_data19);
	write_parameter(otm8009a_cmd_nop, short_reg_data[33]);
	write_block(0xCC, lcd_reg_data20);
	write_parameter(otm8009a_cmd_nop, short_reg_data[29]);
	write_block(0xCC, lcd_reg_data21);
	write_parameter(otm8009a_cmd_nop, short_reg_data[30]);
	write_block(0xCC, lcd_reg_data22);
	write_parameter(otm8009a_cmd_nop, short_reg_data[31]);
	write_block(0xCC, lcd_reg_data23);
	write_parameter(otm8009a_cmd_nop, short_reg_data[32]);
	write_block(0xCC, lcd_reg_data24);
	write_parameter(otm8009a_cmd_nop, short_reg_data[13]);
	write_parameter(0xC5, short_reg_data[46]);
	write_parameter(otm8009a_cmd_nop, short_reg_data[47]);
	write_parameter(0xF5, short_reg_data[48]);
	write_parameter(otm8009a_cmd_nop, short_reg_data[49]);
	write_parameter(0xC6, short_reg_data[50]);
	write_parameter(otm8009a_cmd_nop, short_reg_data[1]);
	write_block(0xFF, lcd_reg_data25);
	write_parameter(otm8009a_cmd_nop, short_reg_data[1]);
	write_parameter(otm8009a_cmd_nop, short_reg_data[1]);
	write_block(0xE1, lcd_reg_data3);
	write_parameter(otm8009a_cmd_nop, short_reg_data[1]);
	write_block(0xE2, lcd_reg_data4);

	write_parameter(otm8009a_cmd_sleep_out, short_reg_data[36]);
	busy_wait_cycles(lcd_sleep_out_delay_cycles);
	write_parameter(otm8009a_cmd_color_mode, short_reg_data[38]);
	write_parameter(otm8009a_cmd_memory_access_control, otm8009a_landscape_memory_access);
	write_block(otm8009a_cmd_column_address_set, lcd_reg_data26);
	write_block(otm8009a_cmd_page_address_set, lcd_reg_data27);
	write_parameter(otm8009a_cmd_write_brightness, short_reg_data[39]);
	write_parameter(otm8009a_cmd_write_control_display, short_reg_data[40]);
	write_parameter(otm8009a_cmd_write_cabc, short_reg_data[41]);
	write_parameter(otm8009a_cmd_write_cabc_minimum_brightness, short_reg_data[42]);
	write_parameter(otm8009a_cmd_display_on, short_reg_data[43]);
	write_parameter(otm8009a_cmd_nop, short_reg_data[1]);
	write_parameter(otm8009a_cmd_memory_write, short_reg_data[44]);

	return ok;
}

bool init_nt35510()
{
	bool ok = true;
	auto write_parameter = [&ok](uint8_t command, uint8_t parameter) { ok = ok && write_panel_parameter(command, parameter); };
	auto write_block = [&ok](uint8_t command, auto const& parameters) { ok = ok && write_panel_command(command, parameters); };

	write_block(0xF0, nt35510_reg_data1);
	write_block(0xB0, nt35510_reg_data2);
	write_block(0xB6, nt35510_reg_data3);
	write_block(0xB1, nt35510_reg_data2);
	write_block(0xB7, nt35510_reg_data4);
	write_block(0xB2, nt35510_reg_data5);
	write_block(0xB8, nt35510_reg_data6);
	write_block(0xBF, nt35510_reg_data7);
	write_block(0xB3, nt35510_reg_data8);
	write_block(0xB9, nt35510_reg_data4);
	write_block(0xB5, nt35510_reg_data9);
	write_block(0xBA, nt35510_reg_data6);
	write_block(0xBC, nt35510_reg_data10);
	write_block(0xBD, nt35510_reg_data10);
	write_block(0xBE, nt35510_reg_data11);

	write_block(0xF0, nt35510_reg_data12);
	write_block(0xB1, nt35510_reg_data13);
	write_block(0xB6, nt35510_reg_data14);
	write_block(0xB5, nt35510_reg_data15);
	write_block(0xB7, nt35510_reg_data16);
	write_block(0xB8, nt35510_reg_data17);
	write_block(0xBC, nt35510_reg_data18);
	write_block(0xCC, nt35510_reg_data19);
	write_block(0xBA, nt35510_reg_data7);

	busy_wait_cycles(lcd_reset_delay_cycles * 10);

	write_parameter(nt35510_cmd_memory_access_control, nt35510_landscape_memory_access);
	write_block(nt35510_cmd_column_address_set, nt35510_column_address);
	write_block(nt35510_cmd_page_address_set, nt35510_page_address);

	write_parameter(nt35510_cmd_sleep_out, 0x00);
	busy_wait_cycles(lcd_reset_delay_cycles);

	write_parameter(nt35510_cmd_color_mode, nt35510_color_mode_rgb888);
	write_parameter(nt35510_cmd_write_brightness, 0x7F);
	write_parameter(nt35510_cmd_write_control_display, 0x2C);
	write_parameter(nt35510_cmd_write_cabc, 0x02);
	write_parameter(nt35510_cmd_write_cabc_minimum_brightness, 0xFF);
	write_parameter(nt35510_cmd_display_on, 0x00);
	write_parameter(nt35510_cmd_memory_write, 0x00);

	uint8_t readback = 0;
	if (hal::dsi.dcs_read(dsi_virtual_channel_id, nt35510_cmd_read_display_power_mode, &readback, 1))
	{
		lcd_power_mode = readback;
	}

	readback = 0;
	if (hal::dsi.dcs_read(dsi_virtual_channel_id, nt35510_cmd_read_display_madctl, &readback, 1))
	{
		lcd_memory_access = readback;
	}

	readback = 0;
	if (hal::dsi.dcs_read(dsi_virtual_channel_id, nt35510_cmd_read_display_pixel_format, &readback, 1))
	{
		lcd_pixel_format = readback;
	}

	return ok;
}

}  // namespace

FrameBuffer& Display::get_buffer()
{
	return buffer;
}

void Display::init()
{
	buffer.fill_test_pattern();

	hal::rcc.enable_hse();
	while (!hal::rcc.is_hse_ready())
	{
		;
	}

	hal::rcc.enable_dsi();
	hal::rcc.enable_ltdc();
	hal::rcc.reset_dsi();
	hal::rcc.reset_ltdc();

	hal::rcc.configure_pllsai_for_ltdc_27mhz();
	hal::rcc.enable_pllsai();
	while (!hal::rcc.is_pllsai_ready())
	{
		;
	}

	reset_lcd_panel();

	if (!hal::dsi.init_two_lane_video_host(dsi_pll_config, dsi_tx_escape_clock_divider))
	{
		return;
	}

	hal::dsi.configure_commands_in_low_power();
	hal::dsi.enable_bus_turnaround();
	hal::dsi.start();

	const LcdController lcd_controller = detect_lcd_controller();
	reset_lcd_panel();
	hal::dsi.stop();

	const hal::DisplayTimings& timings = (lcd_controller == LcdController::NT35510) ? nt35510_timings : otm8009a_timings;
	hal::dsi.configure_video_mode(make_dsi_video_config(timings));
	hal::dsi.configure_phy_timings(dsi_phy_timings);

	configure_ltdc(timings);

	hal::dsi.start();
	hal::dsi.configure_commands_in_low_power();

	const bool panel_ready = (lcd_controller == LcdController::NT35510) ? init_nt35510() : init_otm8009a();
	if (!panel_ready)
	{
		lcd_panel_init_failed = 1;
		return;
	}

	hal::dsi.enable_ltdc_flow();
}

void Display::configure_ltdc(const hal::DisplayTimings& timings)
{
	hal::ltdc.set_timing_registers(timings);
	hal::ltdc.set_dsi_video_polarities();
	hal::ltdc.set_background_color(ltdc_background_red, ltdc_background_green, ltdc_background_blue);

	hal::ltdc.configure_argb8888_layer(timings, buffer.get_start_address(), FrameBuffer::width, FrameBuffer::height);
	hal::ltdc.enable_layer();
	hal::ltdc.reload();
	hal::ltdc.enable();
}

}  // namespace bsp
