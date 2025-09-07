#ifndef RFIDREADER_H
#define RFIDREADER_H

#include <SPI.h>
#include <MFRC522.h>

class RFIDReader {
private:
    MFRC522 mfrc522;
    uint32_t lastUID;
    unsigned long lastScanTime;
    unsigned long debounceDelay;
    
public:
    RFIDReader(uint8_t ssPin, uint8_t rstPin, unsigned long debounceMs = 1000);
    
    bool begin();
    bool isReaderConnected();
    
    bool isCardPresent();
    bool readCard();
    uint32_t getUID();
    String getUIDString();
    
    void printUID();
    void printCardInfo();
    void haltCard();
    
    bool isNewCard();
    void setDebounceDelay(unsigned long ms);
};

#endif