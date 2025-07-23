/**
 * @file TempTelemtry.cpp
 * @author Csaba Freiberger (csaba.freiberger@witzenmann.com)
 * @brief 
 * @version 0.1
 * @date 2024-10-11
 * 
 * @copyright Copyright (c) 2024
 * 
 */

#include "CRC8.h"
#include "TempTelemetry.h"

TempTelemetry::TempTelemetry(TelemetryData &telem)
{
  currentReadAddress = {0};
  is_allExtracted = false;
  numSavedTelem = {0};
  sizeTelem = sizeof(telem);
  //Begin SPI EEPROM communication, but only if it has not begun yet
  if (!EEPROM_SPI.isInitialized())
  {
    EEPROM_SPI.begin();
  }
  
  Serial3.println(F("TempTelemetry object constructed"));
}

TempTelemetry::~TempTelemetry()
{
  EEPROM_SPI.end();
  Serial3.println(F("TempTelemetry object destroyed"));
}


template <typename T, size_t N>
bool TempTelemetry::writeTillCorrectCRC(const T address, uint8_t (&frame)[N]){
  // Serial3.println(F("Call writeTillCorrectCRC uint8_t"));
  // for (size_t i = 0; i < N; i++)
  // {
  //    Serial3.println(frame[i]);
  // }
  // Serial3.print(F("address: "));
  // Serial3.println(address);
  //Check if the writing process was not currupted
  uint8_t checkCRC[N] = {0};
  uint8_t sizeCheckCRC = sizeof(checkCRC);
  // Serial3.print(F("sizeCheckCRC: "));
  // Serial3.println(sizeCheckCRC);
  uint8_t calculatedChecksum = {0};
  uint16_t timeout_ms = 2000;
  uint32_t start_ms = millis();

  //Save new frame on the EEPROM
  EEPROM_SPI.putEEPROMData(address, frame);
  //Check if frame has been written correct
  EEPROM_SPI.getEEPROMData(address, checkCRC);

  calculatedChecksum = CRC8.Compute_CRC8(checkCRC, sizeCheckCRC);

  // Serial3.print(F("Read frame: "));
  // for (size_t i = 0; i < N; i++){
  //   Serial3.println(checkCRC[i]);
  // }
  // Serial3.print(F("Read checksum: "));
  // Serial3.println(calculatedChecksum);
  
  //If not, than try it as long as it is not correct or timeout has not be reached
  while (calculatedChecksum != 0 )
  {
    if (millis() - start_ms > timeout_ms)
    {
      Serial3.println(F("[ERROR]: Timed out while writing frame to EEPROM"));
      return false;
    }
    
    //Save new page number on the EEPROM
    EEPROM_SPI.putEEPROMData(address, frame); //ADDRESS_PAGE_FLAG, pageFrame
    EEPROM_SPI.getEEPROMData(address, checkCRC); //ADDRESS_PAGE_FLAG
    
    // Serial3.print(F("Read frame: "));
    // for (size_t i = 0; i < N; i++){
    //   Serial3.println(checkCRC[i]);
    // }
    // Serial3.print(F("checksum: "));
    
    calculatedChecksum = CRC8.Compute_CRC8(checkCRC, sizeCheckCRC);
    // Serial3.println(calculatedChecksum);
    Serial3.println(F("[WARNING]: Current frame by writing currupted. Try to write it again.."));
  }
  return true;
}

