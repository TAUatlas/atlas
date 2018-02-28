/*
 * spi-falsh.h
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

bool readPage(uint32_t page, uint8_t* data);
bool writePage(uint32_t page, uint8_t* data);
bool eraseChip();

#endif /* SPI_FLASH_H_ */
