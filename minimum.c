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

static void io_setup(void)
{

  
	
	/* PB1 (LED) alt function push/pull - TIM3_CH4 */
	gpio_set_mode(GPIOB,
	  GPIO_MODE_OUTPUT_2_MHZ,
	  GPIO_CNF_OUTPUT_ALTFN_PUSHPULL,
	  GPIO_TIM3_CH4);
	
	/* PB8 (button) input */
	gpio_set_mode(GPIOB,
	  GPIO_MODE_INPUT,
	  GPIO_CNF_INPUT_FLOAT,
	  GPIO8);
}


static void tim_setup(void)
{
  uint16_t temp;
  
	/* Reset TIM3 peripheral to defaults. */
	rcc_periph_reset_pulse(RST_TIM3);

	/* Clock div and mode */
	TIM_CR1(TIM3) = TIM_CR1_CKD_CK_INT | TIM_CR1_CMS_EDGE;

  /* Set PWM period */
	TIM_ARR(TIM3) = 50000;
	
	/* Set PWM prescaler */
	TIM_PSC(TIM3) = 1600;
	
  /* Enable preload (ref sec 14.3.9 p355) */
	timer_enable_preload(TIM3);
	
	/* Set UG to load all regs (ref sec 14.3.9 p355, defs p384) */
  TIM_EGR(TIM3) = TIM_EGR_UG;
  
  /* Set OC4 mode, preload */
  TIM_CCMR2(TIM3) |= TIM_CCMR2_OC4M_PWM1 | TIM_CCMR2_OC4PE;
  
  /* Set OC4 polarity and state */
  TIM_CCER(TIM3) |= TIM_CCER_CC4E;

  /* Set Compare value */  
  TIM_CCR4(TIM3)=100;
  
  /* ARR reload enable */
  TIM_CR1(TIM3) |= TIM_CR1_ARPE;
  
	/* Counter enable. */
	timer_enable_counter(TIM3);

}

int main(void)
{  
  //rcc_clock_setup_in_hse_8mhz_out_72mhz();

  /* configure port B */
	rcc_periph_clock_enable(RCC_GPIOB);
	/* Enable TIM3 clock. */
	rcc_periph_clock_enable(RCC_TIM3);
	rcc_periph_clock_enable(RCC_AFIO);

  io_setup();
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
