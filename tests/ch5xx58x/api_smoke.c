/*
 * This file is part of the libopenwch project.
 *
 * Copyright (C) 2025 libopenwch contributors
 *
 * This library is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library.  If not, see <http://www.gnu.org/licenses/>.
 */

/*
 * CH58x API smoke test.
 *
 * Calls every public function in every peripheral driver and touches every
 * register accessor, so that the whole public API is proven to compile and
 * link together and no header depends on another having been included first.
 *
 * It is a compile-and-link test, not a functional test.  main() deliberately
 * does NOT call the test body: on real hardware it would reconfigure every
 * peripheral, power the crystal, and possibly park flash.
 */

#include <libopenwch/ble/ble.h>
#include <libopenwch/ch5xx58x/adc.h>
#include <libopenwch/ch5xx58x/clk.h>
#include <libopenwch/ch5xx58x/flash.h>
#include <libopenwch/ch5xx58x/gpio.h>
#include <libopenwch/ch5xx58x/i2c.h>
#include <libopenwch/ch5xx58x/pwm.h>
#include <libopenwch/ch5xx58x/pwr.h>
#include <libopenwch/ch5xx58x/rwa.h>
#include <libopenwch/ch5xx58x/spi.h>
#include <libopenwch/ch5xx58x/sys.h>
#include <libopenwch/ch5xx58x/tmr.h>
#include <libopenwch/ch5xx58x/uart.h>

void api_smoke(void);

int main(void) {
	/*
	 * Take the address of the test body so that --gc-sections cannot
	 * discard it.  The volatile pointer is never dereferenced.
	 */
	void (*volatile body)(void) = api_smoke;

	(void)body;

	for (;;) {
		;
	}
}

