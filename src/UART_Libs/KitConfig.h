/**
 * @file KitConfig.h
 * @brief Konfigurationsdatei für UART-Empfänger Projekt
 * @author Basierend auf SMART_WI_Libs
 * @date 2025-07-02
 * 
 * Diese Datei definiert alle wichtigen Konstanten für den UART-Empfänger
 */

#ifndef KitConfig_h
#define KitConfig_h

// Hardware-Plattform Definition
#define DXCORE
#define SMART_WI_1_1

// UART-Konfiguration
const uint32_t SERIAL_MON_BAUDRATE = {115200};  // Baudrate für Serial Monitor (USB)
const uint32_t UART2_BAUDRATE = {115200};         // Baudrate für UART2 (externes Gerät)

// Pin-Definitionen für UART2 (aus HAL.h übernommen)
#define SerialPIN_TX PIN_PF4    // PIN für UART2 TX
#define SerialPIN_RX PIN_PF5    // PIN für UART2 RX

// Timing-Konfiguration
#define DATA_TIMEOUT_MS 5000      // Timeout in ms für "keine Daten" Meldung
#define STATUS_UPDATE_MS 10000    // Status-Update Intervall in ms (10 Sekunden)
#define BUFFER_SIZE 1024          // Größe des Empfangspuffers

// Watchdog-Konfiguration (falls verwendet)
const uint16_t WATCHDOG_TIMEOUT = {16000};      // Watchdog timeout in ms

#endif
