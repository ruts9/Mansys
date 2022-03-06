/**
 * @brief Example usage of Timer peripheral. Use Timer PWM functionality
 *        to generate signals for vibration motor. 
 *
 * EFR32 Application Note on Timers
 * https://www.silabs.com/documents/public/application-notes/AN0014.pdf
 *
 * EFR32MG12 Wireless Gecko Reference Manual (Timer p672)
 * https://www.silabs.com/documents/public/reference-manuals/efr32xg12-rm.pdf
 *
 * Timer API documentation 
 * https://docs.silabs.com/mcu/latest/efr32mg12/group-TIMER
 * 
 * ARM RTOS API
 * https://arm-software.github.io/CMSIS_5/RTOS2/html/group__CMSIS__RTOS.html
 * 
 * Copyright Thinnect Inc. 2019
 * Copyright ProLab TTÜ 2022
 * @license MIT
 * @author Johannes Ehala
 */
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <inttypes.h>

#include "retargetserial.h"
#include "cmsis_os2.h"
#include "platform.h"

#include "SignatureArea.h"
#include "DeviceSignature.h"

#include "loggers_ext.h"
#include "logger_fwrite.h"

#include "timer_handler.h"
#include "em_cmu.h"
#include "em_timer.h"

#include "loglevels.h"
#define __MODUUL__ "main"
#define __LOG_LEVEL__ (LOG_LEVEL_main & BASE_LOG_LEVEL)
#include "log.h"

// Include the information header binary
#include "incbin.h"
INCBIN(Header, "header.bin");

#define LED2_ON_OFF_TIME      1 // seconds
static void LED_control_loop (void *args);

// Heartbeat thread, initialize Timer and print heartbeat messages.
void hp_loop ()
{
    #define ESWGPIO_HB_DELAY 10 // Heartbeat message delay, seconds
    
    // Initialize GPIO and Timer
    LED_gpio_init();
    info1("PWM frequency %lu Hz", timer0_init());
    info1("PWM frequency %lu Hz", timer1_init());
    
    // Create a thread for motor control.
    const osThreadAttr_t LED_thread_attr = { .name = "LED" };
    osThreadNew(LED_control_loop, NULL, &LED_thread_attr);
    
    for (;;)
    {
        osDelay(ESWGPIO_HB_DELAY*osKernelGetTickFreq());
        info1("Heartbeat");
    }
}

static void LED_control_loop (void *args)
{
	// Kasutan 2 muutujat, et muuta duty_cyclet ja j2lgida kas LED peab minema eredamaks v tumedamaks.
	uint8_t counter = 1;
	uint8_t up_or_down = 0;
    
    for (;;)
    {		
    	// Kui duty_cycle on 100, siis peame hakkame ledi tumendama.
    	if (counter == 100){
    		up_or_down = 1;
	}
	// Kui duty_cycle on 0, siis peame hakkame ledi eredamaks.
	if (counter == 1){
    		up_or_down = 0;
	}
	
	// Kui muutuja on 1, siis muudame LEDi tumedamaks, ehk v2hendame duty_cyclet
    	if(up_or_down == 1){
    		counter -= 1;
    	}
    	// Kui muutuja on 0, siis muudame LEDi eredamaks, ehk suurendame duty_cyclet
    	if(up_or_down == 0){
    		counter += 1;	
	}
	// Iga loopi l6pp anname muutuja funktsiooni. Punane (LED0) on p66rdv6rdeline sinise ja rohelise LEDI v22rtusest.
	timer_set_pwm_dc(TIMER0, LED2_CC_CHANNEL, counter);
	timer_set_pwm_dc(TIMER1, LED1_CC_CHANNEL, counter);
	timer_set_pwm_dc(TIMER1, LED0_CC_CHANNEL, (counter - 100) * (-1));
	//Kasutame delayd et oleks silmaga n2ha LEDide muutumist.
	osDelay(10);
    }
}

int logger_fwrite_boot (const char *ptr, int len)
{
    fwrite(ptr, len, 1, stdout);
    fflush(stdout);
    return len;
}

int main ()
{
    PLATFORM_Init();

    // Configure log message output
    RETARGET_SerialInit();
    log_init(BASE_LOG_LEVEL, &logger_fwrite_boot, NULL);

    info1("ESW-Timer "VERSION_STR" (%d.%d.%d)", VERSION_MAJOR, VERSION_MINOR, VERSION_PATCH);

    // Initialize OS kernel.
    osKernelInitialize();

    // Create a thread.
    const osThreadAttr_t hp_thread_attr = { .name = "hp" };
    osThreadNew(hp_loop, NULL, &hp_thread_attr);

    if (osKernelReady == osKernelGetState())
    {
        // Switch to a thread-safe logger
        logger_fwrite_init();
        log_init(BASE_LOG_LEVEL, &logger_fwrite, NULL);

        // Start the kernel
        osKernelStart();
    }
    else
    {
        err1("!osKernelReady");
    }

    for(;;);
}
