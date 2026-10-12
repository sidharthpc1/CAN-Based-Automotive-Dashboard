/*
 * File:   can.c
 * Author: HP
 *
 * Created on 1 October, 2026, 12:10 AM
 */

/* ECU3 */



/*
 * File:   can.c
 * ECU3 - Dashboard
 */

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

    /* Normal CAN mode */
    CAN_SET_OPERATION_MODE_NO_WAIT(e_can_op_mode_normal);

    /* Receive configuration */
    RXB0CON = 0x00;
    RXB0CONbits.RXM0 = 1;
    RXB0CONbits.RXM1 = 1;
}


/* Set standard 11-bit CAN message ID */
static void set_msg_id_std(unsigned int id)
{
    TXB0SIDL = (id & 0x07) << 5;
    TXB0SIDH = (id >> 3);
}


/* Get standard 11-bit CAN message ID */
static unsigned int get_msg_id_std(void)
{
    unsigned int id;

    id = ((unsigned int)RXB0SIDH << 3);
    id |= ((RXB0SIDL >> 5) & 0x07);

    return id;
}


/* CAN TRANSMIT */
void can_transmit(uint16_t msg_id,
                  const uint8_t *data,
                  uint8_t len)
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


/* CAN RECEIVE */
void can_receive(uint16_t *msg_id,
                 uint8_t *data,
                 uint8_t *len)
{
    uint8_t *ptr;
    uint8_t i;

    if (RXB0FUL)
    {
        /* Get message ID */
        *msg_id = get_msg_id_std();

        /* Get data length */
        *len = RXB0DLC;

        /* Point to received data */
        ptr = (uint8_t *)&RXB0D0;

        /* Copy received data */
        for (i = 0; i < *len; i++)
        {
            data[i] = ptr[i];
        }

        /* Clear receive flags */
        RXB0FUL = 0;
        RXB0IF = 0;
    }
    else
    {
        /* No message received */
        *len = 0;
    }
}