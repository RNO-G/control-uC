
# this defaults to a cmake build, but we don't need to do it that way 
#

LORAWAN_INCLUDES=-Ilorawan/system -Ilorawan/board -Ilorawan/mac -Ilorawan/radio/ -Ilorawan/peripherals/soft-se/

RAW_LORAWAN_OBJS=$(addprefix lorawan/system/, gpio.o timer.o delay.o systime.o )
RAW_LORAWAN_OBJS+=$(addprefix lorawan/board/, rtc-board.o delay-board.o gpio-board.o sx1272-board.o utilities.o spi-board.o board.o )
RAW_LORAWAN_OBJS+=lorawan/radio/sx1272/sx1272.o 
RAW_LORAWAN_OBJS+=lorawan/lorawan.o 
RAW_LORAWAN_OBJS+=$(addprefix lorawan/mac/, LoRaMac.o LoRaMacAdr.o LoRaMacClassB.o LoRaMacCommands.o LoRaMacConfirmQueue.o LoRaMacCrypto.o LoRaMacParser.o LoRaMacSerializer.o region/Region.o region/RegionCommon.o region/RegionUS915.o )
RAW_LORAWAN_OBJS+=$(addprefix lorawan/peripherals/soft-se/, soft-se.o aes.o cmac.o )

LORAWAN_OBJS=$(addprefix $(BUILD_DIR)/, $(RAW_LORAWAN_OBJS))

LORAWAN_DIRS=lorawan lorawan/system lorawan/board lorawan/radio/sx1272 lorawan/mac/region  lorawan/peripherals/soft-se
LORAWAN_MKDIRS= $(foreach dir, $(LORAWAN_DIRS), $(addprefix $(BUILD_DIR)/, $(dir)))


