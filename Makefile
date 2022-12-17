#TLDR:  make mcu for micro, otherwise make install to install rno-g-control.h header  (optionally with PREFIX=dest) 





include config.mk 
include station_number.mk


CFLAGS+= \
-x c -mthumb -Os -ffunction-sections -c -std=gnu99  \
-D__SAMD21J18A__ -mcpu=cortex-m0plus -MD -MP  --specs=nano.specs -g3 -D_GNU_SOURCE -fanalyzer

ifeq ($(LORA_DEBUG_GPIO),1)
	CFLAGS += -DUSE_RADIO_DEBUG
endif

ifeq ($(DEBUG_FLAG),1)
	CFLAGS += -DDEBUG
endif

REV=rev_E
VERSION_FLAGS=
ifeq ($(BUILD_FOR_REV_D),1)
	CFLAGS += -D_RNO_G_REV_D
	VERSION_FLAGS += -D_RNO_G_REV_D
	REV=rev_D 
endif


CFLAGS+= $(WARNINGS) 


include asf4.mk 
include lorawan.mk 


CC=arm-none-eabi-gcc 
AS=arm-none-eabi-as 
OC=arm-none-eabi-objcopy 
SZ=arm-none-eabi-size 

#shared flags between application and bootloader
LD_FLAGS_PRE= -Wl,--start-group -lm -Wl,--end-group -mthumb 
LD_FLAGS_POST=-Llinker/ --specs=nano.specs -mcpu=cortex-m0plus -Wl,--gc-sections 


INCLUDES=$(ASF4_INCLUDES) -I./ -Iinclude/

SHARED_OBJS=config_block.o spi_flash.o shared_memory.o programmer.o io.o printf.o driver_init.o base64.o
 
#these need to be compiled separately so that we can use the _BOOTLOADER_ flag 
APP_SHARED_OBJS=$(addprefix $(BUILD_DIR)/shared/, $(SHARED_OBJS))
BL_SHARED_OBJS=$(addprefix $(BUILD_DIR)/bootloader/shared/, $(SHARED_OBJS))

APP_OBJS=$(ASF4_OBJS) $(APP_SHARED_OBJS) $(LORAWAN_OBJS) 
APP_OBJS+=$(addprefix $(BUILD_DIR)/application/, main.o debug.o sbc.o monitors.o power.o)
APP_OBJS+=$(addprefix $(BUILD_DIR)/application/, lte.o i2cbus.o gpio_expander.o time.o reset.o)
APP_OBJS+=$(addprefix $(BUILD_DIR)/application/, mode.o lowpower.o commands.o report.o)
APP_OBJS+=$(addprefix $(BUILD_DIR)/application/, i2cbusmux.o )
BL_OBJS=$(BUILD_DIR)/bootloader/bootloader.o  $(ASF4_BL_OBJS) $(BL_SHARED_OBJS) 


# List the dependency files
APP_DEPS := $(APP_OBJS:%.o=%.d)
BL_DEPS := $(BL_OBJS:%.o=%.d)

RNO_G_INSTALL_DIR?=/rno-g
PREFIX?=$(RNO_G_INSTALL_DIR)



MKDIRS:= $(BUILD_DIR) $(ASF4_MKDIRS) $(LORAWAN_MKDIRS) $(BUILD_DIR)/application $(BUILD_DIR)/bootloader  $(BUILD_DIR)/shared $(BUILD_DIR)/bootloader/shared
OUTPUT_NAME := $(BUILD_DIR)/rno-G-uC-main
BL_OUTPUT_NAME := $(BUILD_DIR)/rno-G-uC-bootloader
COMBO_NAME := $(BUILD_DIR)/rno-G-uC-combined

.PHONY: help install mcu clean rev release

