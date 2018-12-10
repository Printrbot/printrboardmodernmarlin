G21 ;metric values
G90 ;absolute positioning
M82 ;set extruder to absolute mode
G28 ;home
G29 F1200 ;run auto bed leveling
G1 X170 F2000
G1  Z0 F9000 ;move the platform down 
G92 E0 ;zero the extruded length
G1 F500 E25 ;extrude 25mm of feed stock
M106 S255
G4 S5 ;wait 5 seconds while fan runs
M106 S0
G92 E0 ;zero the extruded length again
G1 X160 F18000
G1 X 170 F1500
G1 F9000
M900 K0.1 ;set linear advance value for PLA
;Put printing message on LCD screen
M117 Printing...