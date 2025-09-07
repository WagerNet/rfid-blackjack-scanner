#include "RFIDReader.h"

RFIDReader::RFIDReader(uint8_t ssPin, uint8_t rstPin, unsigned long debounceMs) 
    : mfrc522(ssPin, rstPin), lastUID(0), lastScanTime(0), debounceDelay(debounceMs) {
}

bool RFIDReader::begin() {
    mfrc522.PCD_Init();
    return isReaderConnected();
}

bool RFIDReader::isReaderConnected() {
    byte version = mfrc522.PCD_ReadRegister(MFRC522::VersionReg);
    return (version != 0x00 && version != 0xFF);
}

bool RFIDReader::isCardPresent() {
    return mfrc522.PICC_IsNewCardPresent();
}

bool RFIDReader::readCard() {
    if (!isCardPresent()) {
        return false;
    }
    
    if (!mfrc522.PICC_ReadCardSerial()) {
        return false;
    }
    
    return true;
}

uint32_t RFIDReader::getUID() {
    uint32_t uid = 0;
    for (byte i = 0; i < mfrc522.uid.size; i++) {
        uid = (uid << 8) | mfrc522.uid.uidByte[i];
    }
    return uid;
}

String RFIDReader::getUIDString() {
    String uidStr = "";
    for (byte i = 0; i < mfrc522.uid.size; i++) {
        if (mfrc522.uid.uidByte[i] < 0x10) {
            uidStr += "0";
        }
        uidStr += String(mfrc522.uid.uidByte[i], HEX);
        if (i < mfrc522.uid.size - 1) {
            uidStr += ":";
        }
    }
    uidStr.toUpperCase();
    return uidStr;
}

void RFIDReader::printUID() {
    Serial.print("UID: ");
    for (byte i = 0; i < mfrc522.uid.size; i++) {
        Serial.print(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " ");
        Serial.print(mfrc522.uid.uidByte[i], HEX);
    }
    Serial.print(" (0x");
    Serial.print(getUID(), HEX);
    Serial.println(")");
}

void RFIDReader::printCardInfo() {
    Serial.println("=== Card Information ===");
    
    // Print UID
    printUID();
    
    // Print UID as string
    Serial.print("UID String: ");
    Serial.println(getUIDString());
    
    // Print card type
    MFRC522::PICC_Type piccType = mfrc522.PICC_GetType(mfrc522.uid.sak);
    Serial.print("Card Type: ");
    Serial.print(mfrc522.PICC_GetTypeName(piccType));
    Serial.print(" (SAK: 0x");
    Serial.print(mfrc522.uid.sak, HEX);
    Serial.println(")");
    
    // Print size
    Serial.print("UID Size: ");
    Serial.print(mfrc522.uid.size);
    Serial.println(" bytes");
    
    Serial.println("========================");
}

void RFIDReader::haltCard() {
    mfrc522.PICC_HaltA();
    mfrc522.PCD_StopCrypto1();
}

bool RFIDReader::isNewCard() {
    if (!readCard()) {
        return false;
    }
    
    uint32_t currentUID = getUID();
    unsigned long currentTime = millis();
    
    // Check debounce timing
    if (currentTime - lastScanTime < debounceDelay) {
        haltCard();
        return false;
    }
    
    // Check if same card as last scan
    if (currentUID == lastUID && currentTime - lastScanTime < debounceDelay * 2) {
        haltCard();
        return false;
    }
    
    // Update tracking variables
    lastUID = currentUID;
    lastScanTime = currentTime;
    
    return true;
}

void RFIDReader::setDebounceDelay(unsigned long ms) {
    debounceDelay = ms;
}
