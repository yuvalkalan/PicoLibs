#pragma once

/*----------------------[cc1101 - misc]---------------------------------------*/
#define CC1101_CRYSTAL_FREQUENCY 26000000
#define CC1101_CFG_REGISTER 0x2F       // 47 registers
#define CC1101_FIFOBUFFER 0x42         // size of Fifo Buffer +2 for rssi and lqi
#define CC1101_RSSI_OFFSET_868MHZ 0x4E // dec = 74
#define CC1101_TX_RETRIES_MAX 0x05     // tx_retries_max
#define CC1101_ACK_TIMEOUT 50          // ACK timeout in ms
#define CC1101_COMPARE_REGISTER 0x00   // register compare 0=no compare 1=compare
#define CC1101_BROADCAST_ADDRESS 0x00  // broadcast address
#define CC1101_FREQ_315MHZ 0x01
#define CC1101_FREQ_434MHZ 0x02
#define CC1101_FREQ_868MHZ 0x03
#define CC1101_FREQ_915MHZ 0x04
#define CC1101_TEMP_ADC_MV 3.225 // 3.3V/1023 . mV pro digit
#define CC1101_TEMP_CELS_CO 2.47 // Temperature coefficient 2.47mV per Grad Celsius

/*---------------------------[cc1101 - R/W offsets]---------------------------*/
#define CC1101_WRITE_SINGLE_BYTE 0x00
#define CC1101_WRITE_BURST 0x40
#define CC1101_READ_SINGLE_BYTE 0x80
#define CC1101_READ_BURST 0xC0
/*---------------------------[END R/W offsets]--------------------------------*/

/*------------------------[cc1101 - FIFO commands]----------------------------*/
#define CC1101_TXFIFO_BURST 0x7F        // write burst only
#define CC1101_TXFIFO_SINGLE_BYTE 0x3F  // write single only
#define CC1101_RXFIFO_BURST 0xFF        // read burst only
#define CC1101_RXFIFO_SINGLE_BYTE 0xBF  // read single only
#define CC1101_PATABLE_BURST 0x7E       // power control read/write
#define CC1101_PATABLE_SINGLE_BYTE 0xFE // power control read/write
/*---------------------------[END FIFO commands]------------------------------*/

/*----------------------[cc1101 - config register]----------------------------*/
#define CC1101_IOCFG2 0x00   // GDO2 output pin configuration
#define CC1101_IOCFG1 0x01   // GDO1 output pin configuration
#define CC1101_IOCFG0 0x02   // GDO0 output pin configuration
#define CC1101_FIFOTHR 0x03  // RX FIFO and TX FIFO thresholds
#define CC1101_SYNC1 0x04    // Sync word, high byte
#define CC1101_SYNC0 0x05    // Sync word, low byte
#define CC1101_PKTLEN 0x06   // Packet length
#define CC1101_PKTCTRL1 0x07 // Packet automation control
#define CC1101_PKTCTRL0 0x08 // Packet automation control
#define CC1101_ADDR 0x09     // Device address
#define CC1101_CHANNR 0x0A   // Channel number
#define CC1101_FSCTRL1 0x0B  // Frequency synthesizer control
#define CC1101_FSCTRL0 0x0C  // Frequency synthesizer control
#define CC1101_FREQ2 0x0D    // Frequency control word, high byte
#define CC1101_FREQ1 0x0E    // Frequency control word, middle byte
#define CC1101_FREQ0 0x0F    // Frequency control word, low byte
#define CC1101_MDMCFG4 0x10  // Modem configuration
#define CC1101_MDMCFG3 0x11  // Modem configuration
#define CC1101_MDMCFG2 0x12  // Modem configuration
#define CC1101_MDMCFG1 0x13  // Modem configuration
#define CC1101_MDMCFG0 0x14  // Modem configuration
#define CC1101_DEVIATN 0x15  // Modem deviation setting
#define CC1101_MCSM2 0x16    // Main Radio Cntrl State Machine config
#define CC1101_MCSM1 0x17    // Main Radio Cntrl State Machine config
#define CC1101_MCSM0 0x18    // Main Radio Cntrl State Machine config
#define CC1101_FOCCFG 0x19   // Frequency Offset Compensation config
#define CC1101_BSCFG 0x1A    // Bit Synchronization configuration
#define CC1101_AGCCTRL2 0x1B // AGC control
#define CC1101_AGCCTRL1 0x1C // AGC control
#define CC1101_AGCCTRL0 0x1D // AGC control
#define CC1101_WOREVT1 0x1E  // High byte Event 0 timeout
#define CC1101_WOREVT0 0x1F  // Low byte Event 0 timeout
#define CC1101_WORCTRL 0x20  // Wake On Radio control
#define CC1101_FREND1 0x21   // Front end RX configuration
#define CC1101_FREND0 0x22   // Front end TX configuration
#define CC1101_FSCAL3 0x23   // Frequency synthesizer calibration
#define CC1101_FSCAL2 0x24   // Frequency synthesizer calibration
#define CC1101_FSCAL1 0x25   // Frequency synthesizer calibration
#define CC1101_FSCAL0 0x26   // Frequency synthesizer calibration
#define CC1101_RCCTRL1 0x27  // RC oscillator configuration
#define CC1101_RCCTRL0 0x28  // RC oscillator configuration
#define CC1101_FSTEST 0x29   // Frequency synthesizer cal control
#define CC1101_PTEST 0x2A    // Production test
#define CC1101_AGCTEST 0x2B  // AGC test
#define CC1101_TEST2 0x2C    // Various test settings
#define CC1101_TEST1 0x2D    // Various test settings
#define CC1101_TEST0 0x2E    // Various test settings
/*-------------------------[END config register]------------------------------*/