bool TempTelemetry::writeTillCorrectCRC(const uint32_t address, const EEPROM_address (&frame)){
  // Serial3.println(F("Call writeTillCorrectCRC const T"));
  //uint8_t N = sizeof(T);
  // for (size_t i = 0; i < N; i++)
  // {
      //Serial3.println(frame);
  // }
  // Serial3.print(F("address: "));
  // Serial3.println(address);
  //Check if the writing process was not currupted
  // T checkCRC = {0};
  EEPROM_address checkCRC = {0};
  uint8_t sizeCheckCRC = sizeof(checkCRC);
  // Serial3.print(F("sizeCheckCRC: "));
  // Serial3.println(sizeCheckCRC);
  uint8_t calculatedChecksum = {0};
  uint16_t timeout_ms = 2000;
  uint32_t start_ms = millis();

  //Save new frame on the EEPROM
  EEPROM_SPI.putEEPROMData(address, frame);
  //Check if frame has been written correct
  EEPROM_SPI.getEEPROMData(address, checkCRC);

  calculatedChecksum = CRC8.Compute_CRC8<EEPROM_address>(checkCRC, sizeCheckCRC);

  // Serial3.print(F("Read frame: "));
  // for (size_t i = 0; i < N; i++){
  //   Serial3.println(checkCRC[i]);
  // }
  // Serial3.print(F("Read checksum: "));
  // Serial3.println(calculatedChecksum);
  
  //If not, than try it as long as it is not correct or timeout has not be reached
  while (calculatedChecksum != 0 )
  {
    if (millis() - start_ms > timeout_ms)
    {
      Serial3.println(F("[ERROR]: Timed out while writing frame to EEPROM"));
      return false;
    }
    
    //Save new page number on the EEPROM
    EEPROM_SPI.putEEPROMData(address, frame); //ADDRESS_PAGE_FLAG, pageFrame
    EEPROM_SPI.getEEPROMData(address, checkCRC); //ADDRESS_PAGE_FLAG
    
    // Serial3.print(F("Read frame: "));
    // for (size_t i = 0; i < N; i++){
    //   Serial3.println(checkCRC[i]);
    // }
    // Serial3.print(F("checksum: "));
    
    calculatedChecksum = CRC8.Compute_CRC8<EEPROM_address>(checkCRC, sizeCheckCRC);
    // Serial3.println(calculatedChecksum);
    Serial3.println(F("[WARNING]: Current frame by writing currupted. Try to write it again.."));
  }
  return true;
}


void TempTelemetry::resetTelemAddresses(){
  //Set last telemetry Address value to the beginning = zero
  EEPROM_address lastTelemAddress = {0};
  Serial3.println(F("Reset telemtry Addresses"));
  //Save new last telemtry address to the EEPROM
  writeTillCorrectCRC(ADDRESS_LAST_TELEM_ADDRESS_01, lastTelemAddress);
  //Set the current page to the first one
  uint8_t pageFrame[2] = {1, 0};
  //Variable for calculated crc value
  pageFrame[1] = CRC8.Compute_CRC8(pageFrame[0], sizeof(pageFrame[0]));
  // Save new page number on the EEPROM
  writeTillCorrectCRC(ADDRESS_PAGE_FLAG, pageFrame);
  
}


