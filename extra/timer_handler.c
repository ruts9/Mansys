/**
 * @file timer_handler.c
 *
 * @brief Init TIMER0 to control motor with PWM signal.
 * 
 * @author Johannes Ehala, ProLab.
 * @license MIT
 *
 * Copyright ProLab, TTÜ. 15 January 2022
 */

#include "em_cmu.h"
#include "em_timer.h"

#include "timer_handler.h"

void LED_gpio_init(void)
{
    CMU_ClockEnable(cmuClock_GPIO, true);


    // Configure LED pins as push-pull output pins.
    GPIO_PinModeSet(ESWGPIO_LED0_PORT, ESWGPIO_LED0_PIN, gpioModePushPull, 0);
    GPIO_PinModeSet(ESWGPIO_LED1_PORT, ESWGPIO_LED1_PIN, gpioModePushPull, 0);
    GPIO_PinModeSet(ESWGPIO_LED2_PORT, ESWGPIO_LED2_PIN, gpioModePushPull, 0);
}

/**
 * @brief Init TIMER0 to regulate PWM dutycycle. Returns PWM frequency.
 */
uint32_t timer0_init(void)
{
	// Enable Timer and GPIO clocks.
    CMU_ClockEnable(cmuClock_TIMER0, true);

	// Init CompareCapture for PWM. 
	// https://docs.silabs.com/mcu/latest/efr32mg12/structTIMER-InitCC-TypeDef
	TIMER_InitCC_TypeDef ccInit = TIMER_INITCC_DEFAULT;
	ccInit.mode = timerCCModePWM;
	ccInit.cmoa = timerOutputActionToggle;
	
	TIMER_InitCC(TIMER0, LED2_CC_CHANNEL, &ccInit);

	// Enable GPIO toggling by TIMER and set location of pins to be toggled.
	TIMER0->ROUTEPEN = TIMER_ROUTEPEN_CC0PEN; //Definition at line 589 of file efr32mg12p_timer.h.
	TIMER0->ROUTELOC0 = (LED2_LOCATION);

	// Set TIMER0 top value and initial PWM duty cycle.
	TIMER_TopSet(TIMER0, LED2_TIMER0_TOP_VAL);
	TIMER_CompareBufSet(TIMER0, LED2_CC_CHANNEL, 0); // Initial duty cycle = 0

	// TIMER general init
	TIMER_Init_TypeDef timerInit = TIMER_INIT_DEFAULT;
	timerInit.prescale = LED2_TIMER0_CLOCK_DIV;
	timerInit.enable = true; // Start timer after init.

	TIMER_Init(TIMER0, &timerInit);

    return (uint32_t)(CMU_ClockFreqGet(cmuClock_TIMER0) / (uint32_t)(0x1 << LED2_TIMER0_CLOCK_DIV));
}

uint32_t timer1_init(void)
{
	// Enable Timer and GPIO clocks.
    CMU_ClockEnable(cmuClock_TIMER1, true);

	// Init CompareCapture for PWM. 
	// https://docs.silabs.com/mcu/latest/efr32mg12/structTIMER-InitCC-TypeDef
	TIMER_InitCC_TypeDef ccInit1 = TIMER_INITCC_DEFAULT;
	ccInit1.mode = timerCCModePWM;
	ccInit1.cmoa = timerOutputActionToggle;
	
	TIMER_InitCC(TIMER1, LED1_CC_CHANNEL, &ccInit1);
	
	//Initialise timer 1 channel 1
	TIMER_InitCC(TIMER1, LED0_CC_CHANNEL, &ccInit1);
	

	// Enable GPIO toggling by TIMER and set location of pins to be toggled.
	TIMER1->ROUTEPEN |= TIMER_ROUTEPEN_CC0PEN; //Definition at line 589 of file efr32mg12p_timer.h.
	TIMER1->ROUTELOC0 |= (LED1_LOCATION);
	
	//Enable GPIO toggling for channel 1
	TIMER1->ROUTEPEN |= TIMER_ROUTEPEN_CC1PEN; //Definition at line 589 of file efr32mg12p_timer.h.
	TIMER1->ROUTELOC0 |= (LED0_LOCATION);
	//2
	
	// Set TIMER1 top value and initial PWM duty cycle.
	TIMER_TopSet(TIMER1, LED1_TIMER1_TOP_VAL);
	TIMER_CompareBufSet(TIMER1, LED1_CC_CHANNEL, 0); // Initial duty cycle = 0
	
	//Set TIMER1 channel 1 top value and initial PWM duty cycle.
	TIMER_TopSet(TIMER1, LED0_TIMER1_TOP_VAL);
	TIMER_CompareBufSet(TIMER1, LED0_CC_CHANNEL, 0); // Initial duty cycle = 0
	
	

	// TIMER general init
	TIMER_Init_TypeDef timerInit1 = TIMER_INIT_DEFAULT;
	timerInit1.prescale = LED1_TIMER1_CLOCK_DIV;
	timerInit1.enable = true; // Start timer after init.

	TIMER_Init(TIMER1, &timerInit1);

    return (uint32_t)(CMU_ClockFreqGet(cmuClock_TIMER1) / (uint32_t)(0x1 << LED1_TIMER1_CLOCK_DIV));
}

//Sets timer PWM by its given inputs
void timer_set_pwm_dc(TIMER_TypeDef *timer, uint32_t channel, uint32_t dc)
{
    TIMER_CompareBufSet(timer, channel, dc);
}