/*------------------------[cc1101-command strobes]----------------------------*/
#define CC1101_SRES 0x30    // Reset chip
#define CC1101_SFSTXON 0x31 // Enable/calibrate freq synthesizer
#define CC1101_SXOFF 0x32   // Turn off crystal oscillator.
#define CC1101_SCAL 0x33    // Calibrate freq synthesizer & disable
#define CC1101_SRX 0x34     // Enable RX.
#define CC1101_STX 0x35     // Enable TX.
#define CC1101_SIDLE 0x36   // Exit RX / TX
#define CC1101_SAFC 0x37    // AFC adjustment of freq synthesizer
#define CC1101_SWOR 0x38    // Start automatic RX polling sequence
#define CC1101_SPWD 0x39    // Enter pwr down mode when CSn goes hi
#define CC1101_SFRX 0x3A    // Flush the RX FIFO buffer.
#define CC1101_SFTX 0x3B    // Flush the TX FIFO buffer.
#define CC1101_SWORRST 0x3C // Reset real time clock.
#define CC1101_SNOP 0x3D    // No operation.
/*-------------------------[END command strobes]------------------------------*/

/*----------------------[cc1101 - status register]----------------------------*/
#define CC1101_PARTNUM 0xF0        // Part number
#define CC1101_VERSION 0xF1        // Current version number
#define CC1101_FREQEST 0xF2        // Frequency offset estimate
#define CC1101_LQI 0xF3            // Demodulator estimate for link quality
#define CC1101_RSSI 0xF4           // Received signal strength indication
#define CC1101_MARCSTATE 0xF5      // Control state machine state
#define CC1101_WORTIME1 0xF6       // High byte of WOR timer
#define CC1101_WORTIME0 0xF7       // Low byte of WOR timer
#define CC1101_PKTSTATUS 0xF8      // Current GDOx status and packet status
#define CC1101_VCO_VC_DAC 0xF9     // Current setting from PLL cal module
#define CC1101_TXBYTES 0xFA        // Underflow and # of CC1101_bytes in TXFIFO
#define CC1101_RXBYTES 0xFB        // Overflow and # of CC1101_bytes in RXFIFO
#define CC1101_RCCTRL1_STATUS 0xFC // Last RC Oscillator Calibration Result
#define CC1101_RCCTRL0_STATUS 0xFD // Last RC Oscillator Calibration Result
//--------------------------[END status register]-------------------------------

