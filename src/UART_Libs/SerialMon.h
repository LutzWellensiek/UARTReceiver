#ifndef SerialMon_h
#define SerialMon_h

#include <Arduino.h>

// Set serial for debug console (to the Serial Monitor, default speed 115200)
#if defined (__AVR__)
    #define SerialMon Serial3
#elif defined (__SAMD21G18A__)
    #define SerialMon Serial
#endif

#endif