void TempTelemetry::saveTelemetry(TelemetryData &telemetry){
  //Size of telemetry
  //uint8_t sizeTelem = sizeof(telemetry);
  //Variable for the current page we are writing in
  uint8_t currentPage[2] = {1, 0};
  //Boolean for firstPage
  bool isFirstPage = false;
  //Size of currentPage
  uint8_t sizeCurrentPage = sizeof(currentPage);
  //Variable for the next free Address for writing telemetry data
  EEPROM_address currentFreEAddress = {ADDRESS_FIRST_TELEM_01, 0};
  //Variable for the last telemetry Address
  EEPROM_address lastTelemAddress = {ADDRESS_FIRST_TELEM_01, 0};
  //Size of lastTelemAddress
  uint8_t sizeTelemAddress = sizeof(lastTelemAddress);
  //Variable for calculated crc value
  uint8_t calculatedChecksum = {0};
  
  
  //calculatedChecksum = Compute_CRC8(telemFrame, sizeTelem-1);//-1
  calculatedChecksum = CRC8.Compute_CRC8<TelemetryData>(telemetry, sizeTelem-1);
  Serial3.print(F("CalculatedChecksum of telemFrame: "));
  //Serial3.println(calculatedChecksum);

  telemetry.crcValue = calculatedChecksum;
  
  Serial3.println(telemetry.crcValue);

  //Check in which page we are currently in
  EEPROM_SPI.getEEPROMData(ADDRESS_PAGE_FLAG, currentPage);
  calculatedChecksum = CRC8.Compute_CRC8(currentPage, sizeCurrentPage);
  // Serial3.print(F("currentPage: "));
  // Serial3.println(currentPage[0]);
  // Serial3.print(F("with CRC value: "));
  // Serial3.println(currentPage[1]);
  uint32_t lastAddressOfPage = (uint32_t)PAGE_SIZE * currentPage[0] - sizeTelem;
  //Address of last Telemetry Address which is at the beginning of each page
  uint32_t ADDRESS_LAST_TELEM_ADDRESS_PAGEx = {ADDRESS_LAST_TELEM_ADDRESS_01};//(uint32_t)PAGE_SIZE * (currentPage[0] - 1);

  //Check if currentPage value is correct and then read the current free Address to write in
  if (currentPage[0] <= 0 || currentPage[0] > MAX_PAGE_NUMBER || calculatedChecksum != 0){
    Serial3.println(F("[ERROR]: Currupted page value! Start new at first page at the begining"));
    resetTelemAddresses();
    currentFreEAddress = {0};
    currentFreEAddress.crc = CRC8.Compute_CRC8<uint32_t>(currentFreEAddress.value, sizeof(currentFreEAddress.value));
    currentPage[0] = {1};
    isFirstPage = true;

  }else if (currentPage[0] == 1){
    isFirstPage = true;
    //Get the last telemetry EEPROM Address in current page, where the last data was written
    EEPROM_SPI.getEEPROMData(ADDRESS_LAST_TELEM_ADDRESS_PAGEx, currentFreEAddress);
    // Serial3.print(F("Last telemetry address is: "));
    // Serial3.println(currentFreEAddress.value);
    // Serial3.print(F("with crc: "));
    // Serial3.println(currentFreEAddress.crc);
  } else{
    //Get the last telemetry EEPROM Address in current page, where the last data was written
    ADDRESS_LAST_TELEM_ADDRESS_PAGEx = (uint32_t)PAGE_SIZE * (currentPage[0] - 1);
    EEPROM_SPI.getEEPROMData(ADDRESS_LAST_TELEM_ADDRESS_PAGEx, currentFreEAddress);
    // Serial3.print(F("Last telemetry address is: "));
    // Serial3.println(currentFreEAddress.value);
    // Serial3.print(F("with crc: "));
    // Serial3.println(currentFreEAddress.crc);
  }

  //Check if value of the last telemtry EEPROM Address in current page is currupted
  calculatedChecksum = CRC8.Compute_CRC8<EEPROM_address>(currentFreEAddress, sizeTelemAddress);

  //Check if something went wrong or the current free Address is currupted
  if (currentFreEAddress.value >= MAX_25CSM04_ADDRESS || calculatedChecksum != 0){
    Serial3.println(F("ERROR: Currupted last telemtry address. Start new at first page at the begining"));
    //Set everything to the beginning of the first page
    resetTelemAddresses();
    currentFreEAddress.value = ADDRESS_FIRST_TELEM_01;
    currentPage[0] = {1};
    isFirstPage = true;
    //Set new Address for the last telemetry address
    ADDRESS_LAST_TELEM_ADDRESS_PAGEx = ADDRESS_LAST_TELEM_ADDRESS_01;
    

  }else if (currentFreEAddress.value == 0){   //Check if we are at the beginning of the EEPROM page
    if (isFirstPage)
    {
      currentFreEAddress.value = ADDRESS_FIRST_TELEM_01;
      // Serial3.println(F("We are at the beginning of the first page"));
    }else{
      //Set the address for the current free address at the beginning of each page
      //Plus size of last telem Address, as at the begining of every page, the first Bytes are reserved for the last telemetry address
      currentFreEAddress.value = ADDRESS_LAST_TELEM_ADDRESS_PAGEx + sizeTelemAddress; 
      // Serial3.print(F("We are at the beginning of page "));
      // Serial3.println(currentPage[0]);
    }
    
  }else if (currentFreEAddress.value >= lastAddressOfPage){ //Check if we are at the end of current page
    // Serial3.print(F("Reached end of page "));
    // Serial3.println(currentPage[0]);
    
    if (currentPage[0] == MAX_PAGE_NUMBER){  //if we are in the last page, go to the first
      // Serial3.println(F("Go to the first page at the beginning"));
      currentPage[0] = {1};
      currentPage[1] = CRC8.Compute_CRC8(currentPage[0], sizeof(currentPage[0]));
      isFirstPage = true;
      //Set new Address for the last telemetry address
      ADDRESS_LAST_TELEM_ADDRESS_PAGEx = ADDRESS_LAST_TELEM_ADDRESS_01;
      //Set new Address for the next free telemetry address
      currentFreEAddress.value = ADDRESS_FIRST_TELEM_01;
      
    }else{  //else increase the page number
      // Serial3.println(F("Go to the next page"));
      currentPage[0]++;
      currentPage[1] = CRC8.Compute_CRC8(currentPage[0], sizeof(currentPage[0]));
      //Calculate new Address for the last telemetry address
      ADDRESS_LAST_TELEM_ADDRESS_PAGEx = (uint32_t)PAGE_SIZE * (currentPage[0] - 1);
      //Plus size of last telem Address, as at the begining of every page, the first Bytes are reserved for the last telemetry address
      currentFreEAddress.value = ADDRESS_LAST_TELEM_ADDRESS_PAGEx + sizeTelemAddress;
      
    } 
    //Check if the writing process was not currupted
    writeTillCorrectCRC(ADDRESS_PAGE_FLAG, currentPage);
  }else{
    //Increase address on EEPROM by the size of the previously written data to get the actual current next free address
    // Serial3.println(F("Increase current free EEPROM Address by size of telemetry frame"));
    currentFreEAddress.value += sizeTelem;  
  }
  
  
  // Serial3.print("put data to current free EEPROM Address: ");
  // Serial3.println(currentFreEAddress.value);
  // Serial3.print(F("in page number "));
  // Serial3.println(currentPage[0]);
  // Serial3.print("with crc value: ");
  // Serial3.println(currentPage[1]);
    
  //Put data frame to the current free EEPROM address
  EEPROM_SPI.putEEPROMData(currentFreEAddress.value, telemetry); /*telemetry*/
  //Update last telemetry address  
  lastTelemAddress.value = currentFreEAddress.value;
  
  //Minus last element, cause we do not need the last element as it is the CRC we calculate
  uint8_t sizeForCRC = sizeTelemAddress - 1;

  lastTelemAddress.crc = CRC8.Compute_CRC8<uint32_t>(currentFreEAddress.value, sizeForCRC);
  
  // Serial3.print("put Address of last Telemetry: ");
  // Serial3.println(lastTelemAddress.value,BIN);
  // Serial3.println(lastTelemAddress.crc,BIN);
  // Serial3.println(lastTelemAddress.value);
  // Serial3.println(lastTelemAddress.crc);
  // Serial3.println();

  //Flag for new saved telemetry data
  uint8_t savedNewTelem[2] = {1, 0};
  /*Get first and update only if it is different!*/
  savedNewTelem[1] = CRC8.Compute_CRC8(savedNewTelem[0], sizeof(savedNewTelem[0]));

  uint8_t is_savedNewTelem[2] = {0};
  EEPROM_SPI.getEEPROMData(ADDRESS_SAVED_TELEM_FLAG, is_savedNewTelem);
  if (is_savedNewTelem[0] != savedNewTelem[0] || is_savedNewTelem[1] != savedNewTelem[1])
  {
    Serial3.println(F("Update savedNewTelem with: "));
    Serial3.println(savedNewTelem[0]);
    Serial3.println(savedNewTelem[1]);
    if(!writeTillCorrectCRC(ADDRESS_SAVED_TELEM_FLAG, savedNewTelem)){
      Serial3.println(F("[WARNING]: Failed to set saved new telemetry flag! Data may be ignored if extracted"));
    }
  }

  /*Update the current page and the last telemtry address*/
  if(currentPage[0] == MAX_PAGE_NUMBER){
    //Put the last telemetry EEPROM Address in current page, where the last data was written
    //EEPROM_SPI.putEEPROMData(ADDRESS_LAST_TELEM_ADDRESS_PAGEx, lastTelemAddress);
    if(!writeTillCorrectCRC(ADDRESS_LAST_TELEM_ADDRESS_PAGEx, lastTelemAddress)){
      Serial3.println(F("[ERROR]: Failed to write new value for the last telemtry Address. Go to the beginning of the first page"));
      currentPage[0] = {1};
      currentPage[1] = CRC8.Compute_CRC8(currentPage[0], sizeof(currentPage[0]));
      isFirstPage = true;
      writeTillCorrectCRC(ADDRESS_PAGE_FLAG, currentPage);
      // lastTelemAddress[0] = {0};
      // lastTelemAddress[1] = Compute_CRC8(lastTelemAddress, sizeForCRC);
      lastTelemAddress.value = {0};
      lastTelemAddress.crc = CRC8.Compute_CRC8<EEPROM_address>(lastTelemAddress, sizeForCRC);
      writeTillCorrectCRC(ADDRESS_LAST_TELEM_ADDRESS_01, lastTelemAddress);
    }

  }else{
    //Put the last telemetry EEPROM Address in current page, where the last data was written
    if(!writeTillCorrectCRC(ADDRESS_LAST_TELEM_ADDRESS_PAGEx, lastTelemAddress)){
      Serial3.println(F("[ERROR]: Failed to write new value for the last telemtry Address. Go to the beginning of the next page"));
      currentPage[0]++;
      currentPage[1] = CRC8.Compute_CRC8(currentPage[0], sizeof(currentPage[0]));
      writeTillCorrectCRC(ADDRESS_PAGE_FLAG, currentPage);

      lastTelemAddress.value = {0};
      lastTelemAddress.crc = CRC8.Compute_CRC8<EEPROM_address>(lastTelemAddress, sizeForCRC);
      //Calculate new Address for the last telemetry address
      ADDRESS_LAST_TELEM_ADDRESS_PAGEx = (uint32_t)PAGE_SIZE * (currentPage[0] - 1);
      writeTillCorrectCRC(ADDRESS_LAST_TELEM_ADDRESS_PAGEx, lastTelemAddress);
    } 
  }
  
}


