/*
 * File:   main.c
 * Author: HP
 *
 * Created on 1 October, 2026, 12:09 AM
 */

/* ECU3 */



#include <xc.h>

#include "clcd.h"
#include "can.h"
#include "msg_id.h"


void init_config(void)
{
    init_clcd();
    init_can();

    /*
     * ECU3 indicator LEDs
     *
     * RB0, RB1 -> LEFT
     * RB6, RB7 -> RIGHT
     *
     * RB2 -> CAN TX
     * RB3 -> CAN RX
     */

    TRISB0 = 0;
    TRISB1 = 0;

    TRISB6 = 0;
    TRISB7 = 0;

    /* All indicator LEDs OFF initially */
    RB0 = 0;
    RB1 = 0;
    RB6 = 0;
    RB7 = 0;
}


void main(void)
{
    unsigned int rx_msg_id = 0;

    unsigned char rx_data[8];
    unsigned char rx_len = 0;

    unsigned char gear_data[3] = "ON";
    unsigned char speed_data[3] = "00";

    unsigned int rpm = 0;

    unsigned char rpm_str[5];

    unsigned char indicator_data = 0;


    init_config();


    /*====================================================
     * INITIAL CLCD DISPLAY
     *====================================================*/

    clcd_print("GR",  LINE1(0));
    clcd_print("SP",  LINE1(3));
    clcd_print("RPM", LINE1(6));
    clcd_print("IND", LINE1(11));


    clcd_print(gear_data,  LINE2(0));
    clcd_print(speed_data, LINE2(3));
    clcd_print("0000", LINE2(6));
    clcd_print("     ", LINE2(11));


    while (1)
    {
        /*
         * ================================================
         * RECEIVE CAN MESSAGE
         * ================================================
         */

        can_receive(&rx_msg_id, rx_data, &rx_len);


        /*
         * ================================================
         * GEAR
         * ================================================
         */

        if ((rx_msg_id == GEAR_MSG_ID) && (rx_len == 2))
        {
            gear_data[0] = rx_data[0];
            gear_data[1] = rx_data[1];
            gear_data[2] = '\0';

            clcd_print(gear_data, LINE2(0));
        }


        /*
         * ================================================
         * SPEED
         * ================================================
         */

        else if ((rx_msg_id == SPEED_MSG_ID) && (rx_len == 2))
        {
            speed_data[0] = rx_data[0];
            speed_data[1] = rx_data[1];
            speed_data[2] = '\0';

            clcd_print(speed_data, LINE2(3));
        }


        /*
         * ================================================
         * RPM
         * ================================================
         */

        else if ((rx_msg_id == RPM_MSG_ID) && (rx_len == 2))
        {
            rpm = ((unsigned int)rx_data[0] << 8)
                  | rx_data[1];


            rpm_str[0] = (rpm / 1000) + '0';
            rpm_str[1] = ((rpm / 100) % 10) + '0';
            rpm_str[2] = ((rpm / 10) % 10) + '0';
            rpm_str[3] = (rpm % 10) + '0';
            rpm_str[4] = '\0';


            clcd_print(rpm_str, LINE2(6));
        }


        /*
         * ================================================
         * INDICATOR
         * ================================================
         */

        else if ((rx_msg_id == INDICATOR_MSG_ID) && (rx_len == 1))
        {
            indicator_data = rx_data[0];


            /*
             * SW1 FROM ECU2
             * LEFT INDICATOR
             */

            if (indicator_data == 1)
            {
                /* Toggle LEFT LEDs on ECU3 */

                RB0 = !RB0;
                RB1 = !RB1;

                /* RIGHT LEDs OFF */

                RB6 = 0;
                RB7 = 0;


                /* Existing CLCD - DON'T CHANGE */

                clcd_print("LEFT ", LINE2(11));
            }


            /*
             * SW2 FROM ECU2
             * RIGHT INDICATOR
             */

            else if (indicator_data == 2)
            {
                /* Toggle RIGHT LEDs on ECU3 */

                RB6 = !RB6;
                RB7 = !RB7;

                /* LEFT LEDs OFF */

                RB0 = 0;
                RB1 = 0;


                /* Existing CLCD - DON'T CHANGE */

                clcd_print("RIGHT", LINE2(11));
            }


            /*
             * SW3 FROM ECU2
             * STOP
             */

            else
            {
                /* Turn OFF all indicator LEDs */

                RB0 = 0;
                RB1 = 0;
                RB6 = 0;
                RB7 = 0;


                /* Existing CLCD - DON'T CHANGE */

                clcd_print("     ", LINE2(11));
            }
        }
    }
}
  