#/bin/sh 
DISPLAY=:notarealdisplay sudo JLinkGDBServer -if SWD -device ATSAMD21J18 $@
