/**
 * @file main.c
 *
 * @brief Main source code for the PMOD_Color program.
 *
 * This file contains the main entry point for the PMOD_Color program,
 * which is used to demonstrate the PMOD_Color driver.
 *
 * It interfaces with the PMOD Color sensor module, which uses the I2C communication protocol.
 *  - Product Link: https://digilent.com/shop/pmod-color-color-sensor-module/
 *  - Reference Manual: https://digilent.com/reference/reference/pmod/pmodcolor/reference-manual
 *  - AMS TCS34725 Datasheet: https://ams.com/documents/20143/36005/TCS3472_DS000390_3-00.pdf
 *
 * The following connections must be made:
 *  - PMOD COLOR IO1 / ~INT     (Pin 1)     <-->  Not Connected
 *  - PMOD COLOR IO2 / LED_EN   (Pin 2)     <-->  MSP432 LaunchPad Pin P8.3
 *  - PMOD COLOR SCL            (Pin 3)     <-->  MSP432 LaunchPad Pin P6.5 (SCL)
 *  - PMOD COLOR SDA            (Pin 4)     <-->  MSP432 LaunchPad Pin P6.4 (SDA)
 *  - PMOD COLOR GND            (Pin 5)     <-->  MSP432 LaunchPad GND
 *  - PMOD COLOR VCC            (Pin 6)     <-->  MSP432 LaunchPad VCC (3.3V)
 *
 * @author Aaron Nanas
 *
 */

#include <stdint.h>
#include <stdlib.h>
#include <math.h>
#include "msp.h"
#include "inc/Clock.h"
#include "inc/CortexM.h"
#include "inc/EUSCI_A0_UART.h"
#include "inc/PMOD_Color.h"
#include "inc/GPIO.h"
#include "inc/Motor.h"
#include "inc/SysTick_Interrupt.h"

typedef enum {
    COLOR_GREEN = 0,
    COLOR_RED   = 1,
    COLOR_YELLOW = 2,
    COLOR_UNKNOWN = 3
} Color_t;

#define PATTERN_LENGTH 4
Color_t pattern[PATTERN_LENGTH];

void Generate_Random_Pattern(void);
void Show_Pattern(void);

int CheckPattern(Color_t detected);

Color_t Detect_Color(uint16_t R, uint16_t G, uint16_t B);


// Initialize a global variable for SysTick to keep track of elapsed time in milliseconds
uint32_t SysTick_ms_elapsed = 0;

// Global flag that gets set in Bumper_Switches_Handler.
// This is used to detect if any collisions occurred when any one of the bumper switches are pressed.
uint8_t collision_detected = 0;

/**
 * @brief Interrupt service routine for the SysTick timer.
 *
 * The interrupt service routine for the SysTick timer increments the SysTick_ms_elapsed
 * global variable to keep track of the elapsed milliseconds. If collision_detected is 0, then
 * it checks if 500 milliseconds passed. It toggles the front yellow LEDs and turns off the back red LEDs
 * on the chassis board. Otherwise, if collision_detected is set, it turns off the front yellow LEDs
 * and turns on the back red LEDs on the chassis board.
 *
 * @param None
 *
 * @return None
 */
void SysTick_Handler(void)
{
    SysTick_ms_elapsed++;

    if (collision_detected == 0)
    {
        if (SysTick_ms_elapsed >= 500)
        {
            P8->OUT &= ~0xC0;
            P8->OUT ^= 0x21;
            SysTick_ms_elapsed = 0;
        }
    }

    else
    {
        P8->OUT |= 0xC0;
        P8->OUT &= ~0x21;
    }
}



