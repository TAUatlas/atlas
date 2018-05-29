/*
 * spi-flash.h
 *
 *  Created on: Jan 30, 2018
 *      Author: Nir Zaidman
 */

#ifndef SPI_FLASH_H_
#define SPI_FLASH_H_

#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <ti/drivers/SPI.h>
#include <ti/drivers/PIN.h>
#include <ti/sysbios/knl/Task.h>
#include <ti/sysbios/knl/Clock.h>

#include "Board.h"
#include "config.h"

#ifdef FLASH_512
#define MAX_PAGE 262144
#endif
#ifdef FLASH_256
#define MAX_PAGE 131072
#endif

#define PAGE_SIZE 256
#define HEADER_SIZE 5
#define EMPTY_PAGE 0xFF

#define CONFIG_PAGE 0x0
#define BMI_160_PAGE 0x1
#define DONE_READ_PAGE 0x2

bool spiFlashReadPage(uint32_t page, uint8_t* data);
bool spiFlashWritePage(uint32_t page, uint8_t* data);
bool spiFlashEraseChip();
bool spiFlashReset();
bool spiFlashEnter4ByteMode();

#endif /* SPI_FLASH_H_ */
