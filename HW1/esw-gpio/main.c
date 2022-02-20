/**
 
 * Copyright Thinnect Inc. 2019
 * Copyright ProLab TTÜ 2022
 * @license MIT
 * @author Johannes Ehala
 
 
 * This code works as following:
  It starts with LED0 (RED) in ON state, the buzzer does not work
  When you press the button on the board, the buzzer starts to play 2 tones and the LED1 (GREEN) turns ON and the LED0 (RED) turns OFF.
  If the buzzer is ON and you press the button, the device goes back to the initial state (Buzzer and Green led OFF, RED led ON).
  
  The buzzer plays 2 tones, (500 Hz and 166 Hz).
  It plays 1 tone 1.2 seconds and the the other tone 1.2 seconds aswell and plays it back to back. 
  
 
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

#include "em_cmu.h"
#include "em_gpio.h"

#include "loglevels.h"
#define __MODUUL__ "main"
#define __LOG_LEVEL__ (LOG_LEVEL_main & BASE_LOG_LEVEL)
#include "log.h"

// Include the information header binary
#include "incbin.h"
INCBIN(Header, "header.bin");

#define ESWGPIO_LED0_PORT       gpioPortB
#define ESWGPIO_LED1_PORT       gpioPortB
#define ESWGPIO_BUZZER_PORT     gpioPortA
#define ESWGPIO_BUTTON_PORT     gpioPortF

#define ESWGPIO_LED0_PIN        12     // Red
#define ESWGPIO_LED1_PIN        11     // Green
#define ESWGPIO_BUTTON_PIN      4
#define ESWGPIO_BUZZER_PIN	 0 


#define ESWGPIO_BUZZER_DELAY    2  // 2 ms ON and 2 ms OFF, roughly equals 500 Hz
#define ESWGPIO_BUZZER_DELAY2    6  // 6 ms ON and 6 ms OFF, roughly equals 166 Hz


#define ESWGPIO_EXTI_INDEX      4 // External interrupt number 4.
#define ESWGPIO_EXTI_IF         0x00000010UL // Interrupt flag for external interrupt number 4.


static void buzzer_loop (void *args);
static void button_loop (void *args);
static osThreadId_t buttonThreadId;
static const uint32_t buttonExtIntThreadFlag = 0x00000001;

static void gpio_external_interrupt_init (GPIO_Port_TypeDef port, uint32_t pin, uint32_t exti_if, uint16_t exti_num);
static void gpio_external_interrupt_enable (uint32_t if_exti);
static uint16_t buttonPressed = 0;

// Heartbeat thread, initialize GPIO and print heartbeat messages.
void hp_loop ()
{
    #define ESWGPIO_HB_DELAY 10 // Heartbeat message delay, seconds
    
    // Initialize GPIO peripheral.
    CMU_ClockEnable(cmuClock_GPIO, true);
    
    // Configure LED pins as push-pull output pins.
    GPIO_PinModeSet(ESWGPIO_LED0_PORT, ESWGPIO_LED0_PIN, gpioModePushPull, 0);
    GPIO_PinModeSet(ESWGPIO_LED1_PORT, ESWGPIO_LED1_PIN, gpioModePushPull, 0);
    GPIO_PinModeSet(ESWGPIO_BUZZER_PORT, ESWGPIO_BUZZER_PIN, gpioModePushPull, 0);
    
    
    const osThreadAttr_t buzzer_thread_attr = { .name = "buzz" };
    osThreadNew(buzzer_loop, NULL, &buzzer_thread_attr);

    
    // Configure button pin for external interrupts. 
    gpio_external_interrupt_init(ESWGPIO_BUTTON_PORT, ESWGPIO_BUTTON_PIN, ESWGPIO_EXTI_IF, ESWGPIO_EXTI_INDEX);
    
    // Create thread for handling external interrupts.
    const osThreadAttr_t button_thread_attr = { .name = "button" };
    buttonThreadId = osThreadNew(button_loop, NULL, &button_thread_attr);
    
    // Enanble external interrupts from button.
    gpio_external_interrupt_enable(ESWGPIO_EXTI_IF);
    
    for (;;)
    {
        osDelay(ESWGPIO_HB_DELAY*osKernelGetTickFreq());
        info1("Heartbeat");
    }
}



// BUZZER toggle threads.
static void buzzer_loop (void *args)
{
    // Variable to use in loop, and keep track on how many iterations done.
    uint32_t length = 0;
    //  Becouse the first tone is higher frequency, the delays are smaller than the second tone,
   // becouse of that we need to use the difference of the 2 tones to make them the same length
    uint32_t tone_time_diff = ESWGPIO_BUZZER_DELAY2 / ESWGPIO_BUZZER_DELAY;
    
    for (;;)
    {
    	// If button is NOT pressed RED LED is ON, Green LED is OFF
    	if(buttonPressed == 0){
    	  GPIO_PinOutSet(ESWGPIO_LED0_PORT, ESWGPIO_LED0_PIN);
    	  GPIO_PinOutClear(ESWGPIO_LED1_PORT, ESWGPIO_LED1_PIN);
    	}
    	else{
    	// Button IS pressed, so turn ON Green LED and turn OFF RED LED
    	  GPIO_PinOutSet(ESWGPIO_LED1_PORT, ESWGPIO_LED1_PIN);
    	  GPIO_PinOutClear(ESWGPIO_LED0_PORT, ESWGPIO_LED0_PIN);
    	  
    	  /* 
    	    The logic behind this if block is this:
    	    the 2 osDelays used in this block takes a combined time of 4 ms
    	    so taking ONLY these delays into account, the block is completed once in
    	    every 4 ms. But the else statement block takes a combined time of 12 ms, 
    	    so without multiplying it with tone_time_diff, the first tone would play for
    	    ~ 4 * 100 = 0.4 seconds and the second tone would play ~ 12 * 100 = 1.2 seconds
    	  */ 
    	  
    	  if (length <= 100 * tone_time_diff){
    	    GPIO_PinOutSet(ESWGPIO_BUZZER_PORT, ESWGPIO_BUZZER_PIN);
	    osDelay(ESWGPIO_BUZZER_DELAY);
	    GPIO_PinOutClear(ESWGPIO_BUZZER_PORT, ESWGPIO_BUZZER_PIN);
	    osDelay(ESWGPIO_BUZZER_DELAY);
	    length++;
	    }
	    
	    // When the first tone has played for 1.2 seconds, start to play the second tone    
	  else{
	    GPIO_PinOutSet(ESWGPIO_BUZZER_PORT, ESWGPIO_BUZZER_PIN);
	    osDelay(ESWGPIO_BUZZER_DELAY2);
	    GPIO_PinOutClear(ESWGPIO_BUZZER_PORT, ESWGPIO_BUZZER_PIN);
	    osDelay(ESWGPIO_BUZZER_DELAY2);
	    length++;
	    
	    // When the second tone has played for 1.2 seconds, start to play the first tone again.
	    if(length >= 100 * tone_time_diff + 100){length = 0;}
	    }
    }
}
}




