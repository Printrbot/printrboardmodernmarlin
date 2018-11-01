# printrboardmodernmarlin
Printrboard and Modern Marlin

	Marlin 1.1.9

		This release is configured for a Printrbot Play with stepper drivers inverted, more releases will come in the following days.
9 Point Auto Bilinear Bedleving
Linear Advance Version 1.5 Greatly Improves Print Quality, Resolution and Speed
Babystepping allows live z offset during the start of the print

	This isn't your grandmas 3D printer so please make sure your inductive probe is adjusted properly

Cold End Fan not tested E0_Auto_Fan mapped to Pin 22 - will get to this asap

	Machine Definition can be updated using gCode
    
    Current Steps M92 X80 Y80 Z2020 E94.5
	

	Linear Advance is enabled by default retraction distance should be limited to 0.7mm or less
K value default 0.08
M900 K0.08
Please calibrate your K-factor and submit results 
http://marlinfw.org/tools/lin_advance/k-factor.html

    Babystepping
M290 and LCD menu items to move the axes by tiny increments without changing the current position values. 
This feature is used primarily to adjust the Z axis in the first layer of a print in real-time. Warning: Does not respect endstops!
This replaces the need to set the Z offset using M212
babystepping can also be adjusted using by joging the z axis while printing through octo print control pannel or via usb on your computer.

Zoffset can also be set using M815 but is not completely nessary
M815 Z-1.95 ;this is the default value

Use M500 to save Bed Probing


    LCD semi-tested 
    Custom User Menu Scripts
    Enabled LCD Support for Ultipanel and Ultra_lcd
    SD Card Support on the LCD is dissabled as of right now marlin has exactly engouh memory to fit on the AT90 128


	Start gCode
G90
G28
G29
G1 Z.2 F9000 ;move the platform down
G92 E0 ;zero the extruded length
G1 F300 E3 Z.2
G1 X-50 E50 Z0.2
G92 E0 ;zero the extruded length again

	Optional
M420 S1 ;loads previous bed leving matrix



	Pin Config Diagnstics
	M115
READ: FIRMWARE_NAME:Marlin 1.1.9 (Github) SOURCE_CODE_URL:https://github.com/MarlinFirmware/Marlin PROTOCOL_VERSION:1.0 MACHINE_TYPE:3D Printer EXTRUDER_COUNT:1 UUID:40ce61d1-2ed0-4530-8304-55e3eeaaebff
:SERIAL_XON_XOFF:0
EEPROM:1
VOLUMETRIC:1
AUTOREPORT_TEMP:1
PROGRESS:0
PRINT_JOB:1
AUTOLEVEL:1
Z_PROBE:1
LEVELING_DATA:1
BUILD_PERCENT:0
SOFTWARE_POWER:0
TOGGLE_LIGHTS:0
CASE_LIGHT_BRIGHTNESS:0
EMERGENCY_PARSER:0
AUTOREPORT_SD_STATUS:0
THERMAL_PROTECTION:1

    Pin Configuration
	M43
