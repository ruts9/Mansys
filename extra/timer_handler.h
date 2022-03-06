/**
 * @file timer_handler.h
 *
 * @author Johannes Ehala, ProLab.
 * @license MIT
 *
 * Copyright ProLab, TTÜ. 15 January 2022
 */

#ifndef TIMER_HANDLER_H_
#define TIMER_HANDLER_H_

#include "em_gpio.h"

//
// K6ik PWM v22rtused sain ma silma j2rgi. 
//

#define LED2_CC_CHANNEL        0
#define LED2_LOCATION          TIMER_ROUTELOC0_CC0LOC_LOC5 // PA5 route location for CC channel 0
#define LED2_TIMER0_CLOCK_DIV    timerPrescale256 // Timer0 clock divider
#define LED2_TIMER0_TOP_VAL      100
#define LED2_PWM_DUTY_CYCLE      5

#define LED1_CC_CHANNEL        0
#define LED1_LOCATION          TIMER_ROUTELOC0_CC0LOC_LOC6 // PB11 route location for CC channel 0
#define LED1_TIMER1_CLOCK_DIV    timerPrescale256 // Timer1 clock divider
#define LED1_TIMER1_TOP_VAL      100
#define LED1_PWM_DUTY_CYCLE      100

#define LED0_CC_CHANNEL        1
#define LED0_LOCATION          TIMER_ROUTELOC0_CC1LOC_LOC6 // PB12 route location for CC channel 1
#define LED0_TIMER1_TOP_VAL      50
#define LED0_PWM_DUTY_CYCLE      20

#define ESWGPIO_LED0_PORT       gpioPortB
#define ESWGPIO_LED1_PORT       gpioPortB
#define ESWGPIO_LED2_PORT       gpioPortA


#define ESWGPIO_LED0_PIN        12     // Red
#define ESWGPIO_LED1_PIN        11     // Green
#define ESWGPIO_LED2_PIN        5      // Blue

// Public functions
void LED_gpio_init();
uint32_t timer0_init();
uint32_t timer1_init();
void timer_set_pwm_dc(TIMER_TypeDef *timer, uint32_t channel, uint32_t dc);

#endif // TIMER_HANDLER_H_ */
