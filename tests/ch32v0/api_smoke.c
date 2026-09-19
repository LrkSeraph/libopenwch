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
 * CH32V00x API smoke test.
 *
 * Calls every public function in every peripheral driver, and reads the
 * factory/status registers through every accessor, so that:
 *
 *   - the whole public API is proven to compile and link together,
 *   - the argument counts and types are what the headers claim,
 *   - no header depends on another having been included first,
 *   - the strict warning set used to build the library also passes here.
 *
 * It is a compile-and-link test, not a functional test: nothing here is
 * expected to produce meaningful hardware behaviour, and calling it on real
 * silicon would reconfigure peripherals and erase flash.  Do not run it.
 */

#include <libopenwch/ch32v0/adc.h>
#include <libopenwch/ch32v0/dbgmcu.h>
#include <libopenwch/ch32v0/dma.h>
#include <libopenwch/ch32v0/exti.h>
#include <libopenwch/ch32v0/flash.h>
#include <libopenwch/ch32v0/gpio.h>
#include <libopenwch/ch32v0/i2c.h>
#include <libopenwch/ch32v0/iwdg.h>
#include <libopenwch/ch32v0/opa.h>
#include <libopenwch/ch32v0/pwr.h>
#include <libopenwch/ch32v0/rcc.h>
#include <libopenwch/ch32v0/spi.h>
#include <libopenwch/ch32v0/tim.h>
#include <libopenwch/ch32v0/usart.h>
#include <libopenwch/ch32v0/wwdg.h>

void api_smoke(void);

/*
 * The library's reset path calls main(), so one has to exist for the link
 * test to succeed.  It deliberately does NOT call api_smoke(): doing so on
 * hardware would take the chip apart.
 */
int main(void) {
	/*
	 * Take the address of the test body so that --gc-sections cannot
	 * discard it.  The volatile pointer is never dereferenced: actually
	 * calling it would reconfigure every peripheral and erase flash.
	 */
	void (*volatile body)(void) = api_smoke;

	(void)body;

	for (;;) {
		;
	}
}

