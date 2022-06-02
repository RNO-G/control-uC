#/bin/sh 

SN=""

if [[ -z "${JLINK_SERIAL}" ]] 
then 
  echo "No serial number specified, define JLINK_SERIAL if you want to" 
else
  SN="-select usb=${JLINK_SERIAL}"
fi 

DISPLAY=:notarealdisplay sudo JLinkGDBServer -if SWD -device ATSAMD21J18 $SN $@
