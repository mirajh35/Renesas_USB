/***********************************************************************************************************************
* DISCLAIMER
* This software is supplied by Renesas Electronics Corporation and is only intended for use with Renesas products. No
* other uses are authorized. This software is owned by Renesas Electronics Corporation and is protected under all
* applicable laws, including copyright laws.
* THIS SOFTWARE IS PROVIDED "AS IS" AND RENESAS MAKES NO WARRANTIES REGARDING
* THIS SOFTWARE, WHETHER EXPRESS, IMPLIED OR STATUTORY, INCLUDING BUT NOT LIMITED TO WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT. ALL SUCH WARRANTIES ARE EXPRESSLY DISCLAIMED. TO THE MAXIMUM
* EXTENT PERMITTED NOT PROHIBITED BY LAW, NEITHER RENESAS ELECTRONICS CORPORATION NOR ANY OF ITS AFFILIATED COMPANIES
* SHALL BE LIABLE FOR ANY DIRECT, INDIRECT, SPECIAL, INCIDENTAL OR CONSEQUENTIAL DAMAGES FOR ANY REASON RELATED TO THIS
* SOFTWARE, EVEN IF RENESAS OR ITS AFFILIATES HAVE BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES.
* Renesas reserves the right, without notice, to make changes to this software and to discontinue the availability of
* this software. By using this software, you agree to the additional terms and conditions found by accessing the
* following link:
* http://www.renesas.com/disclaimer
*
* Copyright (C) 2017 Renesas Electronics Corporation. All rights reserved.
***********************************************************************************************************************/
#include <string.h>
#include "usb_thread.h"
#define SEMI_HOSTING

#ifdef SEMI_HOSTING
#ifdef __GNUC__                 /* GCC Compiler */
extern void initialise_monitor_handles(void);
#endif
#endif


/* Variables to store and toggle LED levels */
static ioport_level_t g_led1 = IOPORT_LEVEL_HIGH;
static ioport_level_t g_led2 = IOPORT_LEVEL_HIGH;
static ioport_level_t g_led3 = IOPORT_LEVEL_HIGH;

/* Toggles given LED level. For IOPORT_LEVEL_LOW returns IOPORT_LEVEL_HIGH and vice versa.*/
ioport_level_t toggle_led_level (ioport_level_t level);

/* Updates a LED - sets the given level for LED pin.*/
void update_led (ioport_port_pin_t led_pin, ioport_level_t level);

/* Prints out LED statuses on the terminal via the Communications Framework.*/
void print_led_statuses (void);

/* Generates a status string for a given pin level indicating the corresponding LED is ON or OFF.*/
void get_led_status_string (ioport_level_t level, CHAR * status_str, uint32_t max_len);

/* Communications on USBX Thread entry function */

/*The application project demonstrates the typical use of the Communications Framework APIs.
  The application project USBX Thread entry initializes the Communications Framework on USBX,
  sends a welcome message via the framework, and listens for user input. Each time the user presses an
  adequate key (1, 2, or 3), a corresponding LED toggles, and all LED statuses are sent in a message over
  the framework */

