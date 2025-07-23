#if defined (DXCORE)
#include "watchdogAVR.h"
#include <avr/wdt.h>
#include <util/atomic.h>
//#include "Arduino.h"

WatchdogAVRClass::WatchdogAVRClass(){
    _maxTimePeriod = 0;
}

WatchdogAVRClass Watchdog = WatchdogAVRClass::instance();

// WatchdogAVRClass::~WatchdogAVRClass(){}

void WatchdogAVRClass::enable(int maxTimePeriod){
        //wdt_enable(WDT_PERIOD_8KCLK_gc);
        if (maxTimePeriod<=8)
        {
            _maxTimePeriod = WDT_PERIOD_8CLK_gc;    /* 8 cycles (8ms) */
        }else if(maxTimePeriod<=16)
        {
            _maxTimePeriod = WDT_PERIOD_16CLK_gc;   /* 16 cycles (16ms) */ 
        }else if (maxTimePeriod<=32)
        {
            _maxTimePeriod = WDT_PERIOD_32CLK_gc;   /* 32 cycles (32ms) */
        }else if (maxTimePeriod<=64)
        {
            _maxTimePeriod = WDT_PERIOD_64CLK_gc;   /* 64 cycles (64ms) */
        }else if (maxTimePeriod<=128)
        {
            _maxTimePeriod = WDT_PERIOD_128CLK_gc;  /* 128 cycles (0.128s) */
        }else if (maxTimePeriod<=256)
        {
            _maxTimePeriod = WDT_PERIOD_256CLK_gc;  /* 256 cycles (0.256s) */
        }else if (maxTimePeriod<=512)
        {
            _maxTimePeriod = WDT_PERIOD_512CLK_gc;  /* 512 cycles (0.512s) */
        }else if (maxTimePeriod<=1000)
        {
            _maxTimePeriod = WDT_PERIOD_1KCLK_gc;   /* 1K cycles (1.0s) */
        }else if (maxTimePeriod<=2000)
        {
            _maxTimePeriod = WDT_PERIOD_2KCLK_gc;   /* 2K cycles (2.0s) */
        }else if (maxTimePeriod<=4000)
        {
            _maxTimePeriod = WDT_PERIOD_4KCLK_gc;   /* 4K cycles (4.1s) */
        }else if (maxTimePeriod<=8000)
        {
            _maxTimePeriod = WDT_PERIOD_8KCLK_gc;   /* 8K cycles (8.2s) */
        }else if (maxTimePeriod>8000)
        {
            _maxTimePeriod = WDT_PERIOD_8KCLK_gc;    /*The maximum time period is 8.2s!*/
        }else
        {
            _maxTimePeriod = WDT_PERIOD_OFF_gc;     /* Off */
        }

        wdt_enable(_maxTimePeriod);
}

void WatchdogAVRClass::reset(){
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE){
        wdt_reset();
    }
    
    //Serial3.println(F("wdt_reset"));
}

void WatchdogAVRClass::disable(){
    wdt_disable();
}

#endif