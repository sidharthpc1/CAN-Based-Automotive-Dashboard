/*
 * File:   main.c
 * Author: HP
 *
 * Created on 27 September, 2026, 12:37 PM
 */

/* ECU1 */
#include <xc.h>

#include "clcd.h"
#include "adc.h"
#include "mkp.h"
#include "can.h"
#include "msg_id.h"


void init_config(void)
{
    init_clcd();
    init_matrix_keypad();
    init_adc();
    init_can();

    /* AN4 is used for the potentiometer */
    ADCON1 = 0x07;

    /* RA5/AN4 as input */
    TRISA5 = 1;
}


void main(void)
{
    unsigned char key;
    unsigned char count = 0;

    unsigned short adc_value;
    unsigned char speed;

    const unsigned char gear[8][3] =
    {
        "ON",
        "GN",
        "G1",
        "G2",
        "G3",
        "G4",
        "G5",
        "GR"
    };

    unsigned char speed_str[3];


    init_config();


    while (1)
    {
        /* Read keypad */
        key = read_switches(STATE_CHANGE);


        /* SW1 -> Increase gear */
        if (key == MK_SW1)
        {
            if (count < 7)
            {
                count++;
            }
        }


        /* SW2 -> Decrease gear */
        else if (key == MK_SW2)
        {
            if (count > 0)
            {
                count--;
            }
        }


        /* TRANSMIT GEAR */

        can_transmit(GEAR_MSG_ID, gear[count], 2);

        while (TXB0REQ);


        /*
         * =========================
         * READ SPEED
         * =========================
         */

        adc_value = read_adc(CHANNEL4);

        /* Mentor's calculation */
        speed = (unsigned char)(adc_value / 10.33);


        /* Convert speed into characters */
        speed_str[0] = (speed / 10) + '0';
        speed_str[1] = (speed % 10) + '0';
        speed_str[2] = '\0';


        /*
         * =========================
         * TRANSMIT SPEED
         * =========================
         */

        can_transmit(SPEED_MSG_ID, speed_str, 2);

        while (TXB0REQ);
    }
}