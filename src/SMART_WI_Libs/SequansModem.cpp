/*Definitions of the SequansModem member functions
Created by Csaba Freiberger 02.2023*/
#if defined (DXCORE)
#include "SequansModem.h"
#include "Arduino.h"
//#include "avr/wdt.h"
#include "watchdogAVR.h"

#define AT_GET_MODEM_INFO   "ATI"
#define AT_CONNECT          "AT+CFUN=1"
#define AT_GET_CLOCK        "AT+CCLK?"
#define CEREG_CALLBACK      "CEREG"
//#define PACKET_SERVICE      "CGATT"
#define STAT_REGISTERED_HOME_NETWORK '1'
#define STAT_REGISTERED_ROAMING      '5'

//staticvolatile bool SequansModem::gprs_connected = false;

/**
 * @brief Lambda function for resetting Watchdog in functions which need more than 8s
 * 
 */
const auto wdtReset = []{Watchdog.reset(); Serial3.print(".");};

SequansModem::SequansModem(void){


}

SequansModem::~SequansModem(void){
    
}

bool SequansModem::init(void){
    if (!SequansController.isInitialized())
    {
        SequansController.begin();
    }
    if (SequansController.isInitialized())
    {
        return true;
    }else return false;
    
}

bool SequansModem::restart(){
    /*Until now this solution*/
    // if (!Lte.isConnected())
    // {
    //     if (Lte.begin(7000U))
    //     {
    //         return true;
    //     }else return false;
    // }else return true;   

    /*Maybe only init modem*/
    Watchdog.reset();
    char resbuf[100] = "";
    //char res[50] = "";
    if (!SequansController.isInitialized()) {
        if (!SequansController.begin()) {
            Watchdog.reset();
            return false;
        }else{
            Watchdog.reset();
            if(SequansController.writeCommand("AT+CFUN=1")!=ResponseResult::OK){
                Watchdog.reset();
                Serial3.println(F("Command (CFUN) response NOT OK"));
                return false;
            }
            //Wait for the first CEREG URC, as the modem sends usually CEREG: 2 as response
            if(!SequansController.waitForURC(CEREG_CALLBACK, resbuf, sizeof(resbuf), 60000U, wdtReset, 1000U)){
                return false;
            }else{
                Watchdog.reset();
                Serial3.print(F("CFUN respone 1.1: "));
                Serial3.println(resbuf);
                /*Check if the status is already ok*/
                const char stat = resbuf[2];
                Serial3.println(stat);
                if (stat == STAT_REGISTERED_HOME_NETWORK || stat == STAT_REGISTERED_ROAMING){
                    return true;
                }else{
                    //Wait for the second CEREG URC, which is now the whole response, only if it is necessary
                    if(!SequansController.waitForURC(CEREG_CALLBACK, resbuf, sizeof(resbuf), 60000U, wdtReset, 1000U)){
                        return false;
                    }else{
                        Watchdog.reset();
                        Serial3.print(F("CFUN respone 1.2: "));
                        Serial3.println(resbuf);
                        return true;
                    }
                }
            }
        }
            
    }else{
        Watchdog.reset();
        SequansController.writeCommand("AT+CFUN?");
        char res[10];
        SequansController.waitForURC("CFUN", res, sizeof(res), 10000U, wdtReset, 1000);
        Serial3.print(F("CFUN? res:"));
        Serial3.println(res);
        if (res[1]=='1')
        {   
            Serial3.println(F("Modem already initialized"));
            return true;
        }
        
        if(SequansController.writeCommand("AT+CFUN=1")!=ResponseResult::OK){
            Watchdog.reset();
            Serial3.println(F("Command (CFUN) response NOT OK"));
            return false;
        }
        //Wait for the first CEREG URC, as the modem sends usually CEREG: 2 as response
        if(!SequansController.waitForURC(CEREG_CALLBACK, resbuf, sizeof(resbuf), 60000U, wdtReset, 1000U)){
            return false;
        }else{
            Watchdog.reset();
            Serial3.print(F("CFUN respone 2.1: "));
            Serial3.println(resbuf);
            /*Check if the status is already ok*/
            const char stat = resbuf[2];
            Serial3.println(stat);
            if (stat == STAT_REGISTERED_HOME_NETWORK || stat == STAT_REGISTERED_ROAMING){
                return true;
            }else{
                //Wait for the second CEREG URC, which is now the whole response, only if it is necessary
                if(!SequansController.waitForURC(CEREG_CALLBACK, resbuf, sizeof(resbuf), 60000U, wdtReset, 1000U)){
                    return false;
                }else{
                    Serial3.print(F("CFUN respone 2.2: "));
                    Serial3.println(resbuf);
                    return true;
                }
            }
        }
    } 
}

