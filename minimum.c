#include <libopencm3/cm3/nvic.h>
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/timer.h>


/*
#ifndef __NO_SYSTEM_INIT
void SystemInit()
{}
#endif
*/

static void tim_setup(void)
{
	/* Enable TIM2 clock. */
	rcc_periph_clock_enable(RCC_TIM2);

	/* Enable TIM2 interrupt. */
	nvic_enable_irq(NVIC_TIM2_IRQ);

	/* Reset TIM2 peripheral to defaults. */
	rcc_periph_reset_pulse(RST_TIM2);

	/* Timer global mode:
	 * - No divider
	 * - Alignment edge
	 * - Direction up
	 * (These are actually default values after reset above, so this call
	 * is strictly unnecessary, but demos the api for alternative settings)
	 */
	timer_set_mode(TIM2, TIM_CR1_CKD_CK_INT,
		TIM_CR1_CMS_EDGE, TIM_CR1_DIR_UP);

	/*
	 * Please take note that the clock source for STM32 timers
	 * might not be the raw APB1/APB2 clocks.  In various conditions they
	 * are doubled.  See the Reference Manual for full details!
	 * In our case, TIM2 on APB1 is running at double frequency, so this
	 * sets the prescaler to have the timer run at 5kHz
	 */
	//timer_set_prescaler(TIM2, (rcc_apb1_frequency / 5000));
	timer_set_prescaler(TIM2,1600);

	/* Disable preload. */
	timer_disable_preload(TIM2);
	timer_continuous_mode(TIM2);

	/* count full range, as we'll update compare value continuously */
	timer_set_period(TIM2, 65535);

	/* Set the initual output compare value for OC1. */
	timer_set_oc_value(TIM2, TIM_OC1, 1500);

	/* Counter enable. */
	timer_enable_counter(TIM2);

	/* Enable Channel 1 compare interrupt to recalculate compare values */
	timer_enable_irq(TIM2, TIM_DIER_CC1IE);
}

void tim2_isr(void)
{
	if (timer_get_flag(TIM2, TIM_SR_CC1IF)) {

		/* Clear compare interrupt flag. */
		timer_clear_flag(TIM2, TIM_SR_CC1IF);

    /* Set next interrupt time */
		timer_set_oc_value(TIM2, TIM_OC1, timer_get_counter(TIM2) + 2500 );
		
		/* Toggle LED to indicate compare event. */
    gpio_toggle(GPIOB,GPIO1);
	}
}

int main(void)
{  
  //rcc_clock_setup_in_hse_8mhz_out_72mhz();
  
	rcc_periph_clock_enable(RCC_GPIOB);
	
	/* PB1 (LED) output */
	gpio_set_mode(GPIOB,GPIO_MODE_OUTPUT_2_MHZ,GPIO_CNF_OUTPUT_PUSHPULL,GPIO1);
	
	/* PB8 (button) input */
	gpio_set_mode(GPIOB,GPIO_MODE_INPUT,GPIO_CNF_INPUT_FLOAT,GPIO8);
	
	/* start with LED off */
	gpio_clear(GPIOB,GPIO1);

  tim_setup();

  while(1);
}


/*

page 336

TIMx_CR1  control register 1
  DIR  up/down
  CEN counter enable
  ARPE - write ARR to shadow immediately or on update
  UDIS - update disable (0 by default)

TIMx_CR2  control register 2

TIMx_SMCR  slave mode control register

TIMx_DIER  DMA/Int enable register
  TIE trigger interrupt enable
  UIE update interrupt

TIMx_SR
  TIF  trigger interrupt flag
  UIF update interrupt flag

TIMx_EGR  event generation register

TIMx_CNT  counter

TIMx_PSC prescaler
  CK_CNT = fCK_PSC/PSC+1

TIMx_ARR  auto reload register

====
FOR upcounting reload timer
  - TIMx_SMCR=0 - CK_INT through to prescaler
  - TIMx_PSC  - set prescaler as needed
  - TIMx_ARR - set to reload value
  - TIMx_CNT - zero it just to make sure
  - TIMx_CR1 - set DIR=0 for up
  - TIMx_CR1 - set CEN to start counting


set interrupts:
TIM2 interrupt  */
