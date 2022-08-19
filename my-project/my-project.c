#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/timer.h>
#include <libopencm3/cm3/nvic.h>
#include <libopencm3/stm32/dma.h>
#include <libopencm3/stm32/usart.h>
#include "stdio.h"


static void clock_setup(void) {
	rcc_clock_setup_pll(&rcc_hse_8mhz_3v3[RCC_CLOCK_3V3_180MHZ]);
}

static void gpio_setup(void) {
        rcc_periph_clock_enable(RCC_GPIOA);
        rcc_periph_clock_enable(RCC_USART1);
        rcc_periph_clock_enable(RCC_USART2);
        gpio_mode_setup(GPIOA, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO2 | GPIO3 | GPIO9 | GPIO10);
        gpio_set_output_options(GPIOA, GPIO_OTYPE_OD, GPIO_OSPEED_25MHZ, GPIO9 | GPIO2);
        gpio_set_af(GPIOA, GPIO_AF7, GPIO2 | GPIO3 | GPIO9 | GPIO10);
		gpio_mode_setup(GPIOA, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, GPIO5);
        usart_set_baudrate(USART2, 115200);
        usart_set_databits(USART2, 8);
        usart_set_stopbits(USART2, USART_STOPBITS_1);
        usart_set_mode(USART2, USART_MODE_TX_RX);
        usart_set_parity(USART2, USART_PARITY_NONE);
        usart_set_flow_control(USART2, USART_FLOWCONTROL_NONE);
        usart_enable(USART2);		
}

static void dma_setup(void) {
	rcc_periph_clock_enable(RCC_DMA1);
	nvic_enable_irq(NVIC_DMA1_STREAM5_IRQ);
	dma_stream_reset(DMA1, DMA_STREAM5);
}

static void dma_read(char *data) {
	dma_set_peripheral_address(DMA1, DMA_STREAM5, (uint32_t)&USART2_DR);
	dma_set_memory_address(DMA1, DMA_STREAM5, (uint32_t)data);
	dma_set_number_of_data(DMA1, DMA_STREAM5, 15);
	dma_enable_memory_increment_mode(DMA1, DMA_STREAM5);
	dma_set_peripheral_size(DMA1, DMA_STREAM5, DMA_SxCR_PSIZE_8BIT);
	dma_set_memory_size(DMA1, DMA_STREAM5, DMA_SxCR_MSIZE_8BIT);
	dma_set_priority(DMA1, DMA_STREAM5, DMA_SxCR_PL_HIGH);

	dma_enable_transfer_complete_interrupt(DMA1, DMA_STREAM5);
	dma_channel_select(DMA1, DMA_STREAM5, DMA_SxCR_CHSEL_4);
	dma_enable_stream(DMA1, DMA_STREAM5);

    usart_enable_rx_dma(USART2);
}

char rx[16];

void dma1_stream5_isr(void) {
	gpio_toggle(GPIOA, GPIO5);
	if(dma_get_interrupt_flag(DMA1, DMA_STREAM5, DMA_TCIF)) {
		dma_clear_interrupt_flags(DMA1, DMA_STREAM5, DMA_TCIF);
	}
	dma_disable_transfer_complete_interrupt(DMA1, DMA_STREAM5);
	usart_disable_rx_dma(USART2);
	dma_disable_stream(DMA1, DMA_STREAM5);	

	for(int i=0; i< 15 ;i++) {
		usart_send_blocking(USART2, rx[i]);
		gpio_toggle(GPIOA, GPIO5);
	}

}

int main(void)
{
	clock_setup();
	gpio_setup();
	dma_setup();
	// dma_read(rx);
	while (1) { 
		dma_read(rx);
	}
	return 0;
}
