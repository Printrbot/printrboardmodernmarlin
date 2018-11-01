##Marlin 1.1.9
Printrboard and Modern Marlin

#This release is configured for a Printrbot Play with stepper drivers inverted, more releases will come in the following days.
    
    9 Point Auto Bilinear Bedleving
    Linear Advance Version 1.5 Greatly Improves Print Quality, Resolution and Speed
    Babystepping allows live z offset during the start of the print

This isn't your grandmas 3D printer so please make sure your inductive probe is adjusted properly

    Cold End Fan not tested E0_Auto_Fan mapped to Pin 22 - will get to this asap

	 Machine Definition can be updated using gCode
    
     Current Steps M92 X80 Y80 Z2020 E94.5
	

Linear Advance is enabled by default
    retraction distance should be limited to 0.7mm or less
    K value default 0.08
    M900 K0.08
    Please calibrate your K-factor and submit results 
    http://marlinfw.org/tools/lin_advance/k-factor.html

#Babystepping
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


#Start gCode
    G90
    G28
    G29
    G1 Z.2 F9000 ;move the platform down
    G92 E0 ;zero the extruded length
    G1 F300 E3 Z.2
    G1 X50 E50 Z0.2
    G92 E0 ;zero the extruded length again

Optional
    M420 S1 ;loads previous bed leving matrix