//-------------------[global default settings 868 Mhz]---------------------------------
static uint8_t cc1101_GFSK_1_2_kb[CC1101_CFG_REGISTER] = {
    0x07, // IOCFG2        GDO2 Output Pin Configuration
    0x2E, // IOCFG1        GDO1 Output Pin Configuration
    0x80, // IOCFG0        GDO0 Output Pin Configuration
    0x07, // FIFOTHR       RX FIFO and TX FIFO Thresholds
    0x57, // SYNC1         Sync Word, High Byte
    0x43, // SYNC0         Sync Word, Low Byte
    0x3E, // PKTLEN        Packet Length
    0x0E, // PKTCTRL1      Packet Automation Control
    0x45, // PKTCTRL0      Packet Automation Control
    0xFF, // ADDR          Device Address
    0x00, // CHANNR        Channel Number
    0x08, // FSCTRL1       Frequency Synthesizer Control
    0x00, // FSCTRL0       Frequency Synthesizer Control
    0x21, // FREQ2         Frequency Control Word, High Byte
    0x65, // FREQ1         Frequency Control Word, Middle Byte
    0x6A, // FREQ0         Frequency Control Word, Low Byte
    0xF5, // MDMCFG4       Modem Configuration
    0x83, // MDMCFG3       Modem Configuration
    0x13, // MDMCFG2       Modem Configuration
    0xA0, // MDMCFG1       Modem Configuration
    0xF8, // MDMCFG0       Modem Configuration
    0x15, // DEVIATN       Modem Deviation Setting
    0x07, // MCSM2         Main Radio Control State Machine Configuration
    0x0C, // MCSM1         Main Radio Control State Machine Configuration
    0x18, // MCSM0         Main Radio Control State Machine Configuration
    0x16, // FOCCFG        Frequency Offset Compensation Configuration
    0x6C, // BSCFG         Bit Synchronization Configuration
    0x03, // AGCCTRL2      AGC Control
    0x40, // AGCCTRL1      AGC Control
    0x91, // AGCCTRL0      AGC Control
    0x02, // WOREVT1       High Byte Event0 Timeout
    0x26, // WOREVT0       Low Byte Event0 Timeout
    0x09, // WORCTRL       Wake On Radio Control
    0x56, // FREND1        Front End RX Configuration
    0x17, // FREND0        Front End TX Configuration
    0xA9, // FSCAL3        Frequency Synthesizer Calibration
    0x0A, // FSCAL2        Frequency Synthesizer Calibration
    0x00, // FSCAL1        Frequency Synthesizer Calibration
    0x11, // FSCAL0        Frequency Synthesizer Calibration
    0x41, // RCCTRL1       RC Oscillator Configuration
    0x00, // RCCTRL0       RC Oscillator Configuration
    0x59, // FSTEST        Frequency Synthesizer Calibration Control,
    0x7F, // PTEST         Production Test
    0x3F, // AGCTEST       AGC Test
    0x81, // TEST2         Various Test Settings
    0x3F, // TEST1         Various Test Settings
    0x0B  // TEST0         Various Test Settings
};

static uint8_t cc1101_GFSK_38_4_kb[CC1101_CFG_REGISTER] = {
    0x07, // IOCFG2        GDO2 Output Pin Configuration
    0x2E, // IOCFG1        GDO1 Output Pin Configuration
    0x80, // IOCFG0        GDO0 Output Pin Configuration
    0x07, // FIFOTHR       RX FIFO and TX FIFO Thresholds
    0x57, // SYNC1         Sync Word, High Byte
    0x43, // SYNC0         Sync Word, Low Byte
    0x3E, // PKTLEN        Packet Length
    0x0E, // PKTCTRL1      Packet Automation Control
    0x45, // PKTCTRL0      Packet Automation Control
    0xFF, // ADDR          Device Address
    0x00, // CHANNR        Channel Number
    0x06, // FSCTRL1       Frequency Synthesizer Control
    0x00, // FSCTRL0       Frequency Synthesizer Control
    0x21, // FREQ2         Frequency Control Word, High Byte
    0x65, // FREQ1         Frequency Control Word, Middle Byte
    0x6A, // FREQ0         Frequency Control Word, Low Byte
    0xCA, // MDMCFG4       Modem Configuration
    0x83, // MDMCFG3       Modem Configuration
    0x13, // MDMCFG2       Modem Configuration
    0xA0, // MDMCFG1       Modem Configuration
    0xF8, // MDMCFG0       Modem Configuration
    0x34, // DEVIATN       Modem Deviation Setting
    0x07, // MCSM2         Main Radio Control State Machine Configuration
    0x0C, // MCSM1         Main Radio Control State Machine Configuration
    0x18, // MCSM0         Main Radio Control State Machine Configuration
    0x16, // FOCCFG        Frequency Offset Compensation Configuration
    0x6C, // BSCFG         Bit Synchronization Configuration
    0x43, // AGCCTRL2      AGC Control
    0x40, // AGCCTRL1      AGC Control
    0x91, // AGCCTRL0      AGC Control
    0x02, // WOREVT1       High Byte Event0 Timeout
    0x26, // WOREVT0       Low Byte Event0 Timeout
    0x09, // WORCTRL       Wake On Radio Control
    0x56, // FREND1        Front End RX Configuration
    0x17, // FREND0        Front End TX Configuration
    0xA9, // FSCAL3        Frequency Synthesizer Calibration
    0x0A, // FSCAL2        Frequency Synthesizer Calibration
    0x00, // FSCAL1        Frequency Synthesizer Calibration
    0x11, // FSCAL0        Frequency Synthesizer Calibration
    0x41, // RCCTRL1       RC Oscillator Configuration
    0x00, // RCCTRL0       RC Oscillator Configuration
    0x59, // FSTEST        Frequency Synthesizer Calibration Control,
    0x7F, // PTEST         Production Test
    0x3F, // AGCTEST       AGC Test
    0x81, // TEST2         Various Test Settings
    0x3F, // TEST1         Various Test Settings
    0x0B  // TEST0         Various Test Settings
};