void TempTelemetry::printTelemetry(TelemetryData &_telemetry){
  Serial3.println(F("The telemetry is: "));
  Serial3.println(_telemetry.temp1);
  Serial3.println(_telemetry.temp2);
  Serial3.println(_telemetry.deflection);
  Serial3.println(_telemetry.pressure);
  Serial3.println(_telemetry.picTemp);
  //Serial3.println(_telemetry.timestamp);
  Serial3.println(_telemetry.deviceID);
  Serial3.println(_telemetry.crcValue);
}

bool TempTelemetry::checkForNewSavedTelem(){
  //Serial3.println(F("Begin ckeckForSavedTelem"));
  uint8_t calculatedChecksum = {0};
  uint8_t savedNewTelem[2] = {0};
  EEPROM_SPI.getEEPROMData(ADDRESS_SAVED_TELEM_FLAG, savedNewTelem);
  // Serial3.print(F("The saved telem flag is: "));
  // Serial3.println(savedNewTelem[0]);
  // Serial3.println(savedNewTelem[1]);
  calculatedChecksum = CRC8.Compute_CRC8(savedNewTelem, sizeof(savedNewTelem));
  if (calculatedChecksum == 0)
  {
    if (savedNewTelem[0] == 1)
    {
      Serial3.println(F("There are new saved telemetry data set(s) on the external EEPROM"));
      return true;
    }else
    {
      Serial3.println(F("There are no saved telemetry data set on the external EEPROM"));
      return false;
    }

  }else{
    Serial3.println(F("[ERROR]: Currupted flag for saved new telemetry. All saved data are ignored"));
    return false;
  }
}

