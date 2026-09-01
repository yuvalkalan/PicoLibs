#pragma once

#include "pico/stdlib.h"
#include "hardware/spi.h"

class E01C2G4M27D {
public:
    // Constructor maps the hardware SPI instance and GPIO pins
    E01C2G4M27D(spi_inst_t* spi_port,  uint pin_ce, uint pin_csn, uint pin_sck, uint pin_mosi, uint pin_miso, uint pin_irq );

    void begin();
    
    // Configuration
    void setChannel(uint8_t channel);       // 0-125
    void setPALevel(uint8_t level);         // 0: Min, 3: Max
    void setDataRate(uint8_t rate);         // 0: 1Mbps, 1: 2Mbps, 2: 250kbps
    
    // Pipe addressing
    void openWritingPipe(const uint8_t* address);
    void openReadingPipe(uint8_t number, const uint8_t* address);
    
    // Operation
    void startListening();
    void stopListening();
    
    // Data transfer
    bool write(const void* buf, uint8_t len);
    bool available();
    void read(void* buf, uint8_t len);

    // Quality information
    uint8_t getObserveTx(); // Returns Reg 0x08 (PLOS_CNT and ARC_CNT)
    bool getRPD();          // Returns Reg 0x09 bit 0 (Received Power Detector)

private:
    spi_inst_t* spi;
    uint pin_miso, pin_csn, pin_sck, pin_mosi, pin_ce, pin_irq;
    uint8_t payload_size;

    // GPIO helpers
    void csnLow();
    void csnHigh();
    void ceLow();
    void ceHigh();

    // SPI Operations
    uint8_t readRegister(uint8_t reg);
    void writeRegister(uint8_t reg, uint8_t value);
    void readRegisterMulti(uint8_t reg, uint8_t* buf, uint8_t len);
    void writeRegisterMulti(uint8_t reg, const uint8_t* buf, uint8_t len);
    uint8_t spiTransfer(uint8_t data);
    
    // Module controls
    void flushTx();
    void flushRx();
    uint8_t getStatus();
};