help: 
	@echo "Targets: " 
	@echo "  help:  print this message"
	@echo "  mcu:  build mcu firwmware (requires cross-compiler)"
	@echo "  client-build:  build client programs / libraries (on SBC)"
	@echo "  client-install: install client programs /libraries  (PREFIX is influential, defaults to /rno-g/ or RNO_G_INSTALL_DIR)"
	@echo "  client-uninstall: uninstall client  program /libraries (PREFIX is influential, defaults to /rno-g/ or RNO_G_INSTALL_DIR)"
	@echo "  install: install header file (PREFIX is influential, defaults to /rno-g or RNO_G_INSTALL_DIR/"
	@echo "  uninstall: uninstall header file (PREFIX is influential, defaults to /rno-g or RNO_G_INSTALL_DIR/"
	@echo "  clean: Clean everything"


# MCU 
mcu: $(MKDIRS) $(OUTPUT_NAME).bin $(BL_OUTPUT_NAME).bin $(OUTPUT_NAME).hex $(BL_OUTPUT_NAME).hex $(COMBO_NAME).bin rev

rev: 
	echo $(REV) > $(BUILD_DIR)/rev.txt
	echo $(INITIAL_STATION_NUMBER) > $(BUILD_DIR)/initial_station_number.txt

install: 
	install include/rno-g-control.h $(PREFIX)/include/

client-build: 
	$(MAKE) -C client
	$(MAKE) -C loader

client-install: 
	$(MAKE) -C client install
	$(MAKE) -C loader install

client-uninstall: 
	$(MAKE) -C client uninstall
	$(MAKE) -C loader uninstall



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


$(COMBO_NAME).bin: $(BL_OUTPUT_NAME).bin $(OUTPUT_NAME).bin 
	cat $^ > $@ 

%.hex: %.elf
	$(OC) -O ihex -R .eeprom -R .fuse -R .lock -R .signature $< $@

%.eep: %.elf
	$(OC) -j .eeprom --set-section-flags=.eeprom=alloc,load --change-section-lma \
        .eeprom=0 --no-change-warnings -O binary $< \
        $@ || exit 0
$.lss: %.elf 
	$(OC) -h -S $< > $@ 

station_number.mk:  
	@echo "Creating default station_number.mk" 
	@echo "INITIAL_STATION_NUMBER=999" > $@


config.mk: config.mk.default 
	@echo "Copying config.mk.default to config.mk (backing up old if it exists)" 
	@ if [ -f "$@" ] ; then  diff  -q $@ $< || cp $@ $@.`date -Is`.backup ;  fi 
	@cp $< $@

$(OUTPUT_NAME).elf: $(APP_OBJS)
	@echo Building target: $@
	$(CC) -o $@ $^ $(LD_FLAGS_PRE) -Wl,-Map=$(OUTPUT_NAME).map -Tlinker/main.ld  $(LD_FLAGS_POST) 
	"arm-none-eabi-size" $@

$(BL_OUTPUT_NAME).elf: $(BL_OBJS)
	@echo Building target: $@
	$(CC) -o $@ $^ $(LD_FLAGS_PRE) -Wl,-Map=$(BL_OUTPUT_NAME).map -Tlinker/bootloader.ld $(LD_FLAGS_POST) 
	"arm-none-eabi-size" $@


# special case lorawan includes  (so we don't have to modify the reference implementation files) 
$(BUILD_DIR)/lorawan/%.o: lorawan/%.c config.mk lorawan.mk 
	@echo Building lorawan file: $<
	$(CC) $(CFLAGS) $(INCLUDES) $(LORAWAN_INCLUDES) -MF$(@:%.o=%.d) -MT$(@:%.o=%.d) -MT$(@:%.o=%.o) -DREGION_US915 -o $@ $<
	@echo Finished building: $<

# special case shared for bootloader to define _BOOTLOADER_
$(BUILD_DIR)/bootloader/shared/%.o: shared/%.c config.mk
	@echo Building bootloader file: $<
	$(CC) $(CFLAGS) -D_BOOTLOADER_ $(INCLUDES) -MF$(@:%.o=%.d) -MT$(@:%.o=%.d)  -MT$(@:%.o=%.o) -o $@ $<
	@echo Finished building: $<

