#! /bin/sh


if [ -z "$1" ]; then
    echo "Error: Please specify the path to your combined.bin file."
    echo "Usage: $0 /path/to/firmware.bin"
    exit 1
fi

if [ ! -f "$1" ] ; then
  echo "File $1 not found"
  exit 1
fi

echo "Flashing $1 to MCU"

JLinkExe -device ATSAMD21J18 -if swd -speed 4000 -autoconnect 1 << EOF
R
H
LoadFile "$1"
R
G
Exit
EOF