bool SequansModem::reset(){
    char resbuf[30];
    SequansController.writeCommand("AT^RESET");
    if (!SequansController.waitForURC(F("SYSSTART"), resbuf, sizeof(resbuf), 20000U, wdtReset, 1000)) {
        Serial3.println(F("Timed out waiting for cellular modem to start up"));

        // End the controller to deattach the interrupts
        SequansController.end();

        return false;
    }else
    {
        return true;
    }
    
}

void SequansModem::end(){
    Lte.end();
}

bool SequansModem::testAT(){
    if(SequansController.writeCommand("AT") != ResponseResult::OK){
        return false;
    }else return true;
}

bool SequansModem::changeOparatingMode(){
    char resbuf[30];
    //char res[10];
    uint16_t opMode = 0;

    /*Check if modem is not busy with something*/
    if(!testAT()){
        return false;
    }else{
        Watchdog.reset();
    }

    /*Check which modem firmware version is beeing used, as changing to NB-IOT is only possible for from v.8.2.0.2*/
    String modemInfo = getModemInfo();
    int startFirmwareversion = modemInfo.indexOf(" UE");

    bool newestFirmwareVersion = false;
    if (startFirmwareversion>=0){
        String firmwareVersion = modemInfo.substring(startFirmwareversion+3, startFirmwareversion+10);
        /*Get from substring the first thre numbers of the firmware version*/
        firmwareVersion.replace(".","");

        if (firmwareVersion.toInt()>=8202){
            newestFirmwareVersion = true;
            Serial3.println(F("modem has NB-IoT enabled"));
        }else{
            newestFirmwareVersion = false;
            Serial3.println(F("modem has NB-IoT not enabled"));
        }
    
    }
    
    /*First we have to change functionality, before changing cellular mode*/
    if(SequansController.writeCommand("AT+CFUN=0")!=ResponseResult::OK){
        Watchdog.reset();
        Serial3.println(F("Command (CFUN) response NOT OK"));
        return false;
    }else{
        Watchdog.reset();
    }
    
    /*Check in which cellular mode we are currently in*/
    if (SequansController.writeCommand("AT+SQNMODEACTIVE?")!=ResponseResult::OK){
       Serial3.println(F("Failed to check cellular mode"));
       return false;
    }else{
        if(!SequansController.waitForURC("SQNMODEACTIVE", resbuf, sizeof(resbuf), 10000U, wdtReset, 1000)){
            Serial3.println(F("Timed out while waiting for cellular mode response"));
            return false;
        }else{
            Serial3.print(F("SQNMode resbuf: "));
            Serial3.println(resbuf);

            Watchdog.reset();
            sscanf(resbuf,"%u", &opMode);       
        }
        
    }

    /*Now change the mode to the other cellular mode*/
    switch (opMode){
        case 1: {
            if(!newestFirmwareVersion){
                Serial3.println(F("Changing to NB-IoT not posible with this modem firmware version. Plaese update your firmware first!"));
                return false;
            }
            Serial3.println(F("The modem is in LTE-M mode. Change to NB-IoT mode.."));
            /*here check if modem has correct version*/
            if(SequansController.writeCommand("AT+SQNMODEACTIVE=2")!=ResponseResult::OK){
                Serial3.println("Failed to change to NB-IoT mode");
                return false;
            }else{
                if(!reset()){
                    return false;
                }else{
                    if(!restart()){
                        return false;
                    }else{
                        Serial3.println(F("Change to NB-IoT successfull"));
                        return true;
                    }   
                }
            }
            break;
        }
        case 2: {
            Serial3.println(F("The modem is in NB-IoT mode. Change to LTE-M mode.."));
            if(SequansController.writeCommand("AT+SQNMODEACTIVE=1")!=ResponseResult::OK){
                Serial3.println("Failed to change to LTE-M mode");
                return false;
            }else{
                if(!reset()){
                    return false;
                }else{
                    if(!restart()){
                        return false;
                    }else{
                        Serial3.println(F("Change to LTE-M successfull"));
                        return true;
                    }
                
                }
            }
        }
        default: {
            Serial3.print(F("The modem has somting else: "));
            Serial3.println(opMode);
            SequansController.writeCommand("AT+CFUN=1");
            return true;
            break;
        }
    }
    
    return false;
}

