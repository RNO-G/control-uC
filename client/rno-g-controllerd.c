/** 
 *
 * rno-g-controllerd 
 *
 * The rno-g controller communication process. 
 *
 * exposes a dbus interface (because only one process can talk to the MCU and we need to constantly be listening to it! ) 
 *
 *
 * Cosmin Deaconu <cozzyd@kicp.uchicago.edu> 
 *
 */


#include "rno-g-controller.h" 
#include <stdlib.h> 
#include <stdio.h> 
#include <errno.h> 
#include <time.h>

//use systemd implementation of dbus 
#include <systemd/sd-bus.h> 



const char * DEFAULT_UART = "/dev/ttyController" ; 
rno_g_controller_t *mcu = 0; 
volatile int time_to_quit; 




static void usage() 
{
  fprintf(stderr,"Usage: rno-g-controllerd /dev/ttyX=%s [--user]"); 
  fprintf(stderr," Note: assumes the serial port is set up correctly already! "); 
}


static int method_setval(sd_bus_message * m, void *udata, sd_bus_error * ret_error) 
{

  int what, val; 
  int r = sd_bus_message_read(m,"ii", &what, &val); 
  if (r < 0) 
  {
    fprintf(stderr,"Failed to parse: %s\n", strerror(-r)); 
  }
  if (what < 0 || what >= RNO_G_NOT_A_VAL) 
  {
    sd_bus_error_set_const(ret_error,"org.rno-g.UnknownKey", "Unknown key!"); 
    return -EINVAL; 
  }

  r = rno_g_set_val(mcu, what, val); 

  if (r < 0) 
  {
    fprintf(stderr,"rno_g_set_val returned %d\n", r); 
    sd_bus_error_set_const(ret_error,"org.rno-g.SetValFail", "Set Value Failed!"); 
    return -EIO; 
  }

  return sd_bus_reply_method_return(m, "b", !!r); 
}



static int method_getval(sd_bus_message * m, void *udata, sd_bus_error * ret_error) 
{

  int what, val; 
  int r = sd_bus_message_read(m,"i", &val); 
  if (r < 0) 
  {
    fprintf(stderr,"Failed to parse: %s\n", strerror(-r)); 
  }
  if (what < 0 || what >= RNO_G_NOT_A_VAL) 
  {
    sd_bus_error_set_const(ret_error,"org.rno-g.UnknownKey", "Unknown key!"); 
    return -EINVAL; 
  }

  r = rno_g_get_val(mcu, what, &val); 

  if (r < 0) 
  {
    fprintf(stderr,"rno_g_get_val returned %d\n", r); 
    sd_bus_error_set_const(ret_error,"org.rno-g.GetValFail", "Set Value Failed!"); 
    return -EIO; 
  }

  return sd_bus_reply_method_return(m, "i", val); 
}


static int method_monitor(sd_bus_message  *m, void *udata, sd_bus_error * ret_error) 
{
  int navg; 
  int r = sd_bus_message_read(m,"i",&navg); 
  if (r < 0) 
  {
    fprintf(stderr,"Failed to parse: %s\n", strerror(-r)); 
  }
  rno_g_report_t res; 
  int r = rno_g_monitor(mcu, navg, &res); 

  if (r < 0) 
  {
     fprintf(stderr,"rno_g_monitor returned %d\n", r); 
     sd_bus_error_set_const(ret_error,"org.rno-g.MonitorFail", "Monitor Failed!"); 
     return -EIO; 
  }
 
  sd_bus_message * reply; 
  r = sd_bus_new_method_return(m, &reply); 
  if (r < 0) 
  {
    fprintf(stderr, "Error in sd_bus_new_method_return: %s\n", strerror(-r)); 
    return -EIO; 
  }

  r = sd_bus_message_append_array(reply, 'y', &res, sizeof(res)); 
  if (r < 0) 
  {
    fprintf(Stderr, "Error in sd_bus_message_append_array: %s\n", strerror(-r)); 
    return -EIO; 
  }

  return sd_bus_message_send(reply); 
}


static int method_monitor_json(sd_bus_message  *m, void *udata, sd_bus_error * ret_error) 
{
  int navg; 
  int r = sd_bus_message_read(m,"i",&navg); 
  if (r < 0) 
  {
    fprintf(stderr,"Failed to parse: %s\n", strerror(-r)); 
  }
  char * json = 0; 
  int r = rno_g_monitor_json(mcu, navg, &json); 

  if (r < 0) 
  {
     fprintf(stderr,"rno_g_monitor_json returned %d\n", r); 
     sd_bus_error_set_const(ret_error,"org.rno-g.MonitorJSONFail", "Monitor Failed!"); 
     return -EIO; 
  }
 
  return sd_bus_reply_method_return(m, "s", json); 
}

static int method_reset(sd_bus_message  *m, void *udata, sd_bus_error * ret_error) 
{
  int to; 
  int r = sd_bus_message_read(m,"i",&to); 
  if (r < 0) 
  {
    fprintf(stderr,"Failed to parse: %s\n", strerror(-r)); 
  }
  int r = rno_g_reset(mcu, to); 

  if (r < 0) 
  {
     fprintf(stderr,"rno_g_reset_ returned %d\n", r); 
     sd_bus_error_set_const(ret_error,"org.rno-g.MonitorResetFail", "Reset Failed!"); 
     return -EIO; 
  }
 
  return sd_bus_reply_method_return(m, "b", !!r); 
}

