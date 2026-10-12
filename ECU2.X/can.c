/*
 * File:   can.c
 * Author: HP
 *
 * Created on 30 September, 2026, 2:46 PM
 */


#include <xc.h>
#include "can.h"

/* CAN operation mode values */
typedef enum _CanOpMode
{
    e_can_op_mode_normal = 0x00,
    e_can_op_mode_loop   = 0x40,
    e_can_op_mode_config = 0x80
} CanOpMode;


/* CAN initialization */
void init_can(void)
{
    /* CAN_TX = RB2, CAN_RX = RB3 */
    TRISB2 = 0;
    TRISB3 = 1;

    /* Enter CAN module into configuration mode */
    CAN_SET_OPERATION_MODE_NO_WAIT(e_can_op_mode_config);

    /* Wait until configuration mode is set */
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
     * IMPORTANT:
     * Enter CAN module into normal mode
     */
    CAN_SET_OPERATION_MODE_NO_WAIT(e_can_op_mode_normal);

    /* Receive mode for RX buffer 0 */
    RXB0CON = 0x00;

    /* Receive all messages */
    RXB0CONbits.RXM0 = 1;
    RXB0CONbits.RXM1 = 1;
}


/* Get standard message ID */
static uint16_t get_msg_id_std(void)
{
    uint16_t id = 0;

    id = ((RXB0SIDL >> 5) & 0x7) | (RXB0SIDH << 3);

    return id;
}


/* Set standard message ID */
static void set_msg_id_std(unsigned int id)
{
    TXB0SIDL = (id & 0x7) << 5;
    TXB0SIDH = (id >> 3);
}


/* CAN transmission */
void can_transmit(uint16_t msg_id, const uint8_t *data, uint8_t len)
{
    uint8_t *ptr;
    uint8_t i;

    /* Extended identifier = 0 */
    TXB0EIDH = 0x00;
    TXB0EIDL = 0x00;

    /* Set message ID */
    set_msg_id_std(msg_id);

    /* Set data length */
    TXB0DLC = len;

    /* Data register pointer */
    ptr = (uint8_t *)&TXB0D0;

    /* Copy data into CAN transmit buffer */
    for (i = 0; i < len; i++)
    {
        ptr[i] = data[i];
    }

    /* Request transmission */
    TXB0REQ = 1;
}


/* CAN reception */
void can_receive(uint16_t *msg_id, uint8_t *data, uint8_t *len)
{
    uint8_t *ptr;
    uint8_t i;

    /* Check whether RX buffer contains a message */
    if (RXB0FUL)
    {
        /* Get message ID */
        *msg_id = get_msg_id_std();

        /* Get received data length */
        *len = RXB0DLC;

        /* Pointer to received data */
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
        /* No data available */
        *len = 0;
    }
}