String SequansModem::getModemInfo(){
    String res="";
    char resbuf[100];
    SequansController.writeCommand(AT_GET_MODEM_INFO,resbuf,sizeof(resbuf));
    res = resbuf;
    res.replace("\r\nOK\r\n", "");
    res.replace("\rOK\r", "");
    res.replace("\r\n", " ");
    res.replace("\r", " ");
    res.trim();

    return res;
}

SimStatus SequansModem::getSimStatus(uint32_t timeout_ms){
    String res="";
    char resbuf[100];

    for(uint32_t start = millis(); millis()-start<timeout_ms;){
        SequansController.writeCommand("AT+CPIN?",resbuf,sizeof(resbuf));
        res = resbuf;
        res.replace("\r\n+CPIN: ", "");
        res.replace("+CPIN: ","");
        res.trim();
    
        if (res.equals("READY"))
        {
            return SIM_READY;
        }else if (res.equals("SIM PIN"))
        {
            return SIM_LOCKED;
        }else if (res.equals("SIM PUK"))
        {
            return SIM_LOCKED;
        }else if (res.equals("SIM PIN2"))
        {
            return SIM_LOCKED;
        }else if(res.equals("ERROR")){
            return SIM_ERROR;
        }  
    }
    return SIM_TIMEOUT;
}

bool SequansModem::simUnlock(const char* pin){
    String atCommand = "AT+CPIN=\"";
    atCommand = atCommand+pin+"\"";
    const char* atCommandCPIN = atCommand.c_str();
    String res="";
    char resbuf[100];
    SequansController.writeCommand(atCommandCPIN,resbuf,sizeof(resbuf));
    Serial3.print(F("CPIN resbuf: "));
    Serial3.println(resbuf);
    res = resbuf;
    res.replace("\r\nOK\r\n", "");
    res.replace("\rOK\r", "");
    res.replace("\r\n", " ");
    res.replace("\r", " ");
    res.trim();

    if (res.equals("ERROR"))
    {
        return false;
    }else return true;   
}

bool SequansModem::gprsConnect(const char* apn, const char* user,
                       const char* pwd){
    //Check if Lte.begin() and network connection was successfull
    // if (!Lte.isConnected())
    // {
    //     if (Lte.begin(7000U))
    //     {
    //         return true;
    //     }else return false;
    // }else return true;
    return true;
}

void SequansModem::GprsStatus(char * buffer){
    const char stat = buffer[0];
    // Serial3.print(F("gpsr stat: "));
    // Serial3.println(stat);
    if (stat == '1')
    {
        gprs_connected = true;
    }else gprs_connected = false;
    
}

String SequansModem::getLocalIP(){
    char res[16] = "";
    char resbuf[50] = "";
    if (SequansController.writeCommand("AT+CGPADDR=1", resbuf, sizeof(resbuf))!=ResponseResult::OK) { 
        Serial3.println(F("Command (CGPADDR) response NOT OK")); 
        return ""; 
    }
    // Serial3.print(F("+CGPADDR resbuf: "));
    // Serial3.println(resbuf);
    if(!SequansController.extractValueFromCommandResponse(resbuf, 1, res, sizeof(res), 0)){
        Serial3.println(F("[ERROR]: Failed to extract response from CGPADDR!"));
        return "";
    }
    //Serial3.print(F("+CGPADDR resC: "));
    //Serial3.println(res);
    String localIP = res;
    localIP.trim();
    return localIP;
}