void api_smoke(void) {
	uint8_t buf[8];
	uint32_t saved;

	/* --- rwa --- */
	rwa_open();
	(void)rwa_is_unlocked();
	rwa_close();
	{
		uint32_t before = rwa_unlock();

		(void)before;
		rwa_lock(before);
	}
	RWA_WRITE(MMIO8(R8_GLOB_RESET_KEEP), 0);
	RWA_SET_BITS(MMIO8(R8_GLOB_RESET_KEEP), 1);
	RWA_CLEAR_BITS(MMIO8(R8_GLOB_RESET_KEEP), 1);
	RWA_MODIFY(MMIO8(R8_GLOB_RESET_KEEP), 0x3u, 1);
	RWA_WRITE8(MMIO8(R8_GLOB_RESET_KEEP), 0);

	/* --- clk --- */
	clk_hse_enable();
	clk_hse_disable();
	clk_pll_enable();
	clk_pll_disable();
	clk_lse_enable();
	clk_lsi_enable();
	clk_lse_disable();
	clk_lsi_disable();
	clk_hse_set_current(CLK_HSE_CURRENT_50MA);
	clk_hse_set_capacitance(CLK_HSE_CAP_16PF);
	clk_lse_set_current(CLK_HSE_CURRENT_25MA);
	clk_lse_set_capacitance(CLK_HSE_CAP_10PF);
	clk_32k_select(CLK_SOURCE_LSE);
	clk_set_sys_clock(CLK_SOURCE_PLL_60MHZ);
	clk_set_sys_clock(CLK_SOURCE_HSE_8MHZ);
	(void)clk_get_sys_clock();
	clk_set_sys_clock_48mhz();

	/* --- gpio --- */
	gpio_set_mode(GPIOA, GPIO_MODE_OUTPUT_PP_5MA, GPIO1);
	gpio_set_mode(GPIOB, GPIO_MODE_INPUT_PU, GPIO4);
	gpio_set(GPIOA, GPIO1);
	gpio_clear(GPIOA, GPIO1);
	gpio_toggle(GPIOA, GPIO1);
	(void)gpio_get(GPIOA, GPIO1);
	(void)gpio_port_read(GPIOA);
	gpio_port_write(GPIOA, 0);
	gpio_set_irq_mode(GPIOA, GPIO1, GPIO_IRQ_RISING_EDGE);
	(void)gpio_get_irq_flag(GPIOA);
	gpio_clear_irq_flag(GPIOA, GPIO1);
	gpio_pin_remap(GPIO_REMAP_UART1);
	gpio_analog_enable(GPIO_ANALOG_ADC0);
	gpio_analog_disable(GPIO_ANALOG_ADC0);

	/* --- sys --- */
	(void)sys_get_systick_count();
	(void)sys_get_chip_id();
	(void)sys_get_reset_status();
	(void)sys_reset_keep();
	sys_set_reset_keep(0);
	sys_disable_all_irq(&saved);
	sys_recover_irq(saved);

	/* --- pwr --- */
	pwr_enable_dcdc();
	pwr_disable_dcdc();
	pwr_set_unit(true, PWR_UNIT_HSE);
	pwr_periph_clock_enable(PWR_CLK_UART1);
	pwr_periph_clock_disable(PWR_CLK_UART1);
	pwr_set_wakeup(true, PWR_CLK_UART1, PWR_WAKEUP_DELAY_LONG);
	pwr_enable_voltage_monitor(PWR_VOLTAGE_MONITOR_2V5);
	pwr_disable_voltage_monitor();
	(void)pwr_get_flag(PWR_FLAG_BAT_LOW);
	pwr_enter_idle();
	pwr_enter_halt();
	pwr_enter_sleep(PWR_RETAIN_CORE);
	pwr_enter_shutdown(PWR_RETAIN_CORE);

	/* --- flash --- */
	flash_get_unique_id(buf);
	(void)flash_get_chip_id();
	flash_read(0, buf, sizeof(buf));
	flash_rom_read(0, buf, sizeof(buf));
	flash_set_latency(FLASH_LATENCY_PLL);
	(void)flash_erase_page(0);
	(void)flash_program(0, buf, sizeof(buf));

	/* --- uart --- */
	uart_set_baudrate(UART1, 115200);
	uart_set_databits(UART1, UART_DATA_8BITS);
	uart_set_stopbits(UART1, UART_STOPBITS_1);
	uart_set_parity(UART1, UART_PARITY_NONE);
	uart_enable(UART1);
	uart_send(UART1, 'x');
	(void)uart_recv(UART1);
	uart_send_blocking(UART1, 'y');
	(void)uart_recv_blocking(UART1);
	uart_write(UART1, buf, 1);
	uart_read(UART1, buf, 1);
	uart_enable_rx_interrupt(UART1);
	uart_disable_rx_interrupt(UART1);
	uart_enable_tx_interrupt(UART1);
	uart_disable_tx_interrupt(UART1);
	uart_enable_rx_dma(UART1);
	uart_disable_rx_dma(UART1);
	uart_clear_rx_fifo(UART1);
	uart_clear_tx_fifo(UART1);
	uart_set_fifo_trigger(UART1, UART_FIFO_TRIGGER_1);
	(void)uart_get_flag(UART1, UART_FLAG_TX_FIFO_EMPTY);
	uart_disable(UART1);

	/* --- spi --- */
	spi_init_master(SPI0, 4, SPI_MODE0_MSB);
	spi_init_slave(SPI0, SPI_MODE0_MSB);
	spi_set_clock_divider(SPI0, 8);
	spi_enable(SPI0);
	spi_send(SPI0, 0);
	(void)spi_recv(SPI0);
	(void)spi_xfer(SPI0, 0x5a);
	spi_write(SPI0, buf, 2);
	spi_read(SPI0, buf, 2);
	spi_master_write(SPI0, buf, 2);
	spi_master_read(SPI0, buf, 2);
	spi_enable_rx_dma(SPI0);
	spi_enable_tx_dma(SPI0);
	spi_enable_irq(SPI0, SPI_IRQ_BYTE_END);
	spi_disable_irq(SPI0, SPI_IRQ_BYTE_END);
	(void)spi_get_flag(SPI0, SPI_FLAG_BYTE_END);
	spi_clear_flag(SPI0, SPI_FLAG_BYTE_END);
	spi_disable(SPI0);

	/* --- i2c --- */
	i2c_init_master(I2C1, I2C_SPEED_STANDARD);
	i2c_init_slave(I2C1, 0x20);
	i2c_set_clock_frequency(I2C1);
	i2c_enable(I2C1);
	i2c_disable(I2C1);
	i2c_send_start(I2C1);
	i2c_send_stop(I2C1);
	i2c_send_data(I2C1, 0);
	(void)i2c_read_data(I2C1);
	i2c_send_7bit_address(I2C1, 0x20, 0);
	i2c_set_own_7bit_address(I2C1, 0x20);
	i2c_set_own_10bit_address(I2C1, 0x20);
	i2c_enable_dual_address(I2C1, 0x21);
	i2c_enable_general_call(I2C1);
	i2c_enable_ack(I2C1);
	i2c_disable_ack(I2C1);
	i2c_enable_pec(I2C1);
	i2c_disable_pec(I2C1);
	(void)i2c_get_pec(I2C1);
	i2c_software_reset(I2C1);
	i2c_enable_interrupt(I2C1, I2C_IT_EVT);
	i2c_disable_interrupt(I2C1, I2C_IT_EVT);
	(void)i2c_get_flag(I2C1, I2C_FLAG_SB);
	i2c_clear_flag(I2C1, I2C_FLAG_AF);
	(void)i2c_get_interrupt_status(I2C1);
	i2c_clear_interrupt_pending_bit(I2C1);

	/* --- tmr --- */
	tmr_set_mode(TMR0, TMR_MODE_TIMER);
	tmr_enable(TMR0);
	tmr_disable(TMR0);
	tmr_set_period(TMR0, 1000);
	tmr_set_count(TMR0, 0);
	(void)tmr_get_count(TMR0);
	tmr_set_capture_mode(TMR0, TMR_CAPTURE_RISING);
	(void)tmr_get_capture(TMR0);
	tmr_set_pwm_polarity(TMR0, TMR_POLARITY_ACTIVE_HIGH);
	tmr_enable_pwm(TMR0);
	tmr_disable_pwm(TMR0);
	tmr_set_pwm_repeat(TMR0, TMR_PWM_REPEAT_1);
	tmr_enable_irq(TMR0, TMR_IE_ALL);
	tmr_disable_irq(TMR0, TMR_IE_ALL);
	(void)tmr_get_flag(TMR0, TMR_IF_ALL);
	tmr_clear_flag(TMR0, TMR_IF_ALL);
	tmr_enable_dma(TMR1, TMR_DMA_SINGLE, 0, 0xff);

	/* --- pwm --- */
	pwm_set_cycle(PWMX, PWM_CYCLE_256);
	pwm_set_clock_divider(PWMX, 8);
	pwm_set_channel(PWMX, PWM_CH4, 128);
	pwm_set_polarity(PWMX, PWM_CH4, PWM_POLARITY_ACTIVE_HIGH);
	pwm_enable_channel(PWMX, PWM_CH4);
	pwm_disable_channel(PWMX, PWM_CH4);
	pwm_enable_alternate(PWMX, PWM_CH4);

	/* --- adc --- */
	adc_init_single_channel(ADC, ADC_SAMPLE_CLK_3_2MHZ, ADC_PGA_1);
	adc_init_differential(ADC, ADC_SAMPLE_CLK_8MHZ, ADC_PGA_1);
	adc_init_temperature(ADC);
	adc_init_battery(ADC);
	adc_set_channel(ADC, ADC_CH_EXTIN0);
	adc_set_sample_clock(ADC, ADC_SAMPLE_CLK_4MHZ);
	adc_set_pga(ADC, ADC_PGA_1);
	adc_start(ADC);
	(void)adc_is_ready(ADC);
	(void)adc_read(ADC);
	adc_set_auto_cycle(ADC, 8);
	adc_enable_dma(ADC, ADC_DMA_MODE_SINGLE, 0, 0xff);
	adc_disable_dma(ADC);
	adc_enable_touchkey(ADC);
	adc_disable_touchkey(ADC);
	(void)adc_read_touchkey(ADC, 1, 1);
	(void)adc_to_celsius(0);

	/* --- ble (layer over WCH's closed-source stack) --- */
	{
		static uint32_t
		    ble_heap[BLE_HEAP_SIZE_DEFAULT / 4] OPENWCH_ALIGN4;
		static const uint8_t ble_mac[6] = {1, 2, 3, 4, 5, 6};
		static gapRolesCBs_t role_cbs;
		static gapBondCBs_t bond_cbs;
		static gattAttribute_t attrs[1];
		static gattServiceCBs_t svc_cbs;
		static gattCharCfg_t char_cfg[1];
		ble_config_t cfg;

		ble_config_default(&cfg);
		cfg.heap = ble_heap;
		cfg.heap_size = sizeof(ble_heap);
		cfg.mac = ble_mac;

		(void)ble_init(&cfg);
		(void)ble_version();

		/* tmos */
		(void)ble_tmos_task_register(0);
		(void)ble_tmos_event_set(BLE_TMOS_INVALID_TASK_ID, 0);
		(void)ble_tmos_task_start(BLE_TMOS_INVALID_TASK_ID, 0, 0);
		(void)ble_tmos_task_stop(BLE_TMOS_INVALID_TASK_ID, 0);
		(void)ble_tmos_message_allocate(0);
		(void)ble_tmos_message_receive(BLE_TMOS_INVALID_TASK_ID);
		(void)ble_tmos_message_free(0);
		ble_tmos_memcpy(ble_heap, ble_mac, sizeof(ble_mac));
		ble_tmos_memset(ble_heap, 0, sizeof(ble_mac));
		(void)ble_tmos_memcmp(ble_heap, ble_mac, sizeof(ble_mac));
		ble_tmos_process();

		/* gap */
		(void)ble_gap_set_param(BLE_GAP_PARAM_DISC_ADV_INT_MIN, 0);
		(void)ble_gap_role_peripheral_init();
		(void)ble_gap_role_peripheral_start_device(
		    BLE_TMOS_INVALID_TASK_ID, &bond_cbs, &role_cbs);
		(void)ble_gap_role_set_param(BLE_GAP_ROLE_PARAM_ADVERT_ENABLED,
					     1, &cfg);
		(void)ble_gap_role_get_param(BLE_GAP_ROLE_PARAM_STATE, &cfg);
		(void)ble_gap_role_terminate_link(BLE_CONN_HANDLE_INVALID);
		(void)ble_gap_role_conn_param_update(BLE_CONN_HANDLE_INVALID, 6,
						     12, 0, 100,
						     BLE_TMOS_INVALID_TASK_ID);
		(void)ble_gap_role_update_phy(BLE_CONN_HANDLE_INVALID, 0, 1, 1);
		(void)ble_gap_role_read_rssi(BLE_CONN_HANDLE_INVALID);

		/* gatt server */
		(void)ble_gatt_server_add_service(BLE_GATT_ALL_SERVICES);
		(void)ble_gatt_server_register_service(
		    attrs, BLE_GATT_ATTR_COUNT(attrs),
		    BLE_GATT_ENC_KEY_SIZE_NONE, &svc_cbs);
		ble_gatt_server_char_cfg_init(BLE_CONN_HANDLE_INVALID,
					      char_cfg);
		(void)ble_gatt_server_char_cfg_read(BLE_CONN_HANDLE_INVALID,
						    char_cfg);
		(void)ble_gatt_server_process_ccc_write(
		    BLE_CONN_HANDLE_INVALID, attrs, (uint8_t *)ble_mac, 2, 0,
		    BLE_GATT_CLIENT_CFG_NOTIFY);
	}
}