static uint8_t cc1101_GFSK_100_kb[CC1101_CFG_REGISTER] = {
    0x07, // IOCFG2        GDO2 Output Pin Configuration
    0x2E, // IOCFG1        GDO1 Output Pin Configuration
    0x80, // IOCFG0        GDO0 Output Pin Configuration
    0x07, // FIFOTHR       RX FIFO and TX FIFO Thresholds
    0x57, // SYNC1         Sync Word, High Byte
    0x43, // SYNC0         Sync Word, Low Byte
    0x3E, // PKTLEN        Packet Length
    0x0E, // PKTCTRL1      Packet Automation Control
    0x45, // PKTCTRL0      Packet Automation Control
    0xFF, // ADDR          Device Address
    0x00, // CHANNR        Channel Number
    0x08, // FSCTRL1       Frequency Synthesizer Control
    0x00, // FSCTRL0       Frequency Synthesizer Control
    0x21, // FREQ2         Frequency Control Word, High Byte
    0x65, // FREQ1         Frequency Control Word, Middle Byte
    0x6A, // FREQ0         Frequency Control Word, Low Byte
    0x5B, // MDMCFG4       Modem Configuration
    0xF8, // MDMCFG3       Modem Configuration
    0x13, // MDMCFG2       Modem Configuration
    0xA0, // MDMCFG1       Modem Configuration
    0xF8, // MDMCFG0       Modem Configuration
    0x47, // DEVIATN       Modem Deviation Setting
    0x07, // MCSM2         Main Radio Control State Machine Configuration
    0x0C, // MCSM1         Main Radio Control State Machine Configuration
    0x18, // MCSM0         Main Radio Control State Machine Configuration
    0x1D, // FOCCFG        Frequency Offset Compensation Configuration
    0x1C, // BSCFG         Bit Synchronization Configuration
    0xC7, // AGCCTRL2      AGC Control
    0x00, // AGCCTRL1      AGC Control
    0xB2, // AGCCTRL0      AGC Control
    0x02, // WOREVT1       High Byte Event0 Timeout
    0x26, // WOREVT0       Low Byte Event0 Timeout
    0x09, // WORCTRL       Wake On Radio Control
    0xB6, // FREND1        Front End RX Configuration
    0x17, // FREND0        Front End TX Configuration
    0xEA, // FSCAL3        Frequency Synthesizer Calibration
    0x0A, // FSCAL2        Frequency Synthesizer Calibration
    0x00, // FSCAL1        Frequency Synthesizer Calibration
    0x11, // FSCAL0        Frequency Synthesizer Calibration
    0x41, // RCCTRL1       RC Oscillator Configuration
    0x00, // RCCTRL0       RC Oscillator Configuration
    0x59, // FSTEST        Frequency Synthesizer Calibration Control,
    0x7F, // PTEST         Production Test
    0x3F, // AGCTEST       AGC Test
    0x81, // TEST2         Various Test Settings
    0x3F, // TEST1         Various Test Settings
    0x0B  // TEST0         Various Test Settings
};

