#include <string.h>
#include "application/update_bootloader.h"
#include "linker/map.h"
#include "bootloader_bin.h"
#include "shared/driver_init.h"
#include "hpl_user_area.h"

uint32_t * user0 = (uint32_t*) 0x804000;

int bootloader_check()
{
  uint8_t buf[64];

  uint32_t the_start = (uint32_t) &__boot_rom_start__;
  for (unsigned i = 0; i < bootloader_image_len; i+= sizeof(buf))
  {
    flash_read(&FLASH,the_start +i, buf,sizeof(buf));
    int checklen = bootloader_image_len - i  > sizeof(buf) ? sizeof(buf) : bootloader_image_len-i;
    if (memcmp(buf, bootloader_image+i, checklen))
    {
      return 1;
    }
  }

  return 0;

}

int bootloader_update()
{
  if (bootloader_is_locked()) return -1;
  return bootloader_image_len!= flash_write(&FLASH, (uint32_t) &__boot_rom_start__, (uint8_t*) bootloader_image,bootloader_image_len);
}

int bootloader_is_locked()
{
  uint32_t bootprot = _user_area_read_bits(user0, 0, 3);
  return bootprot != 0x7;
}

int bootloader_lock()
{
  return ERR_NONE != _user_area_write_bits(user0, 0, 0x1, 3);
}

int bootloader_unlock()
{
 return ERR_NONE !=  _user_area_write_bits(user0, 0, 0x7, 3);
}

