
# this defaults to a cmake build, but we don't need to do it that way 


LORAWAN_INCLUDES=-Ilorawan/system -Ilorawan/board -Ilorawan/mac -Ilorawan/radio/
LORAWAN_OBJS=$(addprefix $(BUILD_DIR)/lorawan/system/, gpio.o timer.o delay.o systime.o )
LORAWAN_OBJS+=$(addprefix $(BUILD_DIR)/lorawan/board/, rtc-board.o delay-board.o gpio-board.o sx1272-board.o utilities.o spi-board.o )
LORAWAN_OBJS+=$(BUILD_DIR)/lorawan/radio/sx1272/sx1272.o 
LORAWAN_OBJS+=$(addprefix $(BUILD_DIR)/lorawan/mac/, LoRaMac.o LoRaMacAdr.o LoRaMacClassB.o LoRaMacCommands.o LoRaMacConfirmQueue.o LoRaMacCrypto.o LoRaMacParser.o LoRaMacSerializer.o region/Region.o region/RegionCommon.o region/RegionUS915.o )
LORAWAN_DIRS=lorawan lorawan/system lorawan/board lorawan/radio/sx1272 lorawan/mac/region 

LORAWAN_MKDIRS= $(foreach dir, $(LORAWAN_DIRS), $(addprefix $(BUILD_DIR)/, $(dir)))


