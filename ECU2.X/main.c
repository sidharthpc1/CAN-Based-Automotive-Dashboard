/*
 * File:   main.c
 * Author: HP
 *
 * Created on 28 September, 2026, 2:17 PM
 */

/* ECU2 */

#include <xc.h>

#define _XTAL_FREQ 20000000

#include "adc.h"
#include "dkp.h"
#include "can.h"
#include "msg_id.h"


void init_config(void)
{
    init_digital_keypad();
    init_adc();
    init_can();

    /* AN4 is used for potentiometer */
    ADCON1 = 0x07;

    /* RA5/AN4 as input */
    TRISA5 = 1;

    /*
     * RB7, RB6 -> Left indicator
     * RB0, RB1 -> Right indicator
     * RB2 -> CAN TX
     * RB3 -> CAN RX
     */
    TRISB = 0x38;

    PORTB = 0;
}


void main(void)
{
    unsigned char key;

    unsigned short adc_value;
    unsigned int rpm;

    unsigned char flag = 0;

    unsigned char rpm_data[2];
    unsigned char indicator_data;


    init_config();


    while (1)
    {
        /*
         * =========================
         * READ DIGITAL KEYPAD
         * =========================
         */

        key = read_digital_keypad(STATE_CHANGE);


        /* SW1 -> LEFT */
        if (key == SWITCH1)
        {
            flag = 1;
        }


        /* SW2 -> RIGHT */
        else if (key == SWITCH2)
        {
            flag = 2;
        }


        /* SW3 -> STOP */
        else if (key == SWITCH3)
        {
            flag = 0;
        }


        /*
         * =========================
         * ECU2 LOCAL INDICATOR
         * =========================
         */

        if (flag == 1)
        {
            /* LEFT indicator */
            RB0 = !RB0;
            RB1 = !RB1;

            __delay_ms(500);
        }

        else if (flag == 2)
        {
            /* RIGHT indicator */
            RB7 = !RB7;
            RB6 = !RB6;

            __delay_ms(500);
        }

        else
        {
            /* STOP - all indicators OFF */
            RB0 = 0;
            RB1 = 0;
            RB6 = 0;
            RB7 = 0;
        }


        /*
         * =========================
         * TRANSMIT INDICATOR
         * =========================
         *
         * flag = 1 -> LEFT
         * flag = 2 -> RIGHT
         * flag = 0 -> STOP
         */

        indicator_data = flag;

        can_transmit(INDICATOR_MSG_ID,
                     &indicator_data,
                     1);

        while (TXB0REQ);


        /*
         * =========================
         * READ RPM
         * =========================
         */

        adc_value = read_adc(CHANNEL4);

        /* Maximum RPM = 6000 */
        rpm = (unsigned int)((adc_value / 10.23) * 60);


        /*
         * =========================
         * CONVERT RPM INTO 2 BYTES
         * =========================
         */

        rpm_data[0] = (unsigned char)(rpm >> 8);
        rpm_data[1] = (unsigned char)(rpm & 0xFF);


        /*
         * =========================
         * TRANSMIT RPM
         * =========================
         */

        can_transmit(RPM_MSG_ID,
                     rpm_data,
                     2);

        while (TXB0REQ);
    }
}