uint32_t TempTelemetry::getNumSavedTelem(){
  return numSavedTelem;
}

uint32_t TempTelemetry::calcNumSavedTelem(uint32_t _firstTelemAddress, uint32_t _lastTelemAddress){
  // Serial3.println("Call calcNumSavedTelem");
  // Serial3.print(F("_firstTelemAddress: "));
  // Serial3.println(_firstTelemAddress);
  // Serial3.println("_lastTelemAddress: ");
  // Serial3.println(_lastTelemAddress);
  uint32_t res = _lastTelemAddress - _firstTelemAddress;
  if (res == 0)
  {
    /*Only one telemtry set is saved*/
    return res = {1};
  }else{
    /*More then one telemetry is saved */    
    // Serial3.print(F("res befor division: "));
    // Serial3.println(res);
    //Plus one, as we begin to count from address 0
    return res / sizeTelem + 1;
  }
}

bool TempTelemetry::extractAllTelemetry(uint8_t (&_currentPage)[2], EEPROM_address &_lastTelemAddress, TelemetryData &_savedTelemetry){
  //Variable for CRC value
  uint8_t calculatedChecksum = {0};
  //Size of telemetry data
  //uint8_t sizeTelem = sizeof(_savedTelemetry);


  Serial3.println(F("Begin extractAllTelemtry()")); 

  // Serial3.print("currentReadAddress = ");
  // Serial3.println(currentReadAddress);

  /*Check if only one telemetry data has to be "sent" (extracted)*/
  if (currentReadAddress == _lastTelemAddress.value){
    //Extraxt the only telemetry and save it to the savedTelemetry
    if(extractOnlyTelemetry(_lastTelemAddress.value, _savedTelemetry)){
      //Update the page value and the address of last telemetry address
      updateTelemAddresses(_currentPage, _lastTelemAddress);
      is_allExtracted = true;
      return true;
    }else{
       return false;
    }

  }else{
    is_allExtracted = false;
    //Read telemtry data ///until last address///
    //while (currentReadAddress != lastTelemAddress.value){
      EEPROM_SPI.getEEPROMData(currentReadAddress, _savedTelemetry);
      // Serial3.print("result read on address ");
      // Serial3.print(currentReadAddress);
      // Serial3.println(" : ");
      // printTelemetry(_savedTelemetry);

      // Serial3.print(F("CalculatedChecksum of saved TelemtryData: "));
      // Serial3.println(calculatedChecksum);

      //Update currentReadAddress by size of saved telemetry data
      currentReadAddress += sizeTelem;
    
      //Catch overflow of EEPROM address pointer
      if (currentReadAddress >= (MAX_25CSM04_ADDRESS)){
        currentReadAddress = ADDRESS_FIRST_TELEM_01;
      } 

      //Check if the saved telemtrey data was currupted
      calculatedChecksum = CRC8.Compute_CRC8<TelemetryData>(_savedTelemetry, sizeTelem);
      if (calculatedChecksum!=0){
        Serial3.println(F("[ERROR]: Currupted telemetry data on EEPROM. Data is ignored"));
        return false;
      }else{
        Serial3.println(F("Correct telemetry :)"));
        return true;
      } 
    
  }

}


