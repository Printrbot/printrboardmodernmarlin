; Play Version 2 Heated Bed 12-9-2018
G21 ;metric values
G90 ;absolute positioning
M82 ;set extruder to absolute mode
G28 ;home
G29 F1200 ;run auto bed leveling
G1 X165 F2500
G1 Z0 F9000 ;move the platform down 
G92 E0 ;zero the extruded length
G1 Z1.5 E25 F500 ;extrude 25mm of feed stock
M106 S255
G4 S3 ;wait 5 seconds while fan runs
M106 S0
G92 E0 ;zero the extruded length again
G1 X150 Z2 F18000
G1 Z5
G1 X130 F12000
G1 F9000
M900 K0.1 ;set linear advance value for PLA
;Put printing message on LCD screen
M117 Printing...