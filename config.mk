# Control  board config (careful, this is a Makefile) 

# enable debug GPIOs for LoRaWAN (RX and TX timings).
LORA_DEBUG_GPIO=0

ENABLE_ASSERTS=1

# The build directory
BUILD_DIR=build

WARNINGS=-Wall -Wextra -Werror=implicit-function-declaration

# mostly this enables/disables asserts, I think
DEBUG_FLAG=1



#ASF4 prefix (change if you want to experiment with START, maybe) 
ASF4_PREFIX=asf4


