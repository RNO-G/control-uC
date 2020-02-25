#! /bin/sh 
# Imports relevant files from lorawan reference implementation implementation (LoraMac-node) 
# In our case we only care about the sx1272 
# If you've made changes... be careful this might override. 
# we are going to steal most of the implementation from the saml21

if [ "$#" -ne 1 ] || ! [ -d "$1" ]; then
    echo "Usage: $0 LORAWAN-DIRECTORY" >&2
      exit 1
fi


# radio interface
mkdir -p radio 
cp -r $1/src/radio/sx1272/ radio 
cp -r $1/src/radio/radio.h radio 

mkdir -p mac 
mkdir -p mac/region 

cp -r $1/src/mac/*.h mac
cp -r $1/src/mac/*.c mac
cp -r $1/src/mac/region/*.h mac/region
cp -r $1/src/mac/region/*.c mac/region


# we only need the parts of system used inside mac/radio 
# we will also provide our own implementation of gpio.c 
SYSTEM_FILES="spi.h gpio.h gpio.c timer.h timer.c delay.h delay.c systime.h systime.c"

mkdir -p system 

for i in $SYSTEM_FILES ; 
  do cp $1/src/system/$i system/$i 
done

BOARD_FILES="delay-board.h rtc-board.h sx1272-board.h utilities.h gpio-board.h"

for i in $BOARD_FILES ; 
  do cp $1/src/boards/$i board/$i 
done

SAML21_FILES="gpio-board.c delay-board.c"
for i in $SAML21_FILES ; 
  do cp $1/src/boards/SAML21/$i board/$i 
done

MCU_FILES="utilities.c"
for i in $MCU_FILES ; 
  do cp $1/src/boards/mcu/$i board/$i 
done