bool SequansModem::isGprsConnected(){
    char res[24] = "";
    char resbuf[48] = "";
    String PACKET_SERVICE = "CGATT";
    uint16_t timeout_ms = 1000;

    while(SequansController.writeCommand("AT+CGATT?", resbuf, sizeof(resbuf)) != ResponseResult::OK && --timeout_ms != 0){
        Watchdog.reset();
        delay(50);
    }
    SequansController.extractValueFromCommandResponse(resbuf,0,res,sizeof(res));
    // res = resbuf;
    // res.replace("\r\n+"+PACKET_SERVICE+": \r\n","");
    // res.replace("\r\n+"+PACKET_SERVICE+": \n","");
    // res.replace("\r\n+"+PACKET_SERVICE+": ","");
    // res.replace("\r+"+PACKET_SERVICE+": \r","");
    //char buffer[1];
    //res.toCharArray(resbuf,res.length());
    // Serial3.print(F("isGprs res: "));
    // Serial3.println(res);
    GprsStatus(res);
    /*Check not only if PDP is attached but also if it is connected to an IP-Address*/
    if (gprs_connected){
        String localIP = getLocalIP();
        if (localIP != "" && localIP != "0.0.0.0")
        {
            return true;
        }else
        {
            return false;
        } 
    }
    
    return false;//gprs_connected;
  }

  bool SequansModem::waitForNetwork(uint32_t timeout_ms, bool check_signal, bool changeOPMode){
    for (uint32_t start = millis(); millis() - start < timeout_ms;)
    {
        Watchdog.reset();
        if (check_signal) 
        { 
            if(getSignalQuality()!=99){
                if(isNetworkConnected()){
                    Serial3.println(F("Signal and connected"));
                    return true;
                }else{
                    Serial3.println(F("Signal, but not connected. Trying again.."));
                }
                
            }else{
                Serial3.println(F("No Signal, not connected. Trying again.."));
            } 
        }else{
            if (isNetworkConnected()){
                return true;
            }else{
                Serial3.println(F("Not connected. Trying again.."));
            }
                  
        }
    }
    /*Here we have now the option where we can switch between LTE-M an NB-IoT, see Modem AT manual LR8.2*/
    if(changeOPMode){
        if(!isNetworkConnected()){
            if(changeOparatingMode()){
                if (isNetworkConnected()){
                    return true;
                }else{
                    return false;
                }
            }else{
                return false;
            }
        }else{
            return true;
        }
        
    }

    //If timed out return default false
    return false;
    
}

bool SequansModem::isNetworkConnected(){
    //Now active cke of network connection status with the CEREG command
    char resbuf[50] = "";
    char res[48] = "";
    if(SequansController.writeCommand("AT+CEREG?")!=ResponseResult::OK){
        Serial3.println(F("Command (CEREG) response NOT OK"));
        return false;
    }else{

        if(!SequansController.waitForURC(CEREG_CALLBACK,resbuf,sizeof(resbuf),600000U,wdtReset,1000U)){
            return false;
        }else{
            Watchdog.reset();
            // Serial3.print(F("CEREG resbuf: "));
            // Serial3.println(resbuf);
            
            if(!SequansController.extractValueFromCommandResponse(resbuf, 1, res, sizeof(res), 0)){
                Serial3.println(F("CEREG response extract failed"));
                return false;
            }else{
                // Serial3.print(F("CEREG res: "));
                // Serial3.println(res);

                const char stat = res[0]; 
                //Serial3.print(F("CEREG stat: "));
                //Serial3.println(stat);

                if (stat == STAT_REGISTERED_HOME_NETWORK || stat == STAT_REGISTERED_ROAMING)
                {
                    //Serial3.println(F("Registered to Network"));
                    return true;
                }else{
                    return false;
                } 
            }
        }
    }
}

int16_t SequansModem::getSignalQuality(){
    char resbuf[24] = "";
    char res[12] = "";
    int16_t iRes = 0;
    //Send AT command for check Signal quality
    SequansController.writeCommand("AT+CSQ");
    //Wait for the CSQ URC response and save response after that
    SequansController.waitForURC("CSQ", resbuf, sizeof(resbuf), 10000U, wdtReset, 1000U);
    //Extract first value
    SequansController.extractValueFromCommandResponse(resbuf,0,res,sizeof(res),0);
    Serial3.print(F("CSQ res: "));
    Serial3.println(res);

    //Convert char array in to int and terurn it
    
    sscanf(res, "%d", &iRes);

    return iRes;
}

String SequansModem::getOperator(){
    // char res[30];
    // char resbuf[50];
    // SequansController.writeCommand("AT+COPS?",resbuf, sizeof(resbuf));
    // SequansController.extractValueFromCommandResponse(resbuf,2,res,sizeof(res));
    //Use function from LteClass
    String currentOperator = Lte.getOperator();

    return currentOperator;
}

