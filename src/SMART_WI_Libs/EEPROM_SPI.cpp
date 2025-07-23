/**
 * @file EEPROM_SPI.cpp
 * @author Csaba Freiberger
 * @brief 
 * @version 0.1
 * @date 2024-05-24
 * 
 * @copyright Copyright (c) 2024
 * 
 */
#include "EEPROM_SPI.h"

/**
 * @brief Used to checking whether the SPI communication to the EEPROM has been initialized.
 */
static bool initialized = false;

SPISettings EEPROMSettings(8000000, MSBFIRST, SPI_MODE0);

EEPROM_SPI_Class EEPROM_SPI = EEPROM_SPI_Class::instance();

void EEPROM_SPI_Class::begin(){
    pinMode(chipSelectE, OUTPUT);

    //Set Pins PA4 - PA6 for SPI
    SPI.swap(SPI0_SWAP_DEFAULT);
    //Set to transmit MSB first
    SPI.setBitOrder(MSBFIRST);
    //Init SPI
    SPI.begin();
    // //Begin SPI communication
    //SPI.beginTransaction(EEPROMSettings);
    initialized = true;

}

bool EEPROM_SPI_Class::isInitialized(){
  return initialized;
}

void EEPROM_SPI_Class::end(){
  SPI.endTransaction();
  SPI.end();
  initialized = false;
}

EEPROM_SPI_Class::~EEPROM_SPI_Class()
{
  SPI.end();
}

uint16_t EEPROM_SPI_Class::readEEPROMStatusRegister(){
  uint16_t statusReg = {0};
  SPI.beginTransaction(EEPROMSettings);

  digitalWrite(chipSelectE, LOW);
  //delayMicroseconds(1);

  SPI.transfer(RDSR);
  // for (size_t i = 0; i < 2; i++)
  // {
    statusReg = SPI.transfer(0x00);
  //}
  
  
  digitalWrite(chipSelectE, HIGH);

  //delayMicroseconds(1);

  SPI.endTransaction();

  // Serial3.print("statusRegister: ");
  // Serial3.println(statusReg,  BIN);

  return statusReg;
}

// uint8_t EEPROM_SPI_Class::readBusyStatusLatch(){
//   uint8_t statusReg = {0};
//   SPI.beginTransaction(EEPROMSettings);

//   digitalWrite(chipSelectE, LOW);
//   //delayMicroseconds(1);

//   SPI.transfer(WRBP);
//   statusReg = SPI.transfer(0x00);
//   digitalWrite(chipSelectE, HIGH);

//   //delayMicroseconds(1);

//   SPI.endTransaction();
//   // Serial3.print("readBusyStatusLatch: ");
//   // Serial3.println(statusReg, BIN);


//   return statusReg;
// }

bool EEPROM_SPI_Class::EEPROMisBusy(){
  delayMicroseconds(50);
  uint16_t statusReg = readEEPROMStatusRegister();

  //Serial3.print("statusReg BSY: ");
  //Serial3.println(statusReg, BIN);
  return (statusReg & 0x01);
}

bool EEPROM_SPI_Class::isWriteEnabled(){
  uint16_t statusReg = readEEPROMStatusRegister();
  //Serial3.print("statusReg WEL: ");
  //Serial3.println(statusReg, BIN);
  return (statusReg & 0x02);
}

void EEPROM_SPI_Class::writeExternEEPROM(uint32_t address, const uint8_t *buf, uint16_t sizeBuf){
  uint16_t remainingBytes = sizeBuf;
  uint16_t bufferIndex = {0};
  //Wait till EEPPROM is ready
  while (EEPROMisBusy()){}

  //NEW
  while(remainingBytes > 0){
    uint16_t positionInPage = address % pageSize;
    uint16_t spaceLeftInPage = pageSize - positionInPage;
    uint16_t chunkSize = (remainingBytes < spaceLeftInPage) ? remainingBytes : spaceLeftInPage;
    //Serial3.print(F("sizeBuf: "));
    //Serial3.println(sizeBuf);
    //Begin SPI communication
    SPI.beginTransaction(EEPROMSettings);
    //First we have to enable to write to the EEPROM 
    digitalWrite(chipSelectE, LOW);
    //delayMicroseconds(1);

    //Enable to Write
    SPI.transfer(WREN);
    digitalWrite(chipSelectE, HIGH);

    //Check for WEL
    // if(isWriteEnabled()){
    //   Serial3.println(F("Enabled to write"));
    // }else
    // {
    //   Serial3.println(F("NOT YET READY TO WRITE!!"));
    // }
    

    //Send Write instruction Sequence
    digitalWrite(chipSelectE, LOW);
    SPI.transfer(WRITE);

   
  
    //Write MSB of address 16Bit at once
    address = address & 0x7FFFF;
    // Serial3.print("write to EEPROM address: ");
    // Serial3.println(address, BIN);
    SPI.transfer((byte) (address>>16)); //address>>8)
    //Write LSB of address
    SPI.transfer16(address);

    /*Write arbitary data*/
    for (uint8_t i = 0; i < chunkSize; i++)
    {
      SPI.transfer(buf[bufferIndex + i]);
      //Serial3.print("write 8 Bits: ");
      //Serial3.println(buf[i], BIN);
    }
  
    //End bit Stream
    digitalWrite(chipSelectE, HIGH);
    //delay(5);

    //Check for WEL
    // if(isWriteEnabled()){
    //   Serial3.println(F("Still Enabled to write"));
    // }else
    // {
    //   Serial3.println(F("Disabled to write"));
    // }
    //End transaction of data
    SPI.endTransaction();
    //readBusyStatusLatch();
    //Wait till EEPPROM is ready
    while (EEPROMisBusy()){}

    address += chunkSize;
    bufferIndex += chunkSize;
    remainingBytes -= chunkSize;
  }
}

void EEPROM_SPI_Class::readExternEEPROM(uint32_t address, uint8_t *buf, uint8_t sizeBuf){
    //Wait till EEPPROM is ready
    while (EEPROMisBusy()){}
    
    //Begin SPI communication
    SPI.beginTransaction(EEPROMSettings);
    digitalWrite(chipSelectE, LOW);
    //delayMicroseconds(1);

    //Send Read instruction Sequence
    SPI.transfer(READ);
    address = address & 0x7FFFF;
    // Serial3.print("Read on EEPROM address: ");
    // Serial3.println(address, BIN);
    //Read MSB (16Bits) of address first
    SPI.transfer(address>>16); //address>>8
    //Read LSB of address
    SPI.transfer16(address);
    //Read value
    for (uint8_t i = 0; i < sizeBuf; i++) //for (uint8_t i = 0; i < sizeBuf; i++)
    {
        // Serial3.print("i = ");
        // Serial3.println(i);
        //ptrValue[i]= SPI.transfer(0x00);
        buf[i] = SPI.transfer(0x00); //sizeBuf-1-i
        //Serial3.print("8 Bits read: ");
        //Serial3.println(buf[i], BIN);
    }
    //Serial3.println();

    digitalWrite(chipSelectE, HIGH);
    //delayMicroseconds(1);
 
    //End transaction of reading
    SPI.endTransaction();
}