void usb_thread_entry (void)
{
    /* LED information structure */
    bsp_leds_t leds;
    /* Welcome message to be sent out via Communications Framework */
    uint8_t    welcome_msg[] =
    "*** Communications Module Guide Application Project ***\r\n\nTo toggle LEDs press 1, 2 or 3.\r\n\n";
    /* Parsed command received via Communications framework */
    uint8_t    cmd;
    /* Variable to handle SSP function errors */
    ssp_err_t  err;

    /* Initialize SEMI-HOSTING handlers to output the errors */
    #ifdef SEMI_HOSTING
         #ifdef __GNUC__
             if (CoreDebug->DHCSR & CoreDebug_DHCSR_C_DEBUGEN_Msk)
             {
                 initialise_monitor_handles();
             }
         #endif
     #endif

    /* Acquire LED information */
    err = R_BSP_LedsGet(&leds);

    if(err)
    {
        #ifdef SEMI_HOSTING
            if (CoreDebug->DHCSR & CoreDebug_DHCSR_C_DEBUGEN_Msk)
            {
                printf("\n board led error: %d",err);
            }
        #endif
        tx_thread_sleep(TX_WAIT_FOREVER);
    }

    /* Turn off all LEDs */
    update_led(leds.p_leds[0], g_led1);
    update_led(leds.p_leds[1], g_led2);
    update_led(leds.p_leds[2], g_led3);

    /* Make sure the Communications Framework and all underlying drivers are ready */
    tx_thread_sleep(1000);

    /* Print out the welcome message */
    err = g_sf_comms.p_api->write(g_sf_comms.p_ctrl, welcome_msg, (uint32_t) strlen((const CHAR*)welcome_msg), TX_WAIT_FOREVER);
    if (SSP_SUCCESS != err)
    {
        /* This is a fairly simple error handling - it holds the
         * application execution. In a more realistic scenarios
         * a more robust and complex error handling solution should
         * be provided. */
            #ifdef SEMI_HOSTING
                 if (CoreDebug->DHCSR & CoreDebug_DHCSR_C_DEBUGEN_Msk)
                 {
                     printf("\n USB Write error: %d",err);
                 }
             #endif
        tx_thread_sleep(TX_WAIT_FOREVER);
    }

    /* Print out the initial LED statuses */
    print_led_statuses();

    while (1)
    {
        /* Read user input/command (1, 2 or 3) */
        err = g_sf_comms.p_api->read(g_sf_comms.p_ctrl, &cmd, 1u, TX_WAIT_FOREVER);
        if (SSP_SUCCESS != err)
        {
            #ifdef SEMI_HOSTING
                 if (CoreDebug->DHCSR & CoreDebug_DHCSR_C_DEBUGEN_Msk)
                 {
                     printf("\n USB Read error: %d",err);
                 }
             #endif
            tx_thread_sleep(TX_WAIT_FOREVER);
        }

        /* Parse user input and dispatch read command */
        switch (cmd)
        {
            /* Toggle corresponding LED and Print out the updated LED statuses */

            case '1':
                /* User pressed 1 - toggle LED 1 */
                g_led1 = toggle_led_level(g_led1);
                update_led(leds.p_leds[0], g_led1);
                print_led_statuses();
                break;
            case '2':
                /* User pressed 2 - toggle LED 2 */
                g_led2 = toggle_led_level(g_led2);
                update_led(leds.p_leds[1], g_led2);
                print_led_statuses();
                break;
            case '3':
                /* User pressed 3 - toggle LED 3 */
                g_led3 = toggle_led_level(g_led3);
                update_led(leds.p_leds[2], g_led3);
                print_led_statuses();
                break;
            default:
                /* User pressed unsupported key - ignore, do nothing */
                break;
        }
    }
}

ioport_level_t toggle_led_level (ioport_level_t level)
{
    /*
    ioport_level_t result;

    if (level == IOPORT_LEVEL_HIGH)
    {
        result = IOPORT_LEVEL_LOW;
    }
    else
    {
        result = IOPORT_LEVEL_HIGH;
    }
    return result;
    */
    return level == IOPORT_LEVEL_HIGH ? IOPORT_LEVEL_LOW : IOPORT_LEVEL_HIGH;
}

void update_led (ioport_port_pin_t led_pin, ioport_level_t level)
{
    ssp_err_t led_err;

    led_err = g_ioport.p_api->pinWrite(led_pin, level);
    if (SSP_SUCCESS != led_err)
    {
        #ifdef SEMI_HOSTING
             if (CoreDebug->DHCSR & CoreDebug_DHCSR_C_DEBUGEN_Msk)
             {
                 printf("\n Led Status read failed: %d",led_err);
             }
         #endif
        tx_thread_sleep(TX_WAIT_FOREVER);
    }
}

void print_led_statuses (void)
{
    CHAR led1_str[4];
    CHAR led2_str[4];
    CHAR led3_str[4];
    CHAR msg[40];

    get_led_status_string(g_led1, led1_str, sizeof(led1_str));
    get_led_status_string(g_led2, led2_str, sizeof(led2_str));
    get_led_status_string(g_led3, led3_str, sizeof(led3_str));
    snprintf(msg, sizeof(msg), "LED1: %s\tLED2: %s\tLED3: %s\r\n", led1_str, led2_str, led3_str);

    ssp_err_t err = g_sf_comms.p_api->write(g_sf_comms.p_ctrl, (uint8_t *) msg, (uint32_t) strlen(msg), 1000);
    if (SSP_SUCCESS != err)
    {
        #ifdef SEMI_HOSTING
            if (CoreDebug->DHCSR & CoreDebug_DHCSR_C_DEBUGEN_Msk)
            {
                printf("\n USB Write error: %d",err);
            }
        #endif
        tx_thread_sleep(TX_WAIT_FOREVER);
    }
}

void get_led_status_string (ioport_level_t level, CHAR * status_str, uint32_t max_len)
{
    static CHAR on[]  = "ON";
    static CHAR off[] = "OFF";

    if (level == IOPORT_LEVEL_HIGH)
    {
        snprintf(status_str, max_len, "%s", off);
    }
    else
    {
        snprintf(status_str, max_len, "%s", on);
    }
}