bool SequansModem::getNetworkTime(int* year, int* month, int* day, int* hour,
                                    int* minute, int* second, float* timezone){
    uint16_t iyear = {0};
    uint16_t imonth = {0};
    uint16_t iday = {0};
    uint16_t ihour = {0};
    uint16_t iminute = {0};
    uint16_t isecond = {0};
    uint16_t itimeZone = {0};
    char sgn;
    uint32_t date[6] = {0U, 0U, 0U, 0U, 0U, 0U};           //date has the six values of year, month, day, hour, min, sec
    
    char responseBuffer[48] = ""; //= {"+CCLK: \"19/04/26,17:14:55+32\" OK"};
    char buffer[2] = "";
    String bufferString = "";

    /*Clear buffer from other AT command responses*/
    SequansController.clearReceiveBuffer();
    
    if(SequansController.writeCommand("AT+CCLK?")!=ResponseResult::OK){
        return false;
    }
    if(!SequansController.waitForURC("CCLK", responseBuffer, sizeof(responseBuffer), 7000U, wdtReset, 1000U)){
        return false;
    }
    /*responseBuffer is the received command from modem in form +CCLK: "yy/MM/dd,hh:mm:ss zz"*/
    //Cut of received String till \"
    // Do the replaces twice so we cover both \r and \r\n type endings
    bufferString = responseBuffer;
    // Serial3.print(F("bufferstring: "));
    // Serial3.println(bufferString);

    bufferString.replace(" \"","");
    bufferString.replace("\"","");
    
    //Put cutted String back in to responseBuffer
    bufferString.toCharArray(responseBuffer,sizeof(responseBuffer)/sizeof(responseBuffer[0]));

    // Serial3.println(F("responseBuffer: "));
    // Serial3.println(responseBuffer);

    //counter i is the position for responseBuffer array
    size_t i = {0};
    /*Put the number from response in to the date array*/
    for (size_t j = {0}; j < sizeof(date)/sizeof(date[0]) ; j++)
    {
        /*buffer is for the two numbers of the date sequence*/
        for (size_t k = 0; k < sizeof(buffer)/sizeof(buffer[0]); k++)
        {
            buffer[k] = responseBuffer[i];
            // Serial3.print(F("buffer[k]"));
            // Serial3.println(buffer[k]);
            i++;
        }
        /*Convert char array buffer to an integer value and put it in date array*/
        sscanf(buffer,"%lu", &date[j]);
        if (j<5)
        {
            //To skip the '/' ',' and ':' characters in responseBuffer
            i++;
        }
    }
    
    //Set date and time from date array
    iyear = date[0];
    // Serial3.print(F("iyear:") );
    // Serial3.println(iyear);
    imonth = date[1];
    // Serial3.print(F("imonth:") );
    // Serial3.println(imonth);
    iday = date[2];
    // Serial3.print(F("iday:") );
    // Serial3.println(iday);
    ihour = date[3];
    // Serial3.print(F("ihour:") );
    // Serial3.println(ihour);
    iminute = date[4];
    // Serial3.print(F("iminute:") );
    // Serial3.println(iminute);
    isecond = date[5];
    // Serial3.print(F("isecond:") );
    // Serial3.println(isecond);
    //Set sgn of zz a.k.a. timezone
    sgn = responseBuffer[i];
    // Serial3.print(F("sgn:") );
    // Serial3.println(sgn);
    //Set timezone
    buffer[0] = responseBuffer[i+1];
    buffer[1] = responseBuffer[i+2];
    /*Convert char array buffer to an integer value and put it in itimeZone*/
    sscanf(buffer,"%u",&itimeZone);
    //To get a hole hour
    itimeZone = itimeZone/4;

    //Set correct year and adjust hour to GMT time
    if (iyear<2000)
    {
        iyear += 2000;
    }
    
    // Set pointers
    *year = iyear;
    *month = imonth;
    *day = iday;
    *hour = ihour;
    *minute = iminute;
    *second = isecond;
    //*timezone = itimeZone*(-1.0);
    if (sgn == '+')
    {
        *timezone = (float)itimeZone;
         
    }else if (sgn == '-')
    {
        
        *timezone = itimeZone*(-1.0);
    }else
    {
        /*Something went wrong and the buffer did not have the format "yy/MM/dd,hh:mm:ss zz"*/
        return false;
    }
    
    return true;
    
}

#endif