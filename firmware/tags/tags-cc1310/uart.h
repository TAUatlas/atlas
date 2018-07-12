/*
 * uartTask.h
 *
 */

#ifndef UARTTASK_H_
#define UARTTASK_H_

#include <stdint.h>
#include <ti/drivers/UART.h>
#include <ti/sysbios/knl/Mailbox.h>

extern Mailbox_Handle uartTxMailbox;
extern Mailbox_Handle uartRxMailbox;

extern void uartTasks_init(uint32_t uart_index);

#endif /* UARTTASK_H_ */
