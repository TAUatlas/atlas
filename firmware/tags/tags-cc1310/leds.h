/*
 * leds.h
 *
 *  Created on: 17 באוג 2016
 *      Author: stoledo
 */

#ifndef LEDS_H_
#define LEDS_H_

#include <ti/drivers/PIN.h>

#include "config.h"

//#define LEDS_RED   0
//#define LEDS_GREEN 1

extern void leds_init();
extern void leds_on(uint8_t led);
extern void leds_off(uint8_t led);
extern void leds_toggle(uint8_t led);
extern void leds_blink(uint8_t led, uint8_t howmanytimes);
extern void leds_incrementBlinkCounter();
extern void leds_setBlinkCounter(uint8_t count);

#endif /* LEDS_H_ */
