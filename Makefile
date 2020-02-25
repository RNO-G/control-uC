BUILD_DIR=build

#This includes all the auto-generated stuff from ASF4 
include asf4.mk 

#This includes the LoRaWAN implementation 
include lorawan.mk 

CC=arm-none-eabi-gcc 
AS=arm-none-eabi-as 
OC=arm-none-eabi-objcopy 
SZ=arm-none-eabi-size 

#shared flags between application and bootloader
LD_FLAGS_PRE=-Wl,--start-group -lm -Wl,--end-group -mthumb 
LD_FLAGS_POST=-Llinker/ --specs=nano.specs -Wl,--gc-sections -mcpu=cortex-m0plus 


INCLUDES=$(ASF4_INCLUDES) -I./ 

CFLAGS= \
-x c -mthumb -DDEBUG -Os -ffunction-sections -mlong-calls -g3 -Wall -c -std=gnu99  \
-D__SAMD21J18A__ -mcpu=cortex-m0plus -MD -MP 


SHARED_OBJS= $(addprefix $(BUILD_DIR)/shared/, config_block.o spi_flash.o shared_memory.o programmer.o )

APP_OBJS=$(BUILD_DIR)/application/main.o $(ASF4_OBJS) $(SHARED_OBJS) $(LORAWAN_OBJS) 

BL_OBJS=$(BUILD_DIR)/bootloader/bootloader.o $(ASF4_BL_OBJS) $(SHARED_OBJS) 

# List the dependency files
DEPS := $(OBJS:%.o=%.d)
BL_DEPS := $(BL_OBJS:%.o=%.d)
MKDIRS:= $(BUILD_DIR)  $(ASF4_MKDIRS) $(LORAWAN_MKDIRS) $(BUILD_DIR)/application $(BUILD_DIR)/bootloader  $(BUILD_DIR)/shared
OUTPUT_NAME := $(BUILD_DIR)/rno-G-uC-main
BL_OUTPUT_NAME := $(BUILD_DIR)/rno-G-uC-bootloader

# All Target
all: $(MKDIRS) $(OUTPUT_NAME).bin $(BL_OUTPUT_NAME).bin


%.bin: %.elf
	$(OC) -O binary $< $@

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
	$(CC) $(CFLAGS) $(INCLUDES) $(LORAWAN_INCLUDES) -MF$(@:%.o=%.d) -MT$(@:%.o=%.d)   -o $@ $<
	@echo Finished building: $<

# Compiler targets
$(BUILD_DIR)/%.o: %.c
	@echo Building file: $<
	$(CC) $(CFLAGS) $(INCLUDES) -MF$(@:%.o=%.d) -MT$(@:%.o=%.d)  -o $@ $<
	@echo Finished building: $<


$(BUILD_DIR)/%.o: %.s
	@echo Building file: $<
	$(AS) $(CFLAGS) -MF $(@:%.o=%.d) -MT$(@:%.o=%.d)   -o $@ $<
	@echo Finished building: $<

$(BUILD_DIR)/%.o: %.S
	@echo Building file: $<
	$(CC) $(CFLAGS) -MF $(@:%.o=%.d) -MT$(@:%.o=%.d)   -o $@ $<
	@echo Finished building: $<

# Detect changes in the dependent files and recompile the respective object files.
ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(DEPS)),)
-include $(DEPS)
endif
ifneq ($(strip $(BL_DEPS)),)
-include $(BL_DEPS)
endif

endif

$(MKDIRS):
	mkdir -p "$@"

clean:
	rm -rf $(BUILD_DIR) 
