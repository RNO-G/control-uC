#include "rno-g-controller.h"
#include <systemd/sd-bus.h> 
#include <time.h>


struct rno_g_controller 
{
  enum {CONTROLLER_RAW, CONTROLLER_DBUS, CONTROLLER_SIMULATOR} type; 
  union { FILE *fd; sd_bus * bus, int * mem } iface; 
}; 


/** NOT THREAD SAFE OR REENTRANT
 *
 * Tries to read a line from the file descriptor. If times out, may return less than a line (you should check that it ends in \r\n) nds in \r\n) 
 * timeout= 0 means return immedialy i
 *
 * */ 

const char * raw_getline(int fd, double timeout) 
{
  struct timespec start; 
  struct timespec now; 
  if (timeout > 0) 
  {
    clock_gettime(CLOCK_MONOTONIC,&start); 
  }

  static char buf[512]; 
  static int offs = 0; 
  memset(buf,0,offs); //zero everything non-zero
  offs = 0; 

  while(true) 
  {

    int r = read(fd, buff+offs,1); 
    if (r < 0 && !(errno==EAGAIN || errno==EWOULDBLOCK || errno==EINTR)) 
    {
      return 0; 
    }

    if (r > 0) 
    {
      offs+=r; 
      if (strstr(buf,"\r\n"))
      {
        break; 
      }

      if (offs >= sizeof(buf)-1)
      {
        fprintf(stderr,"raw_getline(): LINE TOO LONG, STARTING ANEW\n"); 
        memset(buf,0,offs); 
        offs=0; 
      }
    }

    if (timeout > 0) 
    {
      clock_gettime(CLOCK_MONOTONIC,&now); 
      if (now.tv_sec-start.tv_sec+1e-9*(now.tv_nsec-start.tv_nsec) > timeout) 
    }
    if (timeout == 0 && (r<=0) break; 
  }
  return buf; 
}


rno_g_controller_t * rno_g_controller_open_raw(const char * uart)
{

  if (uart == NULL) 
  {
    fprintf(stderr,"INFO: No UART passed, using simulator\n"); 

    rno_g_controller_t  * dev= calloc(sizeof(rno_g_controller_t)); 
    dev->type = CONTROLLER_SIMULATOR; 
    dev->iface.mem = calloc(sizeof(int) * RNO_G_NOT_A_VAL);
    return dev; 
  }
  else
  {
    FILE * f= fopen(uart,"a+"); 
    if (!f) 
    {
      fprintf(stderr,"ERROR: Could not open %s\n", uart); 
      rettrn 0; 

    }
    rno_g_controller_t  * dev= calloc(sizeof(rno_g_controller_t)); 
    dev->type=CONTROLLER_RAW; 
    dev->iface.fd = f; 
    //set as non-blocking
    int fd = fileno(fserial); 
    int flags = fcntl(fd, F_GETFL,0); 
    flags |=O_NONBLOCK; 
    fcntl(fd, F_SETFL,flags); 

    return dev; 
  }
}

rno_g_controller_t * rno_g_open_dbus(int userbus) 
{

  rno_g_controller_t  * dev= calloc(sizeof(rno_g_controller_t)); 
  dev->type = CONTROLLER_DBUS; 
  int r = userbus ? sd_bus_open_user(&dev->iface.bus) : sd_bus_open_system(&dev->iface.bus); 
  if (r < 0) 
  {
    fprintf(sterr, "Failed to connect to %s bus: %s\n", userbus ? "user" : "system", strerror(-r)); 
    rno_g_controller_close(dev); 
    return 0; 
  }


  return dev; 
}


void rno_g_controller_close(rno_g_controller_t * dev) 
{
  if (!dev) return; 

  if (dev->type == CONTROLLER_SIMULATOR) 
  {
    free(dev->iface.mem); 
  }

  else if (dev->type = CONTROLLER_RAW) 
  {
    fclose(dev->iface.fd); 
  }

  else if (dev->type == CONTROLLER_DBUS) 
  {
    sd_bus_unref(dev->iface.bus); 
  }

  free(dev); 
}



static int dbus_get(sd_bus * bus, rno_g_controller_key_t what, int *val) 
{

  sd_bus_message *m =NULL;
  sd_bus_error error = SD_BUS_ERROR_NULL; 
  int r = sd_bus_call_method(bus, RNO_G_DBUS_SERVICE, RNO_G_DBUS_OBJECT_PATH, 
                                  RNO_G_DBUS_INTERFACE_NAME,"GetVal", &error, &m, "i", what); 

  if (r < 0) 
  {
    fprintf(stderr,"Failed to issue dbus method call: %s\n", error.message); 
  }
  else
  {
    r = sd_bus_message_read(m,"i", val); 
    if (r <  0) 
    {
      fprintf(stderr,"Failed to parse response message: %s\n", strerror(-r));
    }

  }

  sd_bus_error_free(&error); 
  sd_bus_message_unref(m);
  return r; 
}

static int dbus_set(sd_bus * bus, rno_g_controller_key_t what, int val) 
{

  sd_bus_message *m =NULL;
  sd_bus_error error = SD_BUS_ERROR_NULL; 
  int r = sd_bus_call_method(bus, RNO_G_DBUS_SERVICE, RNO_G_DBUS_OBJECT_PATH, 
                                  RNO_G_DBUS_INTERFACE_NAME,"SetVal", 
                                  &error, &m, "ii", what, val); 

  if (r < 0) 
  {
    fprintf(stderr,"Failed to issue dbus method call: %s\n", error.message); 
  }
  else
  {
    int ok; 
    r = sd_bus_message_read(m,"b", &ok); 
    if (r <  0) 
    {
      fprintf(stderr,"Failed to parse response message: %s\n", strerror(-r));
    }
  }

  sd_bus_error_free(&error); 
  sd_bus_message_unref(m);
  return r < 0 ? r : ok; 
}




static int raw_get(int fd, rno_g_controller_key_t what, int *val) 
{
  switch (what)
  {

    case(


  }

}

static int raw_set(int fd, rno_g_controller_key_t what, int val) 
{

}


static int simulator_get(int * mem, rno_g_controller_t what, int * val) 
{
  *val = mem[what]; 
   return 0; 
}

static int simulator_set(int * mem, rno_g_controller_t what, int val) 
{
   mem[what] = val; 
   return 0; 
}



int rno_g_get_val(rno_g_controller * dev, rno_g_controller_key_t what, int * val) 
{

  if (what < 0 || what >= RNO_G_NOT_A_VAL) return -1; 

  switch (dev->type) 
  {
    case CONTROLLER_RAW: 
      return raw_get (dev->iface.fd, what, val); 
    case CONTROLLER_DBUS: 
      return dbus_get (dev->iface.bus, what, val) 
    case CONTROLLER_SIMULATOR: 
        return simulator_get(dev->iface.mem, what,val); 
    default:
        return -1; 
  }
}

int rno_g_set_val(rno_g_controller * dev, rno_g_controller_key_t what, int  val) 
{

  if (what < 0 || what >= RNO_G_NOT_A_VAL) return -1; 

  switch (dev->type) 
  {
    case CONTROLLER_RAW: 
      return raw_set (dev->iface.fd, what, val); 
    case CONTROLLER_DBUS: 
      return dbus_set (dev->iface.bus, what, val) 
    case CONTROLLER_SIMULATOR: 
        return simulator_set(dev->iface.mem, what,val); 
    default:
        return -1; 
  }
}

static int raw_reset(int fd, int to) 
{
  if (to >=0) 
    dprintf(fd, "#SYS-RESET %d\r\n", to); 
  else
    dprintf(fd, "#SYS-RESET\r\n"); 

  const char * line = raw_getline(fd); 

  return  strstr(line,"#SYS-RESET")!=line;
}

static int dbus_reset(sd_bus * bus, int to) 
{

  sd_bus_message *m =NULL;
  sd_bus_error error = SD_BUS_ERROR_NULL; 
  int r = sd_bus_call_method(bus, RNO_G_DBUS_SERVICE, RNO_G_DBUS_OBJECT_PATH, 
                                  RNO_G_DBUS_INTERFACE_NAME,"Reset", 
                                  &error, &m, "i", to); 
  if (r < 0) 
  {
    fprintf(stderr,"Failed to issue dbus method call: %s\n", error.message); 
  }
  else
  {
    int ok; 
    r = sd_bus_message_read(m,"b", &ok); 
    if (r <  0) 
    {
      fprintf(stderr,"Failed to parse response message: %s\n", strerror(-r));
    }
  }

  sd_bus_error_free(&error); 
  sd_bus_message_unref(m);
  return r < 0 ? r : ok; 
}




int rno_g_reset(rno_g_controller *dev, int to) 
{
  switch (dev->type) 
  {
    case CONTROLLER_RAW: 
      return raw_reset(dev->iface.fd, to);
    case CONTROLLER_DBUS: 
      return dbus_reset(dev->iface.bus, to); 
    case CONTROLLER_SIMULATOR: 
      return 0; 
    default: 
      return -1; 
  }
}
static int dbus_mon(sd_bus * bus, int navg, rno_g_report_t * report) 
{

  sd_bus_message *m =NULL;
  sd_bus_error error = SD_BUS_ERROR_NULL; 
  int r = sd_bus_call_method(bus, RNO_G_DBUS_SERVICE, RNO_G_DBUS_OBJECT_PATH, 
                                  RNO_G_DBUS_INTERFACE_NAME,"Monitor", 
                                  &error, &m, "i", navg); 

  int ok = 0; 

  if (r < 0) 
  {
    fprintf(stderr,"Failed to issue dbus method call: %s\n", error.message); 
  }
  else
  {
    size_t sz; 
    void *ptr = 0; 
    r = sd_bus_message_read_array(m,'y', &ptr,&sz); 
    if (r <  0) 
    {
      fprintf(stderr,"Failed to parse response message: %s\n", strerror(-r));
    }

    if (sz!=sizeof(rno_g_report_t))
    {
      fprintf(stderr,"Got wrong size in dbus_mon, expected: %zu, got: %zu\n", sizeof(rno_g_report_t), sz); 
      ok = 1; 
    }
    else
    {
      memcpy(report, ptr, sz); 
    }
  }

  sd_bus_error_free(&error); 
  sd_bus_message_unref(m);
  return r < 0 ? r : ok; 
}




int rno_g_monitor(rno_g_controller *dev, int navg, rno_g_report_t * result) 
{
  switch (dev->type) 
  {
    case CONTROLLER_RAW: 
      return raw_mon(dev->iface.fd, navg, result);
    case CONTROLLER_DBUS: 
      return dbus_mon(dev->iface.bus, navg, result); 
    case CONTROLLER_SIMULATOR: 
      return simulator_mon(dev->iface.mem, navg, result); 
    default: 
      return -1; 
  }
}

int rno_g_monitor_json(rno_g_controller *dev, int navg, char ** result) 
{
  rno_g_report_t report; 
  int r = rno_g_monitor(dev,navg,&report); 

  if (r < 0) return r; 

  return asprintf(result, RNO_G_REPORT_JSON_FMT, RNO_G_REPORT_JSON_VALS(report)); 
}



