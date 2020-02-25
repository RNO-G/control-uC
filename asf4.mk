# This is mostly ported from asf4/gcc/Makefile with the relevant modifications. 

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
hpl/gclk \
hpl/wdt \
hpl/rtc \
hpl/nvmctrl \
hpl/core \


ASF4_INCLUDES = \
-Iasf4/ \
-Iasf4/config \
-Iasf4/examples \
-Iasf4/hal/include \
-Iasf4/hal/utils/include \
-Iasf4/hpl/core \
-Iasf4/hpl/dmac \
-Iasf4/hpl/gclk \
-Iasf4/hpl/nvmctrl \
-Iasf4/hpl/pm \
-Iasf4/hpl/port \
-Iasf4/hpl/rtc \
-Iasf4/hpl/sercom \
-Iasf4/hpl/sysctrl \
-Iasf4/hpl/systick \
-Iasf4/hpl/tc \
-Iasf4/hpl/wdt \
-Iasf4/hri \
-Iasf4/CMSIS/Core/Include \
-Iasf4/samd21a/include




ASF4_MKDIRS= $(foreach dir, $(ASF4_DIRS), $(addprefix $(BUILD_DIR)/asf4/, $(dir)))

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
hal/src/hal_i2c_m_sync.o \
hal/src/hal_delay.o \
hpl/sysctrl/hpl_sysctrl.o \
hpl/core/hpl_init.o \
hal/src/hal_wdt.o \
hpl/core/hpl_core_m0plus_base.o \
hal/utils/src/utils_assert.o \
hpl/dmac/hpl_dmac.o \
hpl/pm/hpl_pm.o \
hal/src/hal_usart_async.o \
hpl/gclk/hpl_gclk.o \
hal/src/hal_flash.o \
hal/src/hal_init.o \
hal/utils/src/utils_list.o \
hpl/rtc/hpl_rtc.o \
hpl/tc/hpl_tc.o \
driver_init.o \
hpl/sercom/hpl_sercom.o \
hal/utils/src/utils_ringbuffer.o \
hal/src/hal_gpio.o \
hal/utils/src/utils_event.o \
hal/src/hal_sleep.o \
atmel_start.o \
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

ASF4_OBJS := $(foreach obj, $(ASF4_RAW_OBJS), $(addprefix $(BUILD_DIR)/asf4/, $(obj)))
ASF4_BL_OBJS := $(foreach obj, $(ASF4_RAW_BL_OBJS), $(addprefix $(BUILD_DIR)/asf4/, $(obj)))