# likewise special case shared for bootloader to define _BOOTLOADER_
$(BUILD_DIR)/bootloader/%.o: bootloader/%.c Makefile config.mk 
	@echo Building bootloader file: $<
	$(CC) -D_BOOTLOADER_ $(CFLAGS)  $(INCLUDES) -MF$(@:%.o=%.d) -MT$(@:%.o=%.d)  -MT$(@:%.o=%.o) -o $@ $<
	@echo Finished building: $<

#special case config block for INIITAL_STATION_NUMBER (only on application?)
$(BUILD_DIR)/shared/config_block.o: shared/config_block.c Makefile config.mk station_number.mk
	@echo Building file: $<
	$(CC) $(CFLAGS) -DINITIAL_STATION_NUMBER=$(INITIAL_STATION_NUMBER) $(INCLUDES) -MF$(@:%.o=%.d) -MT$(@:%.o=%.d) -MT$(@:%.o=%.o) -o $@ $<
	@echo Finished building: $<


# Compiler targets
$(BUILD_DIR)/%.o: %.c Makefile config.mk
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


$(BUILD_DIR)/test_base64: shared/base64.c | $(BUILD_DIR) 
	gcc -o $@ -Os -D_TEST_ -D_HOST_ $<  -I./

release: 
	@echo "#include \"config/config.h\" "> $(BUILD_DIR)/release.c
	@echo "#include <stdio.h> ">> $(BUILD_DIR)/release.c
	@echo "int main(void) { puts(APP_VERSION \"\\n\"); return 0;}  " >> $(BUILD_DIR)/release.c
	@gcc $(VERSION_FLAGS) -I. -o $(BUILD_DIR)/release.exe $(BUILD_DIR)/release.c 
	@$(BUILD_DIR)/release.exe > $(BUILD_DIR)/release
	@echo Version is `cat $(BUILD_DIR)/release`
	@rm -rf $(BUILD_DIR)/current_release
	@mkdir -p $(BUILD_DIR)/current_release
	@ln $(OUTPUT_NAME).hex $(BUILD_DIR)/current_release/`basename $(OUTPUT_NAME)`-`cat $(BUILD_DIR)/release`.hex
	@ln $(OUTPUT_NAME).bin $(BUILD_DIR)/current_release/`basename $(OUTPUT_NAME)`-`cat $(BUILD_DIR)/release`.bin
	@ln $(BL_OUTPUT_NAME).hex $(BUILD_DIR)/current_release/`basename $(BL_OUTPUT_NAME)`-`cat $(BUILD_DIR)/release`.hex
	@ln $(BL_OUTPUT_NAME).bin $(BUILD_DIR)/current_release/`basename $(BL_OUTPUT_NAME)`-`cat $(BUILD_DIR)/release`.bin
	@ln $(COMBO_NAME).bin $(BUILD_DIR)/current_release/`basename $(COMBO_NAME)`-`cat $(BUILD_DIR)/release`.bin
	@rm -f $(BUILD_DIR)/release-`cat $(BUILD_DIR)/release`.tar.gz 
	@rm -rf $(BUILD_DIR)/release-`cat $(BUILD_DIR)/release`
	@mv -f $(BUILD_DIR)/current_release  $(BUILD_DIR)/release-`cat $(BUILD_DIR)/release`
	@tar -cvzf $(BUILD_DIR)/release-`cat $(BUILD_DIR)/release`.tar.gz $(BUILD_DIR)/release-`cat $(BUILD_DIR)/release` > $(BUILD_DIR)/last_release 
	@gh release create `cat $(BUILD_DIR)/release` --generate-notes  $(BUILD_DIR)/release-`cat $(BUILD_DIR)/release`/*




$(MKDIRS):
	mkdir -p "$@"

clean:
	rm -rf $(BUILD_DIR) 
	make -C client clean
	make -C loader clean

uninstall: 
	rm -f $(PREFIX)/include/rno-g-control.h