static uint8_t cc1101_MSK_250_kb[CC1101_CFG_REGISTER] = {
    0x07, // IOCFG2        GDO2 Output Pin Configuration
    0x2E, // IOCFG1        GDO1 Output Pin Configuration
    0x80, // IOCFG0        GDO0 Output Pin Configuration
    0x07, // FIFOTHR       RX FIFO and TX FIFO Thresholds
    0x57, // SYNC1         Sync Word, High Byte
    0x43, // SYNC0         Sync Word, Low Byte
    0x3E, // PKTLEN        Packet Length
    0x0E, // PKTCTRL1      Packet Automation Control
    0x45, // PKTCTRL0      Packet Automation Control
    0xFF, // ADDR          Device Address
    0x00, // CHANNR        Channel Number
    0x0B, // FSCTRL1       Frequency Synthesizer Control
    0x00, // FSCTRL0       Frequency Synthesizer Control
    0x21, // FREQ2         Frequency Control Word, High Byte
    0x65, // FREQ1         Frequency Control Word, Middle Byte
    0x6A, // FREQ0         Frequency Control Word, Low Byte
    0x2D, // MDMCFG4       Modem Configuration
    0x3B, // MDMCFG3       Modem Configuration
    0x73, // MDMCFG2       Modem Configuration
    0xA0, // MDMCFG1       Modem Configuration
    0xF8, // MDMCFG0       Modem Configuration
    0x00, // DEVIATN       Modem Deviation Setting
    0x07, // MCSM2         Main Radio Control State Machine Configuration
    0x0C, // MCSM1         Main Radio Control State Machine Configuration
    0x18, // MCSM0         Main Radio Control State Machine Configuration
    0x1D, // FOCCFG        Frequency Offset Compensation Configuration
    0x1C, // BSCFG         Bit Synchronization Configuration
    0xC7, // AGCCTRL2      AGC Control
    0x00, // AGCCTRL1      AGC Control
    0xB2, // AGCCTRL0      AGC Control
    0x02, // WOREVT1       High Byte Event0 Timeout
    0x26, // WOREVT0       Low Byte Event0 Timeout
    0x09, // WORCTRL       Wake On Radio Control
    0xB6, // FREND1        Front End RX Configuration
    0x17, // FREND0        Front End TX Configuration
    0xEA, // FSCAL3        Frequency Synthesizer Calibration
    0x0A, // FSCAL2        Frequency Synthesizer Calibration
    0x00, // FSCAL1        Frequency Synthesizer Calibration
    0x11, // FSCAL0        Frequency Synthesizer Calibration
    0x41, // RCCTRL1       RC Oscillator Configuration
    0x00, // RCCTRL0       RC Oscillator Configuration
    0x59, // FSTEST        Frequency Synthesizer Calibration Control,
    0x7F, // PTEST         Production Test
    0x3F, // AGCTEST       AGC Test
    0x81, // TEST2         Various Test Settings
    0x3F, // TEST1         Various Test Settings
    0x0B  // TEST0         Various Test Settings
};

static uint8_t cc1101_MSK_500_kb[CC1101_CFG_REGISTER] = {
    0x07, // IOCFG2        GDO2 Output Pin Configuration
    0x2E, // IOCFG1        GDO1 Output Pin Configuration
    0x80, // IOCFG0        GDO0 Output Pin Configuration
    0x07, // FIFOTHR       RX FIFO and TX FIFO Thresholds
    0x57, // SYNC1         Sync Word, High Byte
    0x43, // SYNC0         Sync Word, Low Byte
    0x3E, // PKTLEN        Packet Length
    0x0E, // PKTCTRL1      Packet Automation Control
    0x45, // PKTCTRL0      Packet Automation Control
    0xFF, // ADDR          Device Address
    0x00, // CHANNR        Channel Number
    0x0C, // FSCTRL1       Frequency Synthesizer Control
    0x00, // FSCTRL0       Frequency Synthesizer Control
    0x21, // FREQ2         Frequency Control Word, High Byte
    0x65, // FREQ1         Frequency Control Word, Middle Byte
    0x6A, // FREQ0         Frequency Control Word, Low Byte
    0x0E, // MDMCFG4       Modem Configuration
    0x3B, // MDMCFG3       Modem Configuration
    0x73, // MDMCFG2       Modem Configuration
    0x42, // MDMCFG1       Modem Configuration
    0xF8, // MDMCFG0       Modem Configuration
    0x00, // DEVIATN       Modem Deviation Setting
    0x07, // MCSM2         Main Radio Control State Machine Configuration
    0x0C, // MCSM1         Main Radio Control State Machine Configuration
    0x18, // MCSM0         Main Radio Control State Machine Configuration
    0x1D, // FOCCFG        Frequency Offset Compensation Configuration
    0x1C, // BSCFG         Bit Synchronization Configuration
    0xC7, // AGCCTRL2      AGC Control
    0x40, // AGCCTRL1      AGC Control
    0xB2, // AGCCTRL0      AGC Control
    0x02, // WOREVT1       High Byte Event0 Timeout
    0x26, // WOREVT0       Low Byte Event0 Timeout
    0x09, // WORCTRL       Wake On Radio Control
    0xB6, // FREND1        Front End RX Configuration
    0x17, // FREND0        Front End TX Configuration
    0xEA, // FSCAL3        Frequency Synthesizer Calibration
    0x0A, // FSCAL2        Frequency Synthesizer Calibration
    0x00, // FSCAL1        Frequency Synthesizer Calibration
    0x19, // FSCAL0        Frequency Synthesizer Calibration
    0x41, // RCCTRL1       RC Oscillator Configuration
    0x00, // RCCTRL0       RC Oscillator Configuration
    0x59, // FSTEST        Frequency Synthesizer Calibration Control,
    0x7F, // PTEST         Production Test
    0x3F, // AGCTEST       AGC Test
    0x81, // TEST2         Various Test Settings
    0x3F, // TEST1         Various Test Settings
    0x0B  // TEST0         Various Test Settings
};