bool TempTelemetry::extractOnlyTelemetry(uint32_t _lastTelemAddress, TelemetryData &_savedTelemetry){
  Serial3.println(F("Call extractOnlyTelemetry()"));
  uint8_t calculatedChecksum = {0};
  //Size of telemetry data
  EEPROM_SPI.getEEPROMData(_lastTelemAddress, _savedTelemetry);
  // Serial3.print("result read on address ");
  // Serial3.print(_lastTelemAddress.value);
  // Serial3.println(" : ");
  // printTelemetry(_savedTelemetry);

  //Check if the saved telemtrey data was currupted
  calculatedChecksum = CRC8.Compute_CRC8<TelemetryData>(_savedTelemetry, sizeTelem);
  // Serial3.print(F("CalculatedChecksum of saved TelemtryData: "));
  // Serial3.println(calculatedChecksum);
  if (calculatedChecksum!=0){
    Serial3.println(F("[ERROR]: Currupted telemetry data on EEPROM. Data is ignored"));
    return false;
  }else{
    Serial3.println(F("Correct telemetry :)"));
    return true;
  }
}

bool TempTelemetry::allTelemExtracted(void){
  return is_allExtracted;
}

bool TempTelemetry::initTelemAddresses(uint8_t (&_currentPage)[2], EEPROM_address &_lastTelemAddress){
  currentReadAddress = 0;
  Serial3.println(F("Call initTelemAddress()"));
  //Size of currentPage
  uint8_t sizeCurrentPage = sizeof(_currentPage);
  //Variable for the first telemetry Address 
  uint32_t firstTelemAddress = ADDRESS_FIRST_TELEM_01;
  //Size of lastTelemAddress
  uint8_t sizeTelemAddress = sizeof(_lastTelemAddress);
  //Address of current last telemetry address
  uint32_t ADDRESS_CURRENT_LAST_TELEM_ADDRESS = {0};
  //Variable for CRC checksum
  uint8_t calculatedChecksum = {0};

  /*Check in which page we are currently in*/
  EEPROM_SPI.getEEPROMData(ADDRESS_PAGE_FLAG, _currentPage);
  // Serial3.print(F("currentPage: "));
  // Serial3.println(_currentPage[0]);
  // Serial3.print(F("with CRC: "));
  // Serial3.println(_currentPage[1]);
  calculatedChecksum = CRC8.Compute_CRC8(_currentPage, sizeCurrentPage);
  if (_currentPage[0] <= 0 || _currentPage[0] > MAX_PAGE_NUMBER || calculatedChecksum != 0){
    Serial3.println(F("[ERROR]: Currupted page value! Ignoring all data. Go to the beginning of the first page"));
    //Reset the page number and address of last telemtry address
    resetTelemAddresses();
    return false;
  }
  //Address of last Telemetry Address which is at the beginning of each page
  const uint32_t ADDRESS_LAST_TELEM_ADDRESS_PAGEx = (uint32_t)PAGE_SIZE * (_currentPage[0] - 1);
  // Serial3.print("ADDRESS_LAST_TELEM_ADDRESS_PAGEx: ");
  // Serial3.println(ADDRESS_LAST_TELEM_ADDRESS_PAGEx);

  /*Calculate the address of the first telemetry data*/
  if (_currentPage[0] == 1){
    //Set the address for the first telemetry data in the first page
    firstTelemAddress = ADDRESS_FIRST_TELEM_01;
    ADDRESS_CURRENT_LAST_TELEM_ADDRESS = ADDRESS_LAST_TELEM_ADDRESS_01;

  }else{
    //Set the address for the first telemetry data in the X page
    firstTelemAddress = ADDRESS_LAST_TELEM_ADDRESS_PAGEx + sizeTelemAddress;
    ADDRESS_CURRENT_LAST_TELEM_ADDRESS = ADDRESS_LAST_TELEM_ADDRESS_PAGEx;
  }
  // Serial3.print("firstTelemAddress: ");
  // Serial3.println(firstTelemAddress);

  //Set the current read Address "pointer" to the Address of the first telemetry data
  currentReadAddress = firstTelemAddress;
  
  //Get the value of the address for the last telemetry data 
  EEPROM_SPI.getEEPROMData(ADDRESS_CURRENT_LAST_TELEM_ADDRESS, _lastTelemAddress);
  // Serial3.print("Value of lastTelemAddress: ");
  // Serial3.println(_lastTelemAddress.value);
  // Serial3.print(F("with CRC: "));
  // Serial3.println(_lastTelemAddress.crc);
  calculatedChecksum = CRC8.Compute_CRC8<EEPROM_address>(_lastTelemAddress, sizeof(_lastTelemAddress));

  //Value of maximum possible Telemetry Address in a page
  const uint32_t maxValueTelemAddress = (uint32_t)PAGE_SIZE*_currentPage[0];
  // Serial3.print("maxValueTelemAddress: ");
  // Serial3.println(maxValueTelemAddress);

  /*Catch wrong last Telemtry Address*/
  if (_lastTelemAddress.value == 0 || _lastTelemAddress.value > maxValueTelemAddress || calculatedChecksum != 0){
    Serial3.println(F("[ERROR]: Currupted last Telemetry address. Ignoring all data. Go to the beginning of the next page"));
    //Update the page value and the last telemetry address
    updateTelemAddresses(_currentPage, _lastTelemAddress);
    
    return false;
  }else{
    //Calculate the number nof saved telemetry data
    numSavedTelem = calcNumSavedTelem(firstTelemAddress, _lastTelemAddress.value);
    return true;
  }
   
}

