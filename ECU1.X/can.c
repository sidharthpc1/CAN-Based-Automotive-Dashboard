/*
 * File:   can.c
 * Author: HP
 *
 * Created on 30 September, 2026, 2:06 PM
 */

/*ECU1*/

#include <xc.h>
#include "can.h"

typedef enum _CanOpMode
{
    e_can_op_mode_normal = 0x00,
    e_can_op_mode_loop   = 0x40,
    e_can_op_mode_config = 0x80
} CanOpMode;


void init_can(void)
{
    /* CAN_TX = RB2, CAN_RX = RB3 */
    TRISB2 = 0;
    TRISB3 = 1;

    /* Configuration mode */
    CAN_SET_OPERATION_MODE_NO_WAIT(e_can_op_mode_config);

    while (CANSTAT != 0x80);

    /* ECAN mode 0 */
    ECANCON = 0x00;

    /* CAN timing */
    BRGCON1 = 0xE1;
    BRGCON2 = 0x1B;
    BRGCON3 = 0x03;

    /* Disable filters */
    RXFCON0 = 0x00;

    /*
     * FINAL IMPLEMENTATION:
     * Normal CAN mode
     */
    CAN_SET_OPERATION_MODE_NO_WAIT(e_can_op_mode_normal);

    /* Receive configuration */
    RXB0CON = 0x00;
    RXB0CONbits.RXM0 = 1;
    RXB0CONbits.RXM1 = 1;
}


static void set_msg_id_std(unsigned int id)
{
    TXB0SIDL = (id & 0x7) << 5;
    TXB0SIDH = (id >> 3);
}


void can_transmit(uint16_t msg_id, const uint8_t *data, uint8_t len)
{
    uint8_t *ptr;
    uint8_t i;

    TXB0EIDH = 0x00;
    TXB0EIDL = 0x00;

    set_msg_id_std(msg_id);

    TXB0DLC = len;

    ptr = (uint8_t *)&TXB0D0;

    for (i = 0; i < len; i++)
    {
        ptr[i] = data[i];
    }

    TXB0REQ = 1;
}