#pragma once

/*
 * E01C2G4M27D (Si24R1 / nRF24L01+) Memory Map and SPI Commands
 */

// ============================================================================
// SPI COMMAND INSTRUCTIONS
// ============================================================================
#define E01C2G4M27D_CMD_R_REGISTER 0x00         // Read command and status registers
#define E01C2G4M27D_CMD_W_REGISTER 0x20         // Write command and status registers
#define E01C2G4M27D_CMD_R_RX_PAYLOAD 0x61       // Read RX payload
#define E01C2G4M27D_CMD_W_TX_PAYLOAD 0xA0       // Write TX payload
#define E01C2G4M27D_CMD_FLUSH_TX 0xE1           // Flush TX FIFO (used in TX mode)
#define E01C2G4M27D_CMD_FLUSH_RX 0xE2           // Flush RX FIFO (used in RX mode)
#define E01C2G4M27D_CMD_REUSE_TX_PL 0xE3        // Reuse last transmitted payload
#define E01C2G4M27D_CMD_R_RX_PL_WID 0x60        // Read RX payload width (for dynamic payloads)
#define E01C2G4M27D_CMD_W_ACK_PAYLOAD 0xA8      // Write Payload to be transmitted together with ACK
#define E01C2G4M27D_CMD_W_TX_PAYLOAD_NOACK 0xB0 // Disables AUTOACK on this specific packet
#define E01C2G4M27D_CMD_NOP 0xFF                // No Operation (Used to read STATUS register)

// ============================================================================
// REGISTER MAP (Addresses 0x00 - 0x1D)
// ============================================================================
#define E01C2G4M27D_REG_CONFIG 0x00      // Configuration Register
#define E01C2G4M27D_REG_EN_AA 0x01       // Enable "Auto Acknowledgment" Function
#define E01C2G4M27D_REG_EN_RXADDR 0x02   // Enabled RX Addresses
#define E01C2G4M27D_REG_SETUP_AW 0x03    // Setup of Address Widths (common for all data pipes)
#define E01C2G4M27D_REG_SETUP_RETR 0x04  // Setup of Automatic Retransmission
#define E01C2G4M27D_REG_RF_CH 0x05       // RF Channel
#define E01C2G4M27D_REG_RF_SETUP 0x06    // RF Setup Register
#define E01C2G4M27D_REG_STATUS 0x07      // Status Register
#define E01C2G4M27D_REG_OBSERVE_TX 0x08  // Transmit observe register
#define E01C2G4M27D_REG_RPD 0x09         // Received Power Detector (Carrier Detect)
#define E01C2G4M27D_REG_RX_ADDR_P0 0x0A  // Receive address data pipe 0 (5 Bytes)
#define E01C2G4M27D_REG_RX_ADDR_P1 0x0B  // Receive address data pipe 1 (5 Bytes)
#define E01C2G4M27D_REG_RX_ADDR_P2 0x0C  // Receive address data pipe 2 (1 Byte, MSBs from P1)
#define E01C2G4M27D_REG_RX_ADDR_P3 0x0D  // Receive address data pipe 3 (1 Byte, MSBs from P1)
#define E01C2G4M27D_REG_RX_ADDR_P4 0x0E  // Receive address data pipe 4 (1 Byte, MSBs from P1)
#define E01C2G4M27D_REG_RX_ADDR_P5 0x0F  // Receive address data pipe 5 (1 Byte, MSBs from P1)
#define E01C2G4M27D_REG_TX_ADDR 0x10     // Transmit address (5 Bytes)
#define E01C2G4M27D_REG_RX_PW_P0 0x11    // Number of bytes in RX payload in data pipe 0
#define E01C2G4M27D_REG_RX_PW_P1 0x12    // Number of bytes in RX payload in data pipe 1
#define E01C2G4M27D_REG_RX_PW_P2 0x13    // Number of bytes in RX payload in data pipe 2
#define E01C2G4M27D_REG_RX_PW_P3 0x14    // Number of bytes in RX payload in data pipe 3
#define E01C2G4M27D_REG_RX_PW_P4 0x15    // Number of bytes in RX payload in data pipe 4
#define E01C2G4M27D_REG_RX_PW_P5 0x16    // Number of bytes in RX payload in data pipe 5
#define E01C2G4M27D_REG_FIFO_STATUS 0x17 // FIFO Status Register
#define E01C2G4M27D_REG_DYNPD 0x1C       // Enable dynamic payload length
#define E01C2G4M27D_REG_FEATURE 0x1D     // Feature Register

// ============================================================================
// BIT MNEMONICS FOR KEY REGISTERS
// ============================================================================

// REG_CONFIG (0x00) bits
#define E01C2G4M27D_MASK_RX_DR 6  // Mask interrupt caused by RX_DR
#define E01C2G4M27D_MASK_TX_DS 5  // Mask interrupt caused by TX_DS
#define E01C2G4M27D_MASK_MAX_RT 4 // Mask interrupt caused by MAX_RT
#define E01C2G4M27D_EN_CRC 3      // Enable CRC
#define E01C2G4M27D_CRCO 2        // CRC encoding scheme (0 = 1 byte, 1 = 2 bytes)
#define E01C2G4M27D_PWR_UP 1      // Power up / power down
#define E01C2G4M27D_PRIM_RX 0     // RX/TX control (1 = PRX, 0 = PTX)

// REG_STATUS (0x07) bits
#define E01C2G4M27D_RX_DR 6   // Data Ready RX FIFO interrupt
#define E01C2G4M27D_TX_DS 5   // Data Sent TX FIFO interrupt
#define E01C2G4M27D_MAX_RT 4  // Maximum number of TX retries interrupt
#define E01C2G4M27D_RX_P_NO 1 // Data pipe number for the payload available (3 bits: 1-3)
#define E01C2G4M27D_TX_FULL 0 // TX FIFO full flag

// REG_RF_SETUP (0x06) bits
#define E01C2G4M27D_CONT_WAVE 7  // Enables continuous carrier transmit
#define E01C2G4M27D_RF_DR_LOW 5  // Set RF Data Rate to 250kbps (bit 5)
#define E01C2G4M27D_PLL_LOCK 4   // Force PLL lock signal (test only)
#define E01C2G4M27D_RF_DR_HIGH 3 // Selects 2Mbps (bit 3)
#define E01C2G4M27D_RF_PWR 1     // RF output power setup (2 bits: 1-2)

// REG_FIFO_STATUS (0x17) bits
#define E01C2G4M27D_TX_REUSE 6      // Reusing last transmitted payload flag
#define E01C2G4M27D_FIFO_TX_FULL 5  // TX FIFO full flag
#define E01C2G4M27D_FIFO_TX_EMPTY 4 // TX FIFO empty flag
#define E01C2G4M27D_FIFO_RX_FULL 1  // RX FIFO full flag
#define E01C2G4M27D_FIFO_RX_EMPTY 0 // RX FIFO empty flag

// REG_FEATURE (0x1D) bits
#define E01C2G4M27D_EN_DPL 2     // Enables Dynamic Payload Length
#define E01C2G4M27D_EN_ACK_PAY 1 // Enables Payload with ACK
#define E01C2G4M27D_EN_DYN_ACK 0 // Enables the W_TX_PAYLOAD_NOACK command

// ============================================================================
// my configurations
// ============================================================================

#define RETRANSMIT_RETRIES 15
#define RETRANSMIT_DELAY_US 250                        // select a number dividing by 250
#define RETRANSMIT_DELAY RETRANSMIT_DELAY_US / 250 - 1 // dont change this line - change RETRANSMIT_DELAY_US instead