void api_smoke(void) {
	/* --- gpio / afio --- */
	gpio_set_mode(GPIOA, GPIO_MODE_OUT_PP, GPIO1);
	gpio_set(GPIOA, GPIO1);
	gpio_clear(GPIOA, GPIO1);
	gpio_toggle(GPIOA, GPIO1);
	(void)gpio_get(GPIOA, GPIO1);
	(void)gpio_port_read(GPIOA);
	gpio_port_write(GPIOA, 0);
	gpio_port_config_lock(GPIOA, GPIO1);
	gpio_primary_remap(GPIO_REMAP_SPI1);
	gpio_secondary_remap(GPIO_REMAP_SPI1);
	gpio_usart1_remap(GPIO_REMAP_USART1_FULL);
	gpio_i2c1_remap(GPIO_REMAP_I2C1_FULL);
	gpio_tim1_remap(GPIO_REMAP_TIM1_FULL);
	gpio_tim2_remap(GPIO_REMAP_TIM2_FULL);
	gpio_exti_select_source(EXTI1, GPIOA);

	/* --- rcc --- */
	rcc_clock_setup_hsi_48mhz();
	rcc_clock_setup_hse_48mhz();
	rcc_periph_clock_enable(RCC_GPIOA);
	rcc_periph_clock_disable(RCC_GPIOA);
	rcc_periph_reset_pulse(RCC_TIM2);
	rcc_periph_reset_hold(RCC_TIM2);
	rcc_periph_reset_release(RCC_TIM2);
	rcc_ahb_set_prescaler(RCC_CFGR0_HPRE_DIV1);
	rcc_apb1_set_prescaler(RCC_CFGR0_PPRE_DIV1);
	rcc_apb2_set_prescaler(RCC_CFGR0_PPRE_DIV1);
	rcc_adc_set_prescaler(RCC_CFGR0_ADCPRE_DIV2);
	rcc_set_pll_source(0);
	rcc_set_sysclk_source(RCC_CFGR0_SW_HSI);
	rcc_osc_on(RCC_OSC_HSI);
	rcc_wait_for_osc_ready(RCC_OSC_HSI);
	rcc_osc_off(RCC_OSC_HSE);
	rcc_clock_security_system_enable();
	rcc_clock_security_system_disable();
	rcc_clear_reset_flags();
	(void)rcc_get_reset_flags();
	{
		struct rcc_clock_scale clocks;

		rcc_get_clocks_freq(&clocks);
		(void)clocks.sysclk;
	}
	(void)rcc_get_sysclk_frequency();
	rcc_clock_setup_sysclk(RCC_SYSCLK_PLL_HSI_48MHZ);

	/* --- usart --- */
	usart_set_baudrate(USART1, 115200);
	usart_set_databits(USART1, 8);
	usart_set_stopbits(USART1, USART_STOPBITS_1);
	usart_set_parity(USART1, USART_PARITY_NONE);
	usart_set_mode(USART1, USART_MODE_TX_RX);
	usart_set_flow_control(USART1, USART_FLOWCONTROL_NONE);
	usart_set_clock(USART1, USART_CLOCK_DISABLE);
	usart_enable(USART1);
	usart_send(USART1, 'x');
	(void)usart_recv(USART1);
	usart_send_blocking(USART1, 'y');
	(void)usart_recv_blocking(USART1);
	{
		const uint8_t buf[1] = { 'z' };

		usart_write(USART1, buf, 1);
	}
	(void)usart_get_flag(USART1, USART_STATR_TXE);
	usart_clear_flag(USART1, USART_STATR_CTS);
	usart_enable_rx_interrupt(USART1);
	usart_disable_rx_interrupt(USART1);
	usart_enable_tx_interrupt(USART1);
	usart_disable_tx_interrupt(USART1);
	usart_enable_rx_dma(USART1);
	usart_disable_rx_dma(USART1);
	usart_enable_tx_dma(USART1);
	usart_disable_tx_dma(USART1);
	usart_disable(USART1);

	/* --- tim --- */
	timer_set_mode(TIM1, TIM_MODE_UP);
	timer_enable(TIM1);
	timer_disable(TIM1);
	timer_set_prescaler(TIM1, 1);
	timer_set_period(TIM1, 100);
	timer_set_counter(TIM1, 0);
	(void)timer_get_counter(TIM1);
	timer_set_alignment(TIM1, TIM_MODE_EDGE_ALIGNED);
	timer_set_direction(TIM1, 0);
	timer_enable_preload(TIM1);
	timer_disable_preload(TIM1);
	timer_set_clock_division(TIM1, TIM_CKD_DIV1);
	timer_generate_event(TIM1, TIM_EVENT_UPDATE);
	timer_set_oc_mode(TIM1, TIM_OC1, TIM_OC_MODE_PWM1);
	timer_set_oc_value(TIM1, TIM_OC1, 50);
	timer_set_oc_polarity(TIM1, TIM_OC1, TIM_OC_POLARITY_ACTIVE_HIGH);
	timer_enable_oc_output(TIM1, TIM_OC1);
	timer_disable_oc_output(TIM1, TIM_OC1);
	timer_enable_oc_preload(TIM1, TIM_OC1);
	timer_disable_oc_preload(TIM1, TIM_OC1);
	timer_set_oc_idle_state(TIM1, TIM_OC1, TIM_OC_IDLE_RESET);
	timer_set_input_filter(TIM1, TIM_IC1, 0);
	timer_set_input_polarity(TIM1, TIM_IC1, TIM_IC_POLARITY_RISING);
	timer_set_ic_prescaler(TIM1, TIM_IC1, 0);
	(void)timer_get_ic_value(TIM1, TIM_IC1);
	timer_enable_break_main_output(TIM1);
	timer_set_deadtime(TIM1, 0);
	timer_enable_irq(TIM1, TIM_IRQ_UPDATE);
	timer_disable_irq(TIM1, TIM_IRQ_UPDATE);
	(void)timer_get_flag(TIM1, TIM_FLAG_UPDATE);
	timer_clear_flag(TIM1, TIM_FLAG_UPDATE);
	(void)timer_get_interrupt_source(TIM1, TIM_DMAINTENR_UIE);

	/* --- spi --- */
	spi_init_master(SPI1, SPI_BAUDRATE_PRESCALER_8, SPI_CPOL_LOW,
			SPI_CPHA_FIRST, SPI_DFF_8BIT, SPI_BIT_ORDER_MSB_FIRST);
	spi_init_slave(SPI1, SPI_CPOL_LOW, SPI_CPHA_FIRST, SPI_DFF_8BIT,
		       SPI_BIT_ORDER_MSB_FIRST);
	spi_enable(SPI1);
	spi_send(SPI1, 0);
	(void)spi_recv(SPI1);
	(void)spi_xfer(SPI1, 0x5a);
	spi_set_baudrate_prescaler(SPI1, SPI_BAUDRATE_PRESCALER_16);
	spi_set_clock_polarity(SPI1, SPI_CPOL_HIGH);
	spi_set_clock_phase(SPI1, SPI_CPHA_SECOND);
	spi_set_dff(SPI1, SPI_DFF_16BIT);
	spi_set_bit_order(SPI1, SPI_BIT_ORDER_LSB_FIRST);
	spi_enable_software_slave_management(SPI1);
	spi_set_nss_high(SPI1);
	spi_set_nss_low(SPI1);
	spi_enable_ss_output(SPI1);
	spi_set_bidirectional_mode(SPI1);
	spi_set_bidirectional_transmit_only(SPI1);
	spi_set_crc_length(SPI1, SPI_CRC_LENGTH_8BIT);
	spi_enable_crc(SPI1);
	spi_disable_crc(SPI1);
	spi_set_crc_polynomial(SPI1, 7);
	(void)spi_get_tx_crc(SPI1);
	(void)spi_get_rx_crc(SPI1);
	spi_enable_rx_dma(SPI1);
	spi_enable_tx_dma(SPI1);
	spi_enable_irq(SPI1, SPI_IRQ_TXE);
	spi_disable_irq(SPI1, SPI_IRQ_TXE);
	(void)spi_get_flag(SPI1, SPI_STATR_TXE);
	spi_clear_flag(SPI1, SPI_STATR_CRCERR);
	spi_disable(SPI1);

	/* --- i2c --- */
	i2c_init_master(I2C1, 400000, I2C_SPEED_STANDARD, I2C_CCR_DUTY_2);
	i2c_init_slave(I2C1, 100000, 0x20);
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
	i2c_set_clock_frequency(I2C1, 400000);
	i2c_set_ccr(I2C1, 10);
	i2c_set_trise(I2C1, 5);
	i2c_enable_ack(I2C1);
	i2c_disable_ack(I2C1);
	i2c_nack_current(I2C1);
	i2c_nack_next(I2C1);
	i2c_enable_pec(I2C1);
	i2c_disable_pec(I2C1);
	(void)i2c_get_pec(I2C1);
	i2c_software_reset(I2C1);
	i2c_enable_interrupt(I2C1, I2C_IT_EVT);
	i2c_disable_interrupt(I2C1, I2C_IT_EVT);
	(void)i2c_get_flag(I2C1, I2C_STAR1_SB);
	i2c_clear_flag(I2C1, I2C_STAR1_AF);
	(void)i2c_get_interrupt_status(I2C1);
	i2c_clear_interrupt_pending_bit(I2C1);

	/* --- adc --- */
	adc_enable(ADC1);
	adc_disable(ADC1);
	adc_start_conversion_regular(ADC1);
	adc_start_conversion_injected(ADC1);
	adc_set_continuous_conversion_mode(ADC1);
	adc_set_single_conversion_mode(ADC1);
	adc_set_scan_mode(ADC1);
	adc_set_channel(ADC1, 0, 1);
	adc_set_sample_time(ADC1, 0, ADC_SAMPLETIME_57CYCLES);
	adc_set_sample_time_on_all_channels(ADC1, ADC_SAMPLETIME_73CYCLES);
	adc_set_right_aligned(ADC1);
	adc_set_external_trigger_regular(ADC1, ADC_EXTTRIG_REGULAR_NONE);
	adc_set_external_trigger_injected(ADC1, ADC_EXTTRIG_INJECTED_NONE);
	adc_enable_external_trigger_regular(ADC1);
	adc_disable_external_trigger_regular(ADC1);
	(void)adc_read_regular(ADC1);
	(void)adc_read_injected(ADC1, 1);
	adc_set_injected_offset(ADC1, 1, 0);
	adc_enable_dma(ADC1);
	adc_disable_dma(ADC1);
	adc_enable_temperature_sensor(ADC1);
	adc_enable_vrefint(ADC1);
	adc_reset_calibration(ADC1);
	adc_start_calibration(ADC1);
	(void)adc_is_calibration_complete(ADC1);
	adc_set_calibration_voltage(ADC1, ADC_CALVOL_75PERCENT);
	adc_set_external_trigger_delay(ADC1, ADC_DLYR_SOURCE_REGULAR, 0);
	adc_enable_analog_watchdog_regular(ADC1);
	adc_set_watchdog_high_threshold(ADC1, 4095);
	adc_set_watchdog_low_threshold(ADC1, 0);
	adc_enable_irq(ADC1, ADC_IRQ_EOC);
	adc_disable_irq(ADC1, ADC_IRQ_EOC);
	(void)adc_get_flag(ADC1, ADC_FLAG_EOC);
	adc_clear_flag(ADC1, ADC_FLAG_EOC);

	/* --- dma --- */
	{
		uint8_t ch;

		for (ch = DMA_CHANNEL1; ch <= DMA_CHANNEL7; ch++) {
			dma_channel_reset(DMA1, ch);
			dma_set_peripheral_address(DMA1, ch, USART1_BASE + 0x04);
			dma_set_memory_address(DMA1, ch, 0x20000000);
			dma_set_number_of_data(DMA1, ch, 16);
			(void)dma_get_number_of_data(DMA1, ch);
			dma_set_read_from_peripheral(DMA1, ch);
			dma_set_read_from_memory(DMA1, ch);
			dma_enable_memory_increment_mode(DMA1, ch);
			dma_disable_memory_increment_mode(DMA1, ch);
			dma_enable_peripheral_increment_mode(DMA1, ch);
			dma_disable_peripheral_increment_mode(DMA1, ch);
			dma_set_peripheral_size(DMA1, ch, DMA_SIZE_8BIT);
			dma_set_memory_size(DMA1, ch, DMA_SIZE_8BIT);
			dma_enable_circular_mode(DMA1, ch);
			dma_enable_transfer_complete_interrupt(DMA1, ch);
			dma_disable_transfer_complete_interrupt(DMA1, ch);
			dma_enable_half_transfer_interrupt(DMA1, ch);
			dma_disable_half_transfer_interrupt(DMA1, ch);
			dma_enable_transfer_error_interrupt(DMA1, ch);
			dma_disable_transfer_error_interrupt(DMA1, ch);
			dma_set_priority(DMA1, ch, DMA_PRIORITY_HIGH);
			dma_enable_mem2mem_mode(DMA1, ch);
			dma_disable_mem2mem_mode(DMA1, ch);
			dma_channel_enable(DMA1, ch);
			dma_channel_disable(DMA1, ch);
			(void)dma_get_flag(DMA1, ch, DMA_GIF);
			dma_clear_flag(DMA1, ch, DMA_GIF);
			(void)dma_get_interrupt_status(DMA1, ch);
			dma_clear_interrupt_pending_bit(DMA1, ch);
		}
	}

	/* --- exti --- */
	exti_set_trigger(EXTI_BASE, EXTI1, EXTI_TRIGGER_RISING);
	exti_enable_request(EXTI_BASE, EXTI1);
	exti_disable_request(EXTI_BASE, EXTI1);
	exti_reset_request(EXTI_BASE, EXTI1);
	(void)exti_get_flag_status(EXTI_BASE, EXTI1);
	exti_trigger_software(EXTI_BASE, EXTI1);
	exti_enable_event(EXTI_BASE, EXTI1);
	exti_disable_event(EXTI_BASE, EXTI1);

	/* --- flash --- */
	flash_unlock(FLASH_R_BASE);
	flash_lock(FLASH_R_BASE);
	(void)flash_wait_for_last_operation(FLASH_R_BASE);
	(void)flash_get_status_flags(FLASH_R_BASE);
	flash_clear_status_flags(FLASH_R_BASE);
	(void)flash_erase_page(FLASH_R_BASE, 0x08000000);
	(void)flash_erase_all_pages(FLASH_R_BASE);
	(void)flash_program_word(FLASH_R_BASE, 0x08000000, 0);
	(void)flash_program_halfword(FLASH_R_BASE, 0x08000000, 0);
	flash_set_latency(FLASH_R_BASE, FLASH_ACTLR_LATENCY_1);
	flash_unlock_option_bytes(FLASH_R_BASE);
	(void)flash_program_option_bytes(FLASH_R_BASE, 0x1ffff800, 0);
	(void)flash_get_option_bytes(FLASH_R_BASE);
	(void)flash_enable_write_protection(FLASH_R_BASE, 0);
	flash_unlock_fast(FLASH_R_BASE);
	flash_lock_fast(FLASH_R_BASE);
	flash_buf_reset(FLASH_R_BASE);
	flash_buf_load(FLASH_R_BASE, 0, 0);
	flash_erase_page_fast(FLASH_R_BASE, 0x08000000);
	flash_program_page_fast(FLASH_R_BASE, 0x08000000);

	/* --- iwdg / wwdg --- */
	iwdg_write_access_enable(IWDG_BASE);
	iwdg_write_access_disable(IWDG_BASE);
	iwdg_set_prescaler(IWDG_BASE, IWDG_PSCR_DIV64);
	iwdg_set_reload(IWDG_BASE, 1000);
	iwdg_reload_counter(IWDG_BASE);
	iwdg_enable(IWDG_BASE);
	(void)iwdg_get_flag(IWDG_BASE, IWDG_FLAG_PVU);

	wwdg_set_prescaler(WWDG_BASE, WWDG_PRESCALER_8);
	wwdg_set_window(WWDG_BASE, 0x50);
	wwdg_enable_interrupt(WWDG_BASE);
	wwdg_set_counter(WWDG_BASE, 0x7f);
	wwdg_enable(WWDG_BASE, 0x7f);
	(void)wwdg_get_flag(WWDG_BASE);
	wwdg_clear_flag(WWDG_BASE);

	/* --- pwr --- */
	pwr_enable_pvd(PWR_BASE);
	pwr_disable_pvd(PWR_BASE);
	pwr_set_pvd_level(PWR_BASE, PWR_PVD_LEVEL_2);
	pwr_enable_auto_wakeup(PWR_BASE);
	pwr_disable_auto_wakeup(PWR_BASE);
	pwr_set_awu_prescaler(PWR_BASE, PWR_AWU_DIV1024);
	pwr_set_awu_window(PWR_BASE, 0x20);
	(void)pwr_get_flag(PWR_BASE, PWR_FLAG_PVDO);

	/* --- opa --- */
	opa_reset(EXTEN_BASE);
	opa_set_inputs(EXTEN_BASE, OPA_PSEL_CHP1, OPA_NSEL_CHN1);
	opa_enable(EXTEN_BASE);
	opa_disable(EXTEN_BASE);

	/* --- dbgmcu --- */
	(void)dbgmcu_get_revision_id();
	(void)dbgmcu_get_device_id();
	(void)dbgmcu_get_control();
	dbgmcu_set_control(0);
	dbgmcu_stop_peripheral(DBGMCU_IWDG_STOP | DBGMCU_TIM1_STOP);
	dbgmcu_resume_peripheral(DBGMCU_ALL_STOP);
}
