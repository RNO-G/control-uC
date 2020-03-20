# This is mostly ported from asf4/gcc/Makefile with the relevant modifications. 



ASF4_USB_DIRS = \
hpl/usb \
usb/ \
usb/device \
usb/class/cdc/device \
usb 


ASF4_DIRS = \
hpl/tc \
hpl/systick \
samd21a/gcc/gcc \
hpl/dmac \
hal/src \
samd21a/gcc \
hpl/pm \
hpl/sysctrl \
hal/utils/src \
hpl/sercom \
hpl/eic \
hpl/gclk \
hpl/wdt \
hpl/rtc \
hpl/nvmctrl \
hpl/core \


ASF4_USB_INCLUDES = \
-I$(ASF4_PREFIX)/usb \
-I$(ASF4_PREFIX)/usb/class/cdc \
-I$(ASF4_PREFIX)/usb/class/cdc/device \
-I$(ASF4_PREFIX)/usb/device \


ASF4_INCLUDES = \
-I$(ASF4_PREFIX)/ \
-I$(ASF4_PREFIX)/config \
-I$(ASF4_PREFIX)/hal/include \
-I$(ASF4_PREFIX)/hal/utils/include \
-I$(ASF4_PREFIX)/hpl/core \
-I$(ASF4_PREFIX)/hpl/dmac \
-I$(ASF4_PREFIX)/hpl/gclk \
-I$(ASF4_PREFIX)/hpl/nvmctrl \
-I$(ASF4_PREFIX)/hpl/pm \
-I$(ASF4_PREFIX)/hpl/port \
-I$(ASF4_PREFIX)/hpl/rtc \
-I$(ASF4_PREFIX)/hpl/sercom \
-I$(ASF4_PREFIX)/hpl/sysctrl \
-I$(ASF4_PREFIX)/hpl/systick \
-I$(ASF4_PREFIX)/hpl/tc \
-I$(ASF4_PREFIX)/hpl/wdt \
-I$(ASF4_PREFIX)/hri \
-I$(ASF4_PREFIX)/CMSIS/Core/Include \
-I$(ASF4_PREFIX)/samd21a/include



ASF4_MKDIRS = $(foreach dir, $(ASF4_DIRS), $(addprefix $(BUILD_DIR)/$(ASF4_PREFIX)/, $(dir)))

ASF4_USB_MKDIRS =  $(foreach dir, $(ASF4_USB_DIRS), $(addprefix $(BUILD_DIR)/$(ASF4_PREFIX)/, $(dir)))


ASF4_USB_OBJS = \
usb/class/cdc/device/cdcdf_acm.o \
hpl/usb/hpl_usb.o \
usb/device/usbdc.o \
usb/usb_protocol.o \
hal/src/hal_init.o \
hal/src/hal_usb_device.o 

ASF4_RAW_OBJS =  \
hal/src/hal_io.o \
hpl/systick/hpl_systick.o \
hal/src/hal_calendar.o \
hpl/wdt/hpl_wdt.o \
samd21a/gcc/gcc/startup_samd21.o \
hal/utils/src/utils_syscalls.o \
samd21a/gcc/system_samd21.o \
hpl/nvmctrl/hpl_nvmctrl.o \
hal/src/hal_spi_m_sync.o \
hal/src/hal_timer.o \
hal/src/hal_i2c_m_async.o \
hal/src/hal_i2c_m_sync.o \
hal/src/hal_flash.o \
hal/src/hal_delay.o \
hpl/eic/hpl_eic.o \
hal/src/hal_ext_irq.o \
hpl/sysctrl/hpl_sysctrl.o \
hpl/core/hpl_init.o \
hal/src/hal_wdt.o \
hpl/core/hpl_core_m0plus_base.o \
hal/utils/src/utils_assert.o \
hpl/dmac/hpl_dmac.o \
hpl/pm/hpl_pm.o \
hal/src/hal_usart_async.o \
hpl/gclk/hpl_gclk.o \
hal/utils/src/utils_list.o \
hpl/rtc/hpl_rtc.o \
hpl/tc/hpl_tc.o \
hpl/sercom/hpl_sercom.o \
hal/utils/src/utils_ringbuffer.o \
hal/src/hal_gpio.o \
hal/utils/src/utils_event.o \
hal/src/hal_sleep.o \
hal/src/hal_atomic.o \

##todo, cull 
ASF4_RAW_BL_OBJS =  \
hal/src/hal_io.o \
samd21a/gcc/gcc/startup_samd21.o \
hal/utils/src/utils_syscalls.o \
samd21a/gcc/system_samd21.o \
hpl/nvmctrl/hpl_nvmctrl.o \
hal/src/hal_spi_m_sync.o \
hpl/sysctrl/hpl_sysctrl.o \
hpl/core/hpl_init.o \
hpl/core/hpl_core_m0plus_base.o \
hal/utils/src/utils_assert.o \
hpl/dmac/hpl_dmac.o \
hpl/pm/hpl_pm.o \
hal/src/hal_usart_async.o \
hpl/gclk/hpl_gclk.o \
hal/src/hal_flash.o \
hal/src/hal_init.o \
hal/utils/src/utils_list.o \
hpl/sercom/hpl_sercom.o \
hal/utils/src/utils_ringbuffer.o \
hal/src/hal_gpio.o \
hal/utils/src/utils_event.o \
hal/src/hal_sleep.o \
hpl/tc/hpl_tc.o \
hal/src/hal_atomic.o \

ASF4_OBJS := $(foreach obj, $(ASF4_RAW_OBJS), $(addprefix $(BUILD_DIR)/$(ASF4_PREFIX)/, $(obj)))
ASF4_BL_OBJS := $(foreach obj, $(ASF4_RAW_BL_OBJS), $(addprefix $(BUILD_DIR)/$(ASF4_PREFIX)/, $(obj)))

