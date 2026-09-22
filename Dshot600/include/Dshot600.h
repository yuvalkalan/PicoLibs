#pragma once

#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "hardware/dma.h"
/*
 * DShot600 is a class that implements the DShot600 protocol for controlling ESCs (Electronic Speed Controllers) using the Raspberry Pi Pico's PIO (Programmable Input/Output) and DMA (Direct Memory Access) features.
 *
 * The DShot600 protocol is a digital communication protocol used to control brushless motors in drones and other applications. It provides precise control over motor speed and direction, as well as additional features like telemetry and special commands.
 *
 * The DShot600 class allows you to initialize the PIO, configure the state machine, set throttle values, and start/stop the continuous DMA transfer for sending DShot600 signals to the ESCs.
 *
 * The class also defines an enumeration of special commands that can be sent to the ESCs, such as beeping, requesting telemetry data, changing motor spin direction, enabling/disabling 3D mode, and more.
 *
 * Usage:
 * 1. Create an instance of the DShot600 class by specifying the PIO instance, state machine index, GPIO pin connected to the ESC signal, and DMA channel to use.
 * 2. Call the init() method to initialize the PIO and DMA.
 * 3. Use set_throttle() to set the desired throttle value (0-2047) or send special commands (1-47).
 * 4. Call start() to begin continuous DMA transfer of DShot600 signals to the ESCs.
 * 5. Call stop() to stop the DMA transfer and reset the PIO when needed.
 *
 * Note: For best memory usage, use the same PIO block for all DShot600 instances.
 * Note: The DShot600 class is designed for use with Raspberry Pi Pico or compatible boards that support PIO and DMA features.
 */
class DShot600
{

public:
    /*
        Value	Command Name	                        Description
        1-5	    BEEP1 to BEEP5	                        Triggers different audio beep sequences using the motors. Very useful for locating a crashed drone.
        6	    ESC_INFO	                            Requests the ESC to send its firmware version and serial number over the telemetry wire.
        7	    SPIN_DIRECTION_1	                    Temporarily sets motor direction to reversed (used for "Turtle Mode" or crash flip).
        8	    SPIN_DIRECTION_2	                    Temporarily sets motor direction to normal.
        9	    3D_MODE_OFF	                            Disables 3D mode (standard flight).
        10	    3D_MODE_ON	                            Enables 3D mode (center throttle is off, up is positive thrust, down is negative thrust).
        11	    SETTINGS_REQUEST	                    Requests the ESC to send its current configuration settings.
        12	    SAVE_SETTINGS	                        Saves the currently active settings to the ESC's EEPROM so they persist after a power cycle.
        13-14	EXTENDED_TELEMETRY_ENABLE / DISABLE	    Toggles Extended DShot Telemetry (EDT) features.
        15-19	- Currently unassigned.
        20-21	SPIN_DIRECTION_NORMAL / REVERSED	    Sets the permanent motor spin direction (usually followed by the SAVE_SETTINGS command).
        22-25	LED0_ON to LED3_ON	                    Turns on specific onboard ESC LEDs (if the ESC hardware includes them).
        26-29	LED0_OFF to LED3_OFF	                Turns off the specific onboard ESC LEDs.
        30-31	AUDIO_STREAM / SILENT_MODE	            Audio toggles (rarely implemented on standard ESCs).
        32-33	SIGNAL_LINE_TELEMETRY_DISABLE / ENABLE	Turns Bidirectional DShot telemetry over the signal wire on or off.
        34-35	CONTINUOUS_ERPM_TELEMETRY	            Tells the ESC to constantly stream eRPM (electrical RPM) data back over the signal line on every normal throttle frame.
        36-41	- Currently unassigned.
        42  	TEMPERATURE_TELEMETRY	                Requests a single temperature reading (1°C per step).
        43	    VOLTAGE_TELEMETRY	                    Requests a single voltage reading (10mV per step).
        44	    CURRENT_TELEMETRY	                    Requests a single current draw reading (100mA per step).
        45	    CONSUMPTION_TELEMETRY	                Requests total mAh consumed since power-up.
        46-47	ERPM_TELEMETRY	                        Requests a single eRPM or eRPM period reading.
    */

    /*
     * DShot Special Commands (Values 1 - 47)
     * Note: Critical commands must be sent at least 6 times consecutively
     * to be accepted by the ESC. The telemetry request bit (bit 12) must
     * also be set to 1.
     */

    enum Cmd
    {
        MOTOR_STOP = 0,
        BEEP1 = 1,
        BEEP2 = 2,
        BEEP3 = 3,
        BEEP4 = 4,
        BEEP5 = 5,
        ESC_INFO = 6,
        SPIN_DIRECTION_1 = 7,
        SPIN_DIRECTION_2 = 8,
        MODE_3D_OFF = 9,
        MODE_3D_ON = 10,
        SETTINGS_REQUEST = 11,
        SAVE_SETTINGS = 12,
        EXTENDED_TELEMETRY_ENABLE = 13,
        EXTENDED_TELEMETRY_DISABLE = 14,
        SPIN_DIRECTION_NORMAL = 20,
        SPIN_DIRECTION_REVERSED = 21,
        LED0_ON = 22,
        LED1_ON = 23,
        LED2_ON = 24,
        LED3_ON = 25,
        LED0_OFF = 26,
        LED1_OFF = 27,
        LED2_OFF = 28,
        LED3_OFF = 29,
        AUDIO_STREAM = 30,
        SILENT_MODE = 31,
        SIGNAL_LINE_TELEMETRY_DISABLE = 32,
        SIGNAL_LINE_TELEMETRY_ENABLE = 33,
        CONTINUOUS_ERPM_TELEMETRY_ON = 34,
        CONTINUOUS_ERPM_TELEMETRY_OFF = 35,
        TEMPERATURE_TELEMETRY = 42,
        VOLTAGE_TELEMETRY = 43,
        CURRENT_TELEMETRY = 44,
        CONSUMPTION_TELEMETRY = 45,
        ERPM_TELEMETRY = 46,
        ERPM_PERIOD_TELEMETRY = 47,
        MIN_THROTTLE = 48,
        MAX_THROTTLE = 2047,
        REPEAT_COUNT = 6
    };

public:
    /**
     * @param pio PIO instance (pio0 or pio1)
     * @param sm State Machine index (0-3)
     * @param pin GPIO pin connected to ESC signal
     * @param dma_chan DMA channel to use
     */
    DShot600(PIO pio, uint sm, uint pin, int dma_chan = -1); // use -1 to auto-claim a DMA channel

    // Initializes PIO, DMA, and GPIO
    void init();

    // Starts the continuous DMA transfer
    void start();

    // Stops the DMA transfer and resets PIO
    void stop();

    /**
     * Sets the target throttle value. Updates in memory instantly;
     * the DMA automatically picks it up on the next frame.
     * @param throttle 0 = Disarmed. 48-2047 = Throttle (0-100%).
     *                 1-47 = Special commands (3D mode, save settings, etc).
     * @param telemetry Set true to request telemetry back from the ESC.
     */
    void set_throttle(uint16_t throttle, bool telemetry = false);

private:
    uint16_t encode_packet(uint16_t value, bool telemetry);

    PIO m_pio;
    uint m_sm;
    uint m_pin;
    uint m_dma_chan;

    // Aligned to 4 bytes for the DMA Ring Wrap feature to work over a single word
    alignas(4) uint32_t m_dma_buffer;
};