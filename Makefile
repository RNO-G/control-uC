CFLAGS= \
-x c -mthumb -DDEBUG -Os -ffunction-sections -Wall -c -std=gnu99  \
-D__SAMD21J18A__ -mcpu=cortex-m0plus -MD -MP  --specs=nano.specs -g3


ifeq ($(DEVBOARD),1) 
BUILD_DIR=builddev
ASF4_PREFIX=devboard
CFLAGS+=-D_DEVBOARD_
else
BUILD_DIR=build
ASF4_PREFIX=asf4
endif



include asf4.mk 

#This includes the LoRaWAN implementation 
include lorawan.mk 


CC=arm-none-eabi-gcc 
AS=arm-none-eabi-as 
OC=arm-none-eabi-objcopy 
SZ=arm-none-eabi-size 

#shared flags between application and bootloader
LD_FLAGS_PRE= -Wl,--start-group -lm -Wl,--end-group -mthumb 
LD_FLAGS_POST=-Llinker/ --specs=nano.specs -mcpu=cortex-m0plus -Wl,--gc-sections 


INCLUDES=$(ASF4_INCLUDES) -I./ 

SHARED_OBJS=config_block.o spi_flash.o shared_memory.o programmer.o io.o printf.o driver_init.o 
 
#these need to be compiled separately so that we can use the _BOOTLOADER_ flag 
APP_SHARED_OBJS=$(addprefix $(BUILD_DIR)/shared/, $(SHARED_OBJS))
BL_SHARED_OBJS=$(addprefix $(BUILD_DIR)/bootloader/shared/, $(SHARED_OBJS))

APP_OBJS=$(BUILD_DIR)/application/main.o $(BUILD_DIR)/application/debug.o $(ASF4_OBJS) $(APP_SHARED_OBJS) $(LORAWAN_OBJS) 
ifneq ($(DEVBOARD),1)
APP_OBJS+=$(BUILD_DIR)/application/lte.o $(BUILD_DIR)/application/i2cbus.o
endif
BL_OBJS=$(BUILD_DIR)/bootloader/bootloader.o  $(ASF4_BL_OBJS) $(BL_SHARED_OBJS) 


# List the dependency files
APP_DEPS := $(APP_OBJS:%.o=%.d)
BL_DEPS := $(BL_OBJS:%.o=%.d)


MKDIRS:= $(BUILD_DIR) $(ASF4_MKDIRS) $(LORAWAN_MKDIRS) $(BUILD_DIR)/application $(BUILD_DIR)/bootloader  $(BUILD_DIR)/shared $(BUILD_DIR)/bootloader/shared
OUTPUT_NAME := $(BUILD_DIR)/rno-G-uC-main
BL_OUTPUT_NAME := $(BUILD_DIR)/rno-G-uC-bootloader

# All Target
all: $(MKDIRS) $(OUTPUT_NAME).bin $(BL_OUTPUT_NAME).bin

# Detect changes in the dependent files and recompile the respective object files.
ifneq ($(MAKECMDGOALS),clean)

ifneq ($(strip $(APP_DEPS)),)
-include $(APP_DEPS)
endif

ifneq ($(strip $(BL_DEPS)),)
-include $(BL_DEPS)
endif

endif



%.bin: %.elf
	$(OC) -O binary $< $@

$(BL_OUTPUT_NAME).bin: $(BL_OUTPUT_NAME).elf
	$(OC) --pad-to=0x4000 --gap-fill=0xff -O binary $< $@



%.hex: %.elf
	$(OC) -O ihex -R .eeprom -R .fuse -R .lock -R .signature $< $@

%.eep: %.elf
	$(OC) -j .eeprom --set-section-flags=.eeprom=alloc,load --change-section-lma \
        .eeprom=0 --no-change-warnings -O binary $< \
        $@ || exit 0
$.lss: %.elf 
	$(OC) -h -S $< > $@ 


$(OUTPUT_NAME).elf: $(APP_OBJS)
	@echo Building target: $@
	$(CC) -o $@ $^ $(LD_FLAGS_PRE) -Wl,-Map=$(OUTPUT_NAME).map -Tlinker/main.ld  $(LD_FLAGS_POST) 
	"arm-none-eabi-size" $@

$(BL_OUTPUT_NAME).elf: $(BL_OBJS)
	@echo Building target: $@
	$(CC) -o $@ $^ $(LD_FLAGS_PRE) -Wl,-Map=$(BL_OUTPUT_NAME).map -Tlinker/bootloader.ld $(LD_FLAGS_POST) 
	"arm-none-eabi-size" $@


# special case lorawan includes  (so we don't have to modify the reference implementation files) 
$(BUILD_DIR)/lorawan/%.o: lorawan/%.c
	@echo Building lorawan file: $<
	$(CC) $(CFLAGS) $(INCLUDES) $(LORAWAN_INCLUDES) -MF$(@:%.o=%.d) -MT$(@:%.o=%.d) -MT$(@:%.o=%.o) -DREGION_US915 -o $@ $<
	@echo Finished building: $<

# special case shared for bootloader to define _BOOTLOADER_
$(BUILD_DIR)/bootloader/shared/%.o: shared/%.c
	@echo Building bootloader file: $<
	$(CC) $(CFLAGS) -D_BOOTLOADER_ $(INCLUDES) -MF$(@:%.o=%.d) -MT$(@:%.o=%.d)  -MT$(@:%.o=%.o) -o $@ $<
	@echo Finished building: $<



# Compiler targets
$(BUILD_DIR)/%.o: %.c
	@echo Building file: $<
	$(CC) $(CFLAGS) $(INCLUDES) -MF$(@:%.o=%.d) -MT$(@:%.o=%.d) -MT$(@:%.o=%.o) -o $@ $<
	@echo Finished building: $<


$(BUILD_DIR)/%.o: %.s
	@echo Building file: $<
	$(AS) $(CFLAGS) -MF $(@:%.o=%.d) -MT$(@:%.o=%.d) -MT$(@:%.o=%.o)  -o $@ $<
	@echo Finished building: $<

$(BUILD_DIR)/%.o: %.S
	@echo Building file: $<
	$(CC) $(CFLAGS) -MF $(@:%.o=%.d) -MT$(@:%.o=%.d)  -MT$(@:%.o=%.o) -o $@ $<
	@echo Finished building: $<



$(MKDIRS):
	mkdir -p "$@"

clean:
	rm -rf $(BUILD_DIR) 