READ: PIN:   0   Port: D0        <unused/unknown>            Input  = 1    TIMER0B   PWM:   128    WGM: 3    COM0B: 0    CS: 3    TCCR0A: 3    TCCR0B: 3    TIMSK0: 5   compare interrupt enabled   overflow interrupt enabled  
READ: PIN:   1   Port: D1        <unused/unknown>            Input  = 1    TIMER2B   PWM:     0    WGM: 1    COM2B: 0    CS: 2    TCCR2A: 1    TCCR2B: 2    TIMSK2: 0  
READ: PIN:   2   Port: D2        E2_STEP_PIN                 Input  = 1  
READ: PIN:   3   Port: D3        E2_DIR_PIN                  Output = 0  
READ: PIN:   4   Port: D4        LCD_PINS_D7                 Output = 0  
READ: PIN:   5   Port: D5        LCD_PINS_D6                 Output = 0  
READ: PIN:   6   Port: D6        LCD_PINS_D5                 Output = 0  
READ: PIN:   7   Port: D7        LCD_PINS_D4                 Output = 0  
READ: PIN:   8   Port: E0        LCD_PINS_ENABLE             Output = 0  
READ: PIN:   9   Port: E1        LCD_PINS_RS                 Output = 1  
READ: PIN:  10   Port: C0        BTN_EN1                     Input  = 1  
READ: PIN:  11   Port: C1        BTN_EN2                     Input  = 1  
READ: PIN:  12   Port: C2        BTN_ENC                     Input  = 1  
READ: PIN:  13   Port: C3        E0_ENABLE_PIN               protected 
READ: PIN:  14   Port: C4        HEATER_BED_PIN              protected 
READ: PIN:  15   Port: C5        HEATER_0_PIN                protected 
READ: PIN:  16   Port: C6        FAN_PIN                     protected 
READ: PIN:  17   Port: C7        Z_ENABLE_PIN                protected 
READ: PIN:  18   Port: E6        Y_ENABLE_PIN                protected 
READ: PIN:  19   Port: E7        X_ENABLE_PIN                protected 
READ: PIN:  20   Port: B0        AVR_SS_PIN                  Input  = 1  
READ: .                          SDSS                        Input  = 1
READ: .                          SS_PIN                      Input  = 1
READ: PIN:  21   Port: B1        AVR_SCK_PIN                 Input  = 1  
READ: .                          SCK_PIN                     Input  = 1
READ: PIN:  22   Port: B2        AVR_MOSI_PIN                Input  = 1  
READ: .                          MOSI_PIN                    Input  = 1
READ: PIN:  23   Port: B3        AVR_MISO_PIN                Input  = 1  
READ: .                          MISO_PIN                    Input  = 1
READ: PIN:  24   Port: B4        Y_MAX_PIN                   protected 
READ: .                          Y_STOP_PIN                  protected 
READ: PIN:  25   Port: B5        E1_STEP_PIN                 Input  = 1    TIMER1A   PWM:  2000    WGM: 4    COM1A: 0    CS: 2    TCCR1A: 0    TCCR1B: 10    TIMSK1: 2   non-standard PWM mode   compare interrupt enabled  
READ: PIN:  26   Port: B6        <unused/unknown>            Input  = 1    TIMER1B   PWM:     0    WGM: 4    COM1B: 0    CS: 2    TCCR1A: 0    TCCR1B: 10    TIMSK1: 2   non-standard PWM mode  
READ: PIN:  27   Port: B7        <unused/unknown>            Input  = 1    TIMER1C   PWM:     0    WGM: 4    COM1C: 0    CS: 2    TCCR1A: 0    TCCR1B: 10    TIMSK1: 2   non-standard PWM mode  
READ:  .                  TIMER0A is also tied to this pin                  TIMER0A   PWM:   130    WGM: 3    COM0A: 0    CS: 3    TCCR0A: 3    TCCR0B: 3    TIMSK0: 5   overflow interrupt enabled
READ: PIN:  28   Port: A0        X_STEP_PIN                  protected 
READ: PIN:  29   Port: A1        X_DIR_PIN                   protected 
READ: PIN:  30   Port: A2        Y_STEP_PIN                  protected 
READ: PIN:  31   Port: A3        Y_DIR_PIN                   protected 
READ: PIN:  32   Port: A4        Z_STEP_PIN                  protected 
READ: PIN:  33   Port: A5        Z_DIR_PIN                   protected 
READ: PIN:  34   Port: A6        E0_STEP_PIN                 protected 
READ: PIN:  35   Port: A7        E0_DIR_PIN                  protected 
READ: PIN:  36   Port: E4        Z_MIN_PIN                   protected 
READ: .                          Z_STOP_PIN                  protected 
READ: PIN:  37   Port: E5        E1_DIR_PIN                  Output = 0  
READ: PIN:  38   Port: F0 (A 0)  TEMP_BED_PIN                protected 
READ: PIN:  39   Port: F1 (A 1)  TEMP_0_PIN                  protected 
READ: PIN:  40   Port: F2 (A 2)  FILWIDTH_PIN                Analog in =   214  
READ: .                          TEMP_1_PIN                  Analog in =   216
READ: PIN:  41   Port: F3 (A 3)  TEMP_2_PIN                  Analog in =   292  
READ: PIN:  42   Port: F4 (A 4)  E1_ENABLE_PIN               Output = 1  
READ: PIN:  43   Port: F5 (A 5)  E2_ENABLE_PIN               Output = 1  
READ: PIN:  44   Port: F6 (A 6)  HEATER_1_PIN                Output = 0  
READ: PIN:  45   Port: F7 (A 7)  HEATER_2_PIN                Output = 0  
READ: PIN:  46   Port: E2        <unused/unknown>            Input  = 1
READ: PIN:  47   Port: E3        X_MIN_PIN                   protected 
READ: .                          X_STOP_PIN                  protected 

