#ifdef VH_BASESTATION_FIRMWARE

/*
 * basestation.h
 *
 */

#ifndef BASESTATION_H_
#define BASESTATION_H_

#include <ti/sysbios/knl/Mailbox.h>

Mailbox_Handle basestationTxMailbox;

void basestationTask_init();
void basestation_gotoSetup(uint8_t c);

#endif /* BASESTATION_H_ */

#endif // base station firmware