int main(void)
{
    // Ensure that interrupts are disabled during initialization
    DisableInterrupts();

    // Initialize the 48 MHz Clock
    Clock_Init48MHz();

    //Initialize GPIO
    LED2_Init();
    Buttons_Init();

    //Initialize Timer & Motor
    Timer_A0_PWM_Init(TIMER_A0_PERIOD_CONSTANT, 0, 0);
    Motor_Init();

    // Initialize the SysTick timer to generate periodic interrupts every 1 ms
    SysTick_Interrupt_Init(SYSTICK_INT_NUM_CLK_CYCLES, SYSTICK_INT_PRIORITY);

    // Initialize EUSCI_A0_UART
    EUSCI_A0_UART_Init_Printf();

    // Initialize the PMOD Color module
    PMOD_Color_Init();

    // Indicate that the PMDO Color module has been initialized and powered on
    printf("PMOD COLOR has been initialized and powered on.\n");

    // Enable the interrupts used by the modules
    EnableInterrupts();

    // Display the PMOD Color Device ID
    printf("PMOD Color Device ID: 0x%02X\n", PMOD_Color_Get_Device_ID());

    // Declare structs for both raw and normalized PMOD Color data
    PMOD_Color_Data pmod_color_data;
    PMOD_Calibration_Data calibration_data;

    pmod_color_data = PMOD_Color_Get_RGBC();
    calibration_data = PMOD_Color_Init_Calibration_Data(pmod_color_data);
    Clock_Delay1us(2400);

    srand(time(NULL)); // reset the rand()

    Generate_Random_Pattern();
    Show_Pattern();


    while(1)
    {
        // The on-board LED on the PMOD COLOR module can be controlled using the PMOD_Color_LED_Control function
        // Uncomment the line below if you'd like to see the on-board LED
        PMOD_Color_LED_Control(PMOD_COLOR_ENABLE_LED);

        // Sample the PMOD COLOR sensor every 50 ms
        pmod_color_data = PMOD_Color_Get_RGBC();
        PMOD_Color_Calibrate(pmod_color_data, &calibration_data);
        pmod_color_data = PMOD_Color_Normalize_Calibration(pmod_color_data, calibration_data);
        printf("r=%04x g=%04x b=%04x\r\n", pmod_color_data.red, pmod_color_data.green, pmod_color_data.blue);
        Clock_Delay1ms(50);


        uint16_t R = pmod_color_data.red;
        uint16_t G = pmod_color_data.green;
        uint16_t B = pmod_color_data.blue;

        Color_t detect = Hold_Color(R, G, B);

        int result = CheckPattern(detect);

        if (result == 1)
        {
            printf("Correct step!\n");
            LED2_Output(RGB_LED_WHITE);
            Clock_Delay1ms(500);
            LED2_Output(RGB_LED_OFF);

        }
        else if (result == 2)
        {
            printf("ACCESS GRANTED!\n");
            LED2_Output(RGB_LED_SKY_BLUE);
            Clock_Delay1ms(3000);
            LED2_Output(RGB_LED_OFF);

            Motor_Forward(3000, 3000);
            Clock_Delay1ms(2000);
            Motor_Backward(3000, 3000);
            Clock_Delay1ms(2000);
            Motor_Stop();

            Generate_Random_Pattern();
            Show_Pattern();
        }
        else if (result == 0)
        {
            printf("Wrong! Restarting...\n");
            LED2_Output(RGB_LED_PINK);
            Clock_Delay1ms(2500);
            LED2_Output(RGB_LED_OFF);

            Clock_Delay1ms(500);
            Motor_Left(4500, 4500);
            Clock_Delay1ms(2000);
            Motor_Right(4500, 4500);
            Clock_Delay1ms(2000);
            Motor_Stop();

            Show_Pattern();
        }


    }
}

Color_t Detect_Color(uint16_t R, uint16_t G, uint16_t B)
{
    // ---- GREEN ----
    if (G > R + 3000 && G > B + 3000)
    {
        printf("GREEN\n");
        LED2_Output(RGB_LED_GREEN);
        return COLOR_GREEN;
    }

    // ---- YELLOW ----
    else if (R > 0x2000 && G > 0x2000 && B < 0x3000)
    {
        printf("YELLOW\n");
        LED2_Output(RGB_LED_YELLOW);
        return COLOR_YELLOW;
    }

    // ---- RED  ----
    else if (R > G + 6000 && R > B + 6000)
    {
        printf("RED\n");
        LED2_Output(RGB_LED_RED);
        return COLOR_RED;
    }
    else
    {
        LED2_Output(RGB_LED_OFF);
        return COLOR_UNKNOWN;
    }
}

Color_t Hold_Color(uint16_t R, uint16_t G, uint16_t B)
{
    Color_t color = Detect_Color(R, G, B);   // your existing color logic

    if (color != COLOR_UNKNOWN)
    {
        // Hold the detected color for 1 second
        Clock_Delay1ms(1000);

        return color;  // return the locked-in color
    }

    return COLOR_UNKNOWN;
}



void Generate_Random_Pattern(void)
{
    for (int i = 0; i < PATTERN_LENGTH; i++)
    {
        pattern[i] = rand() % 3;   // 0 = green, 1 = red, 2 = yellow
    }
}


void Show_Pattern(void)
{
    for (int i = 0; i < PATTERN_LENGTH; i++)
    {
        switch(pattern[i])
        {
            case COLOR_GREEN:
                LED2_Output(RGB_LED_GREEN);
                break;

            case COLOR_RED:
                LED2_Output(RGB_LED_RED);
                break;

            case COLOR_YELLOW:
                LED2_Output(RGB_LED_YELLOW);
                break;
        }

        Clock_Delay1ms(700);  // hold the color
        LED2_Output(RGB_LED_OFF);
        Clock_Delay1ms(300);  // gap between colors
    }
}

int CheckPattern(Color_t detected)
{
    static int index = 0;
    static int failCount = 0; // counts consecutive failures

    if (detected == COLOR_UNKNOWN)
        return -1;  // ignore noise completely

    // ---------- CORRECT COLOR ----------
    if (detected == pattern[index])
    {
        failCount = 0;    // reset failure counter
        index++;

        if (index == PATTERN_LENGTH)
        {
            index = 0;
            return 2;     // full pattern matched
        }
        return 1; // correct so far
    }

    // ---------- WRONG COLOR ----------
    else
    {
        failCount++;

        if (failCount >= 2)   // only fail after 2 bad reads in a row
        {
            index = 0;
            failCount = 0;
            return 0;   // full failure: restart needed
        }

        return -1;   // mild failure → IGNORE (do not restart)
    }
}