static void button_loop (void *args)
{
    for (;;)
    {
        // Wait for external interrupt signal from button.
        osThreadFlagsClear(buttonExtIntThreadFlag);
        osThreadFlagsWait(buttonExtIntThreadFlag, osFlagsWaitAny, osWaitForever); // Flags are automatically cleared
        
        // If buttonPressed is 0, that means we want to turn it ON
        if(buttonPressed == 0){
          buttonPressed = 1;
        }
	else{
	// If the button was pressed, then we want to turn it OFF, so we equal the variable with 0.
	  buttonPressed = 0;
	}
        info1("Button");
        
        
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

    info1("ESW-GPIO "VERSION_STR" (%d.%d.%d)", VERSION_MAJOR, VERSION_MINOR, VERSION_PATCH);

    // Initialize OS kernel.
    osKernelInitialize();

    // Create a thread.
    const osThreadAttr_t hp_thread_attr = { .name = "hb" };
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

/**
 * @brief Initialize GPIO port and pin for external interrupts and enable 
 * GPIO hardware interrupts (NVIC). 
 */
static void gpio_external_interrupt_init (GPIO_Port_TypeDef port, uint32_t pin, uint32_t exti_if, uint16_t exti_num)
{
    // Configure pin. Input with glitch filtering and pull-up.
    GPIO_PinModeSet(port, pin, gpioModeInputPullFilter, 1);
    
    // Configure external interrupts.
    GPIO_IntDisable(exti_if); // Disable before config to avoid unwanted interrupt triggering.
    GPIO_ExtIntConfig(port, pin, exti_num, false, true, false); // Port, pin, EXTI number, rising edge, falling edge, enabled.
    GPIO_InputSenseSet(GPIO_INSENSE_INT, GPIO_INSENSE_INT);
}

static void gpio_external_interrupt_enable (uint32_t exti_if)
{
    GPIO_IntClear(exti_if);
    
    NVIC_EnableIRQ(GPIO_EVEN_IRQn);
    NVIC_SetPriority(GPIO_EVEN_IRQn, 3);

    GPIO_IntEnable(exti_if);
}

void GPIO_EVEN_IRQHandler (void)
{
    // Get all pending and enabled interrupts.
    uint32_t pending = GPIO_IntGetEnabled();
    
    // Check if button interrupt is enabled
    if (pending & ESWGPIO_EXTI_IF)
    {
        // Clear interrupt flag.
        GPIO_IntClear(ESWGPIO_EXTI_IF);

        // Trigger button thread to resume.
        osThreadFlagsSet(buttonThreadId, buttonExtIntThreadFlag);
    }
    else ; // This was not a button interrupt.
}
