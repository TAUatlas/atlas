#include "spi_flash.h"

static PIN_Handle hFlashPin = NULL;
static PIN_State pinState;
static PIN_Config BoardFlashPinTable[] = {
    Board_SPI_FLASH_CS | PIN_GPIO_OUTPUT_EN | PIN_GPIO_HIGH | PIN_PUSHPULL | PIN_DRVSTR_MAX, /* Ext. flash chip select */
    PIN_TERMINATE
};

SPI_Handle      spiHandle;
SPI_Transaction spiTransaction;
uint8_t spiTxBuffer[PAGE_SIZE+HEADER_SIZE];
uint8_t spiRxBuffer[PAGE_SIZE+HEADER_SIZE];

int getWIP(){
    // read status regiter command
    spiTxBuffer[0] = 0x5;
    spiTransaction.count = 2;
    PIN_setOutputValue(hFlashPin,Board_SPI_FLASH_CS,Board_FLASH_CS_ON);
    bool ret = SPI_transfer(spiHandle, &spiTransaction);
    PIN_setOutputValue(hFlashPin,Board_SPI_FLASH_CS,Board_FLASH_CS_OFF);

    if (ret == false)
        return -1;

    return (spiRxBuffer[1] & 1);
}

bool setWEN(){
    // read status regiter command
    spiTxBuffer[0] = 0x6;
    spiTransaction.count = 1;
    PIN_setOutputValue(hFlashPin,Board_SPI_FLASH_CS,Board_FLASH_CS_ON);
    bool ret = SPI_transfer(spiHandle, &spiTransaction);
    PIN_setOutputValue(hFlashPin,Board_SPI_FLASH_CS,Board_FLASH_CS_OFF);

    // read status regiter command
    spiTxBuffer[0] = 0x5;
    spiTransaction.count = 2;
    PIN_setOutputValue(hFlashPin,Board_SPI_FLASH_CS,Board_FLASH_CS_ON);
    ret &= SPI_transfer(spiHandle, &spiTransaction);
    PIN_setOutputValue(hFlashPin,Board_SPI_FLASH_CS,Board_FLASH_CS_OFF);

    if (ret == false || (spiRxBuffer[1] & 2)== 0)
        return false;

    return true;
}

bool spiFlashEraseChip(){
    // wait until previous action is done
    while (getWIP() != 0);

    // set write enable
    if (setWEN()){
        // write command
        spiTxBuffer[0] = 0xC7;

        // write address
        spiTransaction.count = 1;
        PIN_setOutputValue(hFlashPin,Board_SPI_FLASH_CS,Board_FLASH_CS_ON);
        bool ret = SPI_transfer(spiHandle, &spiTransaction);
        PIN_setOutputValue(hFlashPin,Board_SPI_FLASH_CS,Board_FLASH_CS_OFF);
        return ret;
    }
    return false;
}

bool spiFlashReset(){
    // wait until previous action is done
    while (getWIP() != 0);

    // reset enable
    spiTxBuffer[0] = 0x66;
    spiTransaction.count = 1;
    PIN_setOutputValue(hFlashPin,Board_SPI_FLASH_CS,Board_FLASH_CS_ON);
    bool ret = SPI_transfer(spiHandle, &spiTransaction);
    PIN_setOutputValue(hFlashPin,Board_SPI_FLASH_CS,Board_FLASH_CS_OFF);

    if (ret == false)
        return false;

    Task_sleep(1000);

    // reset memory
    spiTxBuffer[0] = 0x99;
    spiTransaction.count = 1;
    PIN_setOutputValue(hFlashPin,Board_SPI_FLASH_CS,Board_FLASH_CS_ON);
    ret = SPI_transfer(spiHandle, &spiTransaction);
    PIN_setOutputValue(hFlashPin,Board_SPI_FLASH_CS,Board_FLASH_CS_OFF);

    return ret;
}

bool spiFlashWritePage(uint32_t page, uint8_t* data){
    // wait until previous action is done
    while (getWIP() != 0);

    // set write enable
    if (setWEN()){
        // write command
        spiTxBuffer[0] = 0x12;

        // write address
        page=page*PAGE_SIZE;
        spiTxBuffer[4] = page & 0xFF;
        spiTxBuffer[3] = (page & 0xFF00) >> 8;
        spiTxBuffer[2] = (page & 0xFF0000) >> 16;
        spiTxBuffer[1] = (page & 0xFF000000) >> 24;
        spiTransaction.count = PAGE_SIZE+HEADER_SIZE;
        memcpy(spiTxBuffer + HEADER_SIZE, data, PAGE_SIZE);
        PIN_setOutputValue(hFlashPin,Board_SPI_FLASH_CS,Board_FLASH_CS_ON);
        bool ret = SPI_transfer(spiHandle, &spiTransaction);
        PIN_setOutputValue(hFlashPin,Board_SPI_FLASH_CS,Board_FLASH_CS_OFF);
        return ret;
    }
    return false;
}

bool spiFlashReadPage(uint32_t page, uint8_t* data){
    // wait until previous action is done
    while (getWIP() != 0);


    // read command
    spiTxBuffer[0] = 0x13;

    // read address
    page=page*PAGE_SIZE;
    spiTxBuffer[4] = page & 0xFF;
    spiTxBuffer[3] = (page & 0xFF00) >> 8;
    spiTxBuffer[2] = (page & 0xFF0000) >> 16;
    spiTxBuffer[1] = (page & 0xFF000000) >> 24;
    spiTransaction.count = PAGE_SIZE+HEADER_SIZE;
    PIN_setOutputValue(hFlashPin,Board_SPI_FLASH_CS,Board_FLASH_CS_ON);
    bool ret = SPI_transfer(spiHandle, &spiTransaction);
    PIN_setOutputValue(hFlashPin,Board_SPI_FLASH_CS,Board_FLASH_CS_OFF);
    memcpy(data, spiRxBuffer + HEADER_SIZE, PAGE_SIZE);
    return ret;

}

void spiFlash_init() {
    hFlashPin = PIN_open(&pinState, BoardFlashPinTable);
    SPI_Params      spiparams;
    SPI_Params_init(&spiparams);
    spiparams.bitRate  = 40000;
    spiparams.mode = SPI_MASTER;
    spiparams.transferMode = SPI_MODE_BLOCKING;
    spiparams.frameFormat = SPI_POL1_PHA1; // default
    spiparams.dataSize = 8; // bits, default
    spiHandle = SPI_open(Board_SPI0 , &spiparams);

    spiTransaction.txBuf = spiTxBuffer;
    spiTransaction.rxBuf = spiRxBuffer;
}

bool spiFlashEnter4ByteMode() {
    // wait until previous action is done
    while (getWIP() != 0);

    // set 4 byte mode command
    spiTxBuffer[0] = 0xB7;

    spiTransaction.count = 1;
    PIN_setOutputValue(hFlashPin,Board_SPI_FLASH_CS,Board_FLASH_CS_ON);
    bool ret = SPI_transfer(spiHandle, &spiTransaction);
    PIN_setOutputValue(hFlashPin,Board_SPI_FLASH_CS,Board_FLASH_CS_OFF);
    return ret;
}



