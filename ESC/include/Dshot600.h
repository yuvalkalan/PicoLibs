#pragma once

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

#define DSHOT_CMD_MOTOR_STOP 0

#define DSHOT_CMD_BEEP1 1
#define DSHOT_CMD_BEEP2 2
#define DSHOT_CMD_BEEP3 3
#define DSHOT_CMD_BEEP4 4
#define DSHOT_CMD_BEEP5 5
#define DSHOT_CMD_ESC_INFO 6
#define DSHOT_CMD_SPIN_DIRECTION_1 7
#define DSHOT_CMD_SPIN_DIRECTION_2 8
#define DSHOT_CMD_3D_MODE_OFF 9
#define DSHOT_CMD_3D_MODE_ON 10
#define DSHOT_CMD_SETTINGS_REQUEST 11
#define DSHOT_CMD_SAVE_SETTINGS 12
#define DSHOT_CMD_EXTENDED_TELEMETRY_ENABLE 13
#define DSHOT_CMD_EXTENDED_TELEMETRY_DISABLE 14

/* 15-19 Unassigned */

#define DSHOT_CMD_SPIN_DIRECTION_NORMAL 20
#define DSHOT_CMD_SPIN_DIRECTION_REVERSED 21
#define DSHOT_CMD_LED0_ON 22
#define DSHOT_CMD_LED1_ON 23
#define DSHOT_CMD_LED2_ON 24
#define DSHOT_CMD_LED3_ON 25
#define DSHOT_CMD_LED0_OFF 26
#define DSHOT_CMD_LED1_OFF 27
#define DSHOT_CMD_LED2_OFF 28
#define DSHOT_CMD_LED3_OFF 29
#define DSHOT_CMD_AUDIO_STREAM 30
#define DSHOT_CMD_SILENT_MODE 31
#define DSHOT_CMD_SIGNAL_LINE_TELEMETRY_DISABLE 32
#define DSHOT_CMD_SIGNAL_LINE_TELEMETRY_ENABLE 33
#define DSHOT_CMD_CONTINUOUS_ERPM_TELEMETRY_ON 34
#define DSHOT_CMD_CONTINUOUS_ERPM_TELEMETRY_OFF 35

/* 36-41 Unassigned */

#define DSHOT_CMD_TEMPERATURE_TELEMETRY 42
#define DSHOT_CMD_VOLTAGE_TELEMETRY 43
#define DSHOT_CMD_CURRENT_TELEMETRY 44
#define DSHOT_CMD_CONSUMPTION_TELEMETRY 45
#define DSHOT_CMD_ERPM_TELEMETRY 46
#define DSHOT_CMD_ERPM_PERIOD_TELEMETRY 47

/* Standard Throttle Range */
#define DSHOT_MIN_THROTTLE 48
#define DSHOT_MAX_THROTTLE 2047

/* The number of times a command must be repeated for the ESC to execute it */
#define DSHOT_CMD_REPEAT_COUNT 6