static uint8_t cc1101_OOK_4_8_kb[CC1101_CFG_REGISTER] = {
    0x06, // IOCFG2        GDO2 Output Pin Configuration
    0x2E, // IOCFG1        GDO1 Output Pin Configuration
    0x06, // IOCFG0        GDO0 Output Pin Configuration
    0x47, // FIFOTHR       RX FIFO and TX FIFO Thresholds
    0x57, // SYNC1         Sync Word, High Byte
    0x43, // SYNC0         Sync Word, Low Byte
    0xFF, // PKTLEN        Packet Length
    0x04, // PKTCTRL1      Packet Automation Control
    0x05, // PKTCTRL0      Packet Automation Control
    0x00, // ADDR          Device Address
    0x00, // CHANNR        Channel Number
    0x06, // FSCTRL1       Frequency Synthesizer Control
    0x00, // FSCTRL0       Frequency Synthesizer Control
    0x21, // FREQ2         Frequency Control Word, High Byte
    0x65, // FREQ1         Frequency Control Word, Middle Byte
    0x6A, // FREQ0         Frequency Control Word, Low Byte
    0x87, // MDMCFG4       Modem Configuration
    0x83, // MDMCFG3       Modem Configuration
    0x3B, // MDMCFG2       Modem Configuration
    0x22, // MDMCFG1       Modem Configuration
    0xF8, // MDMCFG0       Modem Configuration
    0x15, // DEVIATN       Modem Deviation Setting
    0x07, // MCSM2         Main Radio Control State Machine Configuration
    0x30, // MCSM1         Main Radio Control State Machine Configuration
    0x18, // MCSM0         Main Radio Control State Machine Configuration
    0x14, // FOCCFG        Frequency Offset Compensation Configuration
    0x6C, // BSCFG         Bit Synchronization Configuration
    0x07, // AGCCTRL2      AGC Control
    0x00, // AGCCTRL1      AGC Control
    0x92, // AGCCTRL0      AGC Control
    0x87, // WOREVT1       High Byte Event0 Timeout
    0x6B, // WOREVT0       Low Byte Event0 Timeout
    0xFB, // WORCTRL       Wake On Radio Control
    0x56, // FREND1        Front End RX Configuration
    0x17, // FREND0        Front End TX Configuration
    0xE9, // FSCAL3        Frequency Synthesizer Calibration
    0x2A, // FSCAL2        Frequency Synthesizer Calibration
    0x00, // FSCAL1        Frequency Synthesizer Calibration
    0x1F, // FSCAL0        Frequency Synthesizer Calibration
    0x41, // RCCTRL1       RC Oscillator Configuration
    0x00, // RCCTRL0       RC Oscillator Configuration
    0x59, // FSTEST        Frequency Synthesizer Calibration Control
    0x7F, // PTEST         Production Test
    0x3F, // AGCTEST       AGC Test
    0x81, // TEST2         Various Test Settings
    0x35, // TEST1         Various Test Settings
    0x09, // TEST0         Various Test Settings
};