void TempTelemetry::updateTelemAddresses(uint8_t (_currentPage)[2], EEPROM_address _lastTelemAddress){
  uint8_t myCurrentPage[2] = {_currentPage[0], _currentPage[1]};
  Serial3.println(F("Call updateTelemAddress()"));
  uint8_t sizeTelemAddress = sizeof(_lastTelemAddress);
  uint32_t _AddressCurrentLastTelemAddress = {0};
  uint8_t savedNewTelem[2] = {0};
  savedNewTelem[1] = CRC8.Compute_CRC8(savedNewTelem[0], sizeof(savedNewTelem[0]));
  writeTillCorrectCRC(ADDRESS_SAVED_TELEM_FLAG, savedNewTelem);

  //Go to the next page of EEPROM 
  if (myCurrentPage[0] == MAX_PAGE_NUMBER){
    myCurrentPage[0] = {1};
    myCurrentPage[1] = CRC8.Compute_CRC8(myCurrentPage[0], sizeof(myCurrentPage[0]));
    _AddressCurrentLastTelemAddress = ADDRESS_LAST_TELEM_ADDRESS_01;
      
  }else{
    myCurrentPage[0]++;
    myCurrentPage[1] = CRC8.Compute_CRC8(myCurrentPage[0], sizeof(myCurrentPage[0]));
    _AddressCurrentLastTelemAddress =  (uint32_t)PAGE_SIZE * (myCurrentPage[0] - 1);
  } 

  //Put updated page value & check if the writing process was not currupted
  writeTillCorrectCRC(ADDRESS_PAGE_FLAG, myCurrentPage);//
  //Set value of last Telem Addres to the beginnning
  _lastTelemAddress.value = {0};
  _lastTelemAddress.crc = CRC8.Compute_CRC8<EEPROM_address>(_lastTelemAddress, sizeTelemAddress - 1);
  //Put updated address of last telemetry & check if the writing process was not currupted
  writeTillCorrectCRC(_AddressCurrentLastTelemAddress, _lastTelemAddress);  
    
  // Serial3.print(F("New page is: "));
  // Serial3.println(_currentPage[0]);
  // Serial3.print(F("with new CRC value: "));
  // Serial3.println(_currentPage[1]);
  // Serial3.print(F("New last telemetry address is: "));
  // Serial3.println(_lastTelemAddress.value);
  // Serial3.print(F("with new CRC value: "));
  // Serial3.println(_lastTelemAddress.crc);
  // Serial3.println();
}