static int method_program_flash(sd_bus_message * m, void *udata, sd_bus_error * ret_error) 
{

  int slot, fd; 
  int r = sd_bus_message_read(m,"ih", &slot, &fd); 
  if (r < 0) 
  {
    fprintf(stderr,"Failed to parse: %s\n", strerror(-r)); 
  }
  if (slot < 1 || slot > 4) 
  {
    sd_bus_error_set_const(ret_error,"org.rno-g.BadSlot", "Bad Slot!"); 
    return -EINVAL; 
  }

  FILE * f = fdopen(dup(fd), "r"); 

  fseek(f, 0, SEEK_END); 
  int sz = ftell(FILE*); 
  fseek(f, 0, SEEK_SET); 

  //Load the entire program into memory...  
  uint8_t * buf = calloc(sz); 
  fread(buf, sz,1,f); 
  fclose(f); 


  r = rno_g_program_flash(mcu, slot, sz, buf); 
  free(buf); 
  if (r < sz) 
  {
      fprintf(stderr,"program failed: %d\n", r); 
      sd_bus_error_set_const(ret_error,"org.rno-g.ProgramFail","Programming Failed"); 
      return -EIO: 
  }

  return sd_bus_reply_method_return(m, "i", sz); 
}


static int method_read_flash(sd_bus_message * m, void *udata, sd_bus_error * ret_error) 
{

  int slot, fd; 
  int r = sd_bus_message_read(m,"i", &slot); 
  if (r < 0) 
  {
    fprintf(stderr,"Failed to parse: %s\n", strerror(-r)); 
  }
  if (slot < 1 || slot > 4) 
  {
    sd_bus_error_set_const(ret_error,"org.rno-g.BadSlot", "Bad Slot!"); 
    return -EINVAL; 
  }

  fd = memfd_create("rno-g-flash-read", MFD_ALLOW_SEALING); 

  if (fd < 0) 
  {
    sd_bus_error_set_const(ret_error,"org.rno-g.MemFDFailed", "memfd_create failed!"); 
    return errno; 
  }

  int offs = 0;
  while (true)
  {
    char buf[128]; 
    r = rno_g_read_flash(mcu, slot, offs, 128, buf); 
    if( r > 0)
    {
      offs+= r; 
      write(fd, buf, r); 
    }
    else if (r < 0) 
    {
      close(fd); 
      fprintf(stderr,"rno_g_read_flash  failed: %d\n", r); 
      sd_bus_error_set_const(ret_error,"org.rno-g.ReadFail","Reading Failed Failed"); 
      return -EIO: 
    }
  }

  //seal the program?
  fcntl(fd , F_ADD_SEALS, F_SEAL_WRITE); 

  return sd_bus_reply_method_return(m, "h", fd); 
}



const sd_bus_vtable controller_vtable[] = 
{
  SD_BUS_VTABLE_START(0), 
  SD_BUS_METHOD("SetVal","ii","b", method_setval),
  SD_BUS_METHOD("GetVal","i","i", method_getval),
  SD_BUS_METHOD("Monitor","i","ay", method_monitor),
  SD_BUS_METHOD("MonitorJSON","i","s", method_monitor_json),
  SD_BUS_METHOD("Reset","i","b", method_reset),
  SD_BUS_METHOD("ProgramFlash","ih","i", method_program_flash),
  SD_BUS_METHOD("ReadFlash","i","h", method_read_flash),
  SD_BUS_VTABLE_END
}; 


int main(int nargs, char ** args) 
{
  int user = 0; 
  sd_bus_slot * slot = NULL; 
  sd_bus * bus = NULL; 
  sd_bus_error_e =  SD_BUS_ERROR_NULL; 
  int r; 
  int i; 
  const char * dev = NULL; 

  if (nargs < 2) 
  {
    usage(); 
    return 1; 
  }

  for (i = 1; i < nargs; i++) 
  {
    if (!strcmp(args[i],"--user")); 
    {
      user = 1;  
      printf("Using user bus!\n"); 
    }
    else 
    {
      if (dev!=NULL) 
      {
        usage(); 
        return EXIT_FAILURE; 
      }

      dev = args[i]; 

    }
  }

  if (dev == NULL) 
  {
    usage(); 
    return EXIT_FAILURE; 
  }


  //Try to open MCU 
  mcu = rno_g_controller_open_raw(dev); 
  if (mcu == NULL) 
  {
    fprintf(stderr,"Could not open device at %s\n", dev); 
    return EXIT_FAILURE; 
  }


  r = user ? sd_bus_open_user(&bus) : sd_bus_open_system(&bus); 

  if (r < 0) 
  {
    fprintf(stderr, "Failed to connect to %s bus: %s\n", user ? "user" : "system", strerror(-r)); 
    goto finish; 
  }


  r = sd_bus_add_object(bus, &slot, RNO_G_DBUS_OBJECT_PATH, 
                        RNO_G_DBUS_INTERFACE_NAME, 
                        controller_vtable, NULL); 

  if (r < 0) 
  {
    fprintf(stderr, "sd_bus_add_object failed: %s\n", strerror(-r)); 

  }

  // Claim service name 

  r = sd_bus_request_name(RNO_G_DBUS_SERVICE); 

  while(!time_to_quit) 
  {
    r = sd_bus_process(bus,NULL); 

    if (r < 0) 
    {
      fprintf(stderr,"Failed to process bus: %s\n", strerror(-r)); 
      goto finish; 
    }

    if (r > 0) 
    {
      continue;  //we did something, let's see if there's something else to do 
    }

    r = sd_bus_wait(bus, (uint64_t) -1); 
    if (r < 0) 
    {
      fprintf(stderr,"Failed to wait on bus: %s\n",strerror(-r)); 
    }

  }


finish: 
  if (slot) sd_bus_slot_unref(slot);
  if (bus) sd_bus_unref(bus); 
  if (mcu) rno_g_controller_close(dev); 

  return r < 0 ? EXIT_FAILURE : EXIT_SUCCESS; 

}

