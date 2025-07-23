/*Watchdog header for AVR-IoT-Cellular in order to use Watchdog class in smart-products-firemware.ino
Cretaed by Csaba Freiberger 01.2023*/
#ifndef watchdogAVR_h
#define watchdogAVR_h

#if defined (DXCORE)

class WatchdogAVRClass
{
private:
    int _maxTimePeriod;
    WatchdogAVRClass();
    // ~WatchdogAVRClass();
public:
    static WatchdogAVRClass& instance(void){
        static WatchdogAVRClass instance;
        return instance;
    }
    //WatchdogAVRClass();
    void enable(int maxTimePeriod);
    void reset();
    void disable();
};

extern WatchdogAVRClass Watchdog;

#endif
#endif