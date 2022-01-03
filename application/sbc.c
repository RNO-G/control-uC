#include "application/sbc.h" 
#include "application/gpio_expander.h" 
#include "application/lte.h" 
#include "shared/driver_init.h" 
#include "shared/io.h" 
#include "application/time.h" 
#include "application/reset.h" 
#include "shared/spi_flash.h" 
#include "hal_calendar.h" 
#include "shared/printf.h" 
#include "shared/base64.h" 
#include "include/rno-g-control.h" 
#include "hal_flash.h"
#include "application/i2cbus.h" 
#include "shared/programmer.h" 
#include "linker/map.h"
#include "application/mode.h" 
#include "application/power.h" 
#include "application/monitors.h" 
#include "application/report.h" 
#include "lorawan/lorawan.h" 

//these define the maximum line length! 
#define SBC_BUF_LEN 128 
#define SBC_CONSOLE_BUF_LEN 128

ASYNC_TOKENIZED_BUFFER(SBC_BUF_LEN, sbc,"\r\n", SBC_UART_DESC); 

#ifndef _DEVBOARD_
ASYNC_TOKENIZED_BUFFER(SBC_CONSOLE_BUF_LEN, sbc_console,"\r\n", SBC_UART_CONSOLE_DESC); 
#endif

#define SBC_CURRENT_THRESH 100 

static sbc_state_t the_sbc_state; 
static int noff_tries = 0; //give it up to 20 secs to turn off, I guess? 
static int up_turning_off = 0; 
static int up_turning_on = 0; 
static int ready_to_turn_off = 0; 


static int16_t sbc_V() 
{
  monitor_select(MON_SBC_5V); 
  delay_us(10000); 
  return monitor(MON_SBC_5V,50); 
}

void sbc_init()
{
  i2c_gpio_expander_t i2c_gpio; 
  get_gpio_expander_state(&i2c_gpio,1); //this must be called after i2cbus_init, so we'll have a value; 
  

  //current is off but power is on, so we're probably out of sync... 
  if (sbc_V() < SBC_CURRENT_THRESH && i2c_gpio.sbc)  
  {
    i2c_gpio.sbc=0;
    i2c_gpio_expander_t i2c_mask = {.sbc = 1} ; 
    set_gpio_expander_state(i2c_gpio,i2c_mask); 
  }

  the_sbc_state = i2c_gpio.sbc ? SBC_ON : SBC_OFF; 

  if (the_sbc_state == SBC_ON) sbc_io_init(); 
}


static inline int abs(int v) 
{
  return v < 0 ? -v : v; 

}

static void do_release_boot(const struct timer_task * const task)
{
    (void) task; 
    gpio_set_pin_direction(SBC_BOOT_SDCARD, GPIO_DIRECTION_IN); 
}

static struct timer_task sbc_release_boot_task = { .cb  = do_release_boot, .interval = 500, .mode = TIMER_TASK_ONE_SHOT }; 


static void do_turn_on(int release_boot)
{
  i2c_gpio_expander_t turn_on_sbc = {.sbc=1}; 
  set_gpio_expander_state (turn_on_sbc,turn_on_sbc); 
  the_sbc_state = SBC_ON; 

  //this means we have to release the boot select switch 
  if (release_boot) 
  {
    timer_add_task(&SHARED_TIMER, &sbc_release_boot_task);
  }
}




int sbc_turn_on(sbc_boot_mode_t boot_mode) 
{
  if (the_sbc_state != SBC_OFF) return -1;  

  sbc_io_init(); 

  if (boot_mode == SBC_BOOT_SDCARD) 
  {
    gpio_set_pin_direction(SBC_BOOT_SDCARD, GPIO_DIRECTION_OUT); 
    gpio_set_pin_level(SBC_BOOT_SDCARD, 0); 
    the_sbc_state = SBC_TURNING_ON; 
    up_turning_on = uptime(); 
  }
  else
  {
    do_turn_on(0); 
  }
  return 0; 
}






static int sbc_io_process()
{
  if (the_sbc_state == SBC_OFF) return 0; 


    // SBC Commands
    //
    //  Commands start with a # and end with \r\n 
    //  Must not contain \r\n or \0. 
    //
    //  This might be torn out and replaced with something better later 
    //
    //  Note that this in addition to the "normal" command processing, that can 
    //  come via LoRaWAN and eventually LTE. 


    //find the next \r\n: 
    
    int dontconsume = 0; 
    int nvalid = 0; 
    int need_sync = 0; 

    while(async_tokenized_buffer_ready(&sbc)) 
    {
      int valid = 0; 


      char * in = (char*) sbc.buf+1; 
      //we don't start with a #. Skip to end
      if (sbc.buf[0] !='#')
      {

      }
      //check if it's a programmer command
      else if (programmer_check_command((char*) sbc.buf) && !d_check (SBC_UART_DESC,5))
      {
        // don't echo out negative return values, since those might have corrupted sbc.buf 
         valid = programmer_cmd((char*) sbc.buf, sbc.len) <=0; 
      }

      //now let's try to match commands 
      //First commands with no arguments
      else if (!strcmp(in, "LTE-ON"))
      {
        valid=1; 
        if (mode_query()!=RNO_G_NORMAL_MODE)
        {
          printf("#LTE-ON: MODE IS OFF \r\n"); 
        }
        else
        {
          lte_turn_on(0); 
          printf("#LTE-ON: ACK \r\n"); 
        }
      }
      else if (!strcmp(in, "LTE-ON!"))
      {
        valid=1; 
        if (mode_query()!=RNO_G_NORMAL_MODE)
        {
          printf("#LTE-ON!: MODE IS OFF \r\n"); 
        }
        else {
          lte_turn_on(1); 
          printf("#LTE-ON!: ACK \r\n"); 
        }
      }
 
      else if (!strcmp(in,"LTE-OFF"))
      {
        valid =1; 
        if (mode_query()!=RNO_G_NORMAL_MODE)
        {
          printf("#LTE-OFF: MODE IS OFF \r\n"); 
        }
        else {
          lte_turn_off(0); 
          printf("#LTE-OFF: ACK\r\n"); 
        }
      }
      else if (!strcmp(in,"LTE-OFF!"))
      {
        valid =1; 

        if (mode_query()!=RNO_G_NORMAL_MODE)
        {
          printf("#LTE-OFF!: MODE IS OFF \r\n"); 
        }
        else
        {
          lte_turn_off(1); 
          printf("#LTE-OFF!: ACK\r\n"); 
        }
      }
      else if (!strcmp(in,"LTE-STATE"))
      {
        valid =1 ; 
        printf("#LTE-STATE: %d\r\n",lte_get_state()); 
      }
      else if (!strcmp(in,"LTE-STATS"))
      {
        valid =1 ; 
        const rno_g_lte_stats_t * st = lte_get_stats(); 
        printf("#LTE-STATS: { when:%d, mcc: %hd mnc: %hd, earcfn: %hu, \r\n",st->when, st->mcc, st->mnc, st->earfcn); 
        printf("              rsrp:%hhd, rssi: %hhd, rsrqx10: -%hhu, \r\n", st->rsrp, st->rssi, st->neg_rsrq_x10); 
        printf("              band: %hhu, svcdom: %hhd, prseok: %hhd } \r\n", st->band, st->service_domain, st->parsed_ok); 
      }
 
      else if (!strcmp(in,"RADIANT-ON"))
      {
        valid=1; 
        i2c_gpio_expander_t turn_on_radiant = {.radiant = 1}; 
        set_gpio_expander_state(turn_on_radiant, turn_on_radiant); 
        printf("#RADIANT-ON: ACK\r\n"); 
      }

      else if (!strcmp(in,"RADIANT-OFF") )
      {
        valid=1;
        i2c_gpio_expander_t turn_off_radiant = {0}; 
        i2c_gpio_expander_t turn_off_radiant_mask = {.radiant = 1}; 
        set_gpio_expander_state(turn_off_radiant, turn_off_radiant_mask); 
        printf("#RADIANT-OFF: ACK\r\n"); 
      }
      else if (!strcmp(in,"LOWTHRESH-ON"))
      {
        valid=1; 
        i2c_gpio_expander_t turn_on_lt = {.lt = 1}; 
        set_gpio_expander_state(turn_on_lt, turn_on_lt); 
        printf("#LOWTHRESH-ON: ACK\r\n"); 
      }

      else if (!strcmp(in,"LOWTHRESH-OFF") )
      {
        valid=1;
        i2c_gpio_expander_t turn_off_lt = {0}; 
        i2c_gpio_expander_t turn_off_lt_mask = {.lt = 1}; 
        set_gpio_expander_state(turn_off_lt, turn_off_lt_mask); 
        printf("#LOWTHRESH-OFF: ACK\r\n"); 
      }
 
      else if (prefix_matches(in,"AMPS-SET"))
      {
        uint8_t surf, dh; 
        const char * nxt=0;
        valid =1; 
        if (parse_hex(in+sizeof("AMPS-SET"),&nxt, &surf) ||
            parse_hex(nxt,&nxt,&dh) || (surf >> 6) || (dh >> 3) )
        {
          printf("#ERR: Failed to interpret #%s\r\n",in); 
        }
        else
        {
          i2c_gpio_expander_t set = {.surface_amps = surf, .dh_amps = dh};
          i2c_gpio_expander_t mask = {.surface_amps = 0x3f, .dh_amps = 0x7};
          set_gpio_expander_state(set,mask); 
          printf("#AMPS-SET: %x %x\r\n", surf, dh); 
        }
      }
      else if (!strcmp(in,"EXPANDER-STATE"))
      {
        int force  = 0; 
        parse_int(in + sizeof("EXPANDER-STATE"),0,&force); 
        valid =1; 
        i2c_gpio_expander_t exp_state; 
        get_gpio_expander_state(&exp_state,!force); 
        printf("#EXPANDER-STATE: surf: %x, dh: %x, radiant: %x, lt: %x, sbc: %x\r\n", exp_state.surface_amps, exp_state.dh_amps, 
            exp_state.radiant, exp_state.lt, exp_state.sbc); 
      }
      else if (!strcmp(in,"FAULT-STATE"))
      {
        valid =1; 
        i2c_gpio_expander_t exp_faults; 
        get_gpio_expander_fault_state(&exp_faults); 
        printf("#FAULT-STATE: surf: %x, dh: %x, radiant: %x, lt: %x, sbc: %x\r\n", exp_faults.surface_amps, exp_faults.dh_amps, 
            exp_faults.radiant, exp_faults.lt, exp_faults.sbc); 
      }
 
      else if (!strcmp(in,"MONITOR"))
      {
#ifdef _RNO_G_REV_D
        const rno_g_report_t *report = report_get(); 
        rno_g_monitor_t mon = report->analog_monitor;
        rno_g_power_system_monitor_t pwr = report->power_monitor;
        rno_g_power_state_t st = report->power_state; 
        printf("#MONITOR: analog: { when: %u, temp: %d.%02u C, i_surf3V: [%hu,%hu,%hu,%hu,%hu,%hu] mA, i_down3v: [%hu,%hu,%hu] mA, i_sbc5v: %hu, i_radiant: %hu mA, i_lt: %hu mA}\r\n", 
            mon.when, mon.temp_cC/100, abs(mon.temp_cC) % 100, mon.i_surf3v[0],  mon.i_surf3v[1],  mon.i_surf3v[2], mon.i_surf3v[3],  mon.i_surf3v[4],  mon.i_surf3v[5], 
            mon.i_down3v[0], mon.i_down3v[1], mon.i_down3v[2], mon.i_sbc5v, mon.i_5v[0], mon.i_5v[1]); 
        printf("#MONITOR: power: { when: %u, PV_V: %d.%02u V, PV_I: %d mA, BAT_V: %d.%02d V, BAT_I: %d mA}\r\n", 
                pwr.when_power,
                pwr.PVv_cV/100, pwr.PVv_cV % 100, pwr.PVi_mA, 
                pwr.BATv_cV/100, pwr.BATv_cV % 100, pwr.BATi_mA) ;

        const char* sixteenths[] = {"0", "0625","125","1875","25","3125","375","4375","5","5625","625","6875","75","8125","875","9375"}; 

         printf("#MONITOR: temp: { when: %u, local: %d.%s C, remote1: %d.%s C, remote2: %d.%s} \r\n",
                pwr.when_temp, 
                pwr.local_T_C, sixteenths[pwr.local_T_sixteenth_C], 
                pwr.remote1_T_C, sixteenths[pwr.remote1_T_sixteenth_C], 
                pwr.remote2_T_C, sixteenths[pwr.remote2_T_sixteenth_C]);  
         printf("#MONITOR: power_state: { low_power: %d, sbc_power: %d, lte_power: %d, radiant_power: %d, lowthresh_power: %d, dh_amp_power: %x, surf_amp_power: %x}\r\n", 
                 st.low_power_mode, st.sbc_power, st.lte_power, st.radiant_power, st.lowthresh_power, st.dh_amp_power, st.surf_amp_power); 

#else
       const rno_g_report_v2_t * report = report_get(); 
       printf(RNO_G_REPORT_V2_JSON_FMT "\r\n", RNO_G_REPORT_V2_JSON_VALS(report)); 
#endif
         valid=1; 
      }
      else if (!strcmp(in,"B64MON"))
      {
        const RNO_G_REPORT_T *report = report_get(); 
        printf("#B64MON: "); 
        base64_print(SBC_UART_DESC, sizeof(RNO_G_REPORT_T), (uint8_t*) report); 
        printf("\r\n"); 
        valid=1; 
      }
      else if (prefix_matches(in,"SET-BATT-MILLIVS"))
      {
        valid =1;  
        int turnon, turnoff; 
        const char * nxt = 0; 

        if (parse_int(in+sizeof("SET-BATT-MILLIVS"), &nxt,&turnoff) ||
            parse_int(nxt,&nxt,&turnon)) 
        {
          printf("#ERR: Failed to interpret:%s\r\n", in); 
        }
        else
        {
          if (turnon > 100) //make sure reasonable in V
          {
            float f = turnon/1000. ; 

            if (f!=config_block()->app_cfg.turnon_voltage)
            {
              config_block()->app_cfg.turnon_voltage = f; 
              need_sync = 1; 
            }
          }
          if (turnoff > 100) //make sure reasonable in V
          {
            float f = turnoff/1000. ; 

            if (f!=config_block()->app_cfg.turnoff_voltage)
            {
              config_block()->app_cfg.turnoff_voltage = f; 
              need_sync = 1; 
            }
          }
          printf("#SET-BATT-MILLIVS: %d %d\r\n", turnoff, turnon); 
        }

      }
      else if (!strcmp(in,"GET-BATT-MILLIVS"))
      {
        valid = 1; 
        int turnoff = config_block()->app_cfg.turnoff_voltage *1000;
        int turnon = config_block()->app_cfg.turnon_voltage *1000;
        printf("#GET-BATT-MILLIVS: %d %d\r\n", turnoff, turnon); 

      }
      else if (!strcmp(in,"MODE-GET"))
      {
        valid = 1; 
        printf("#MODE-GET: %d\r\n", mode_query()); 
      }
      else if (prefix_matches(in,"MODE-SET"))
      {
        int tomode = 0; 
        parse_int(in + sizeof("MODE-SET"),0,&tomode); 
        if (tomode > 0 && tomode < RNO_G_NOT_A_MODE) 
        {
          printf("MODE-SET: %d\r\n", tomode); 
          mode_set(tomode); 
        }
        else
        {
          printf("#ERR: Invalid mode\r\n"); 
        }
        valid = 1; 
      }
      else if (prefix_matches(in,"MONITOR-SCHED"))
      {
        int navg = 10; 
        parse_int(in + sizeof("MONITOR-SCHED"),0,&navg); 
        printf("#MONITOR-SCHED %d\r\n",navg); 
        report_schedule(navg); 
        valid=1; 
      }
      else if (prefix_matches(in,"I2C-WRITE"))
      {
        static i2c_task_t write_task  = {.addr=0,.write=1,.reg=0,.data=0,.done=1};

        //check if we are ready, otherwise break out of this loop 
        if (!write_task.done) 
        {
          dontconsume=1; 
          valid =1; 
          break;
        }

        const char * nxt = 0; 
        uint8_t addr; 
        uint8_t reg; 
        uint8_t data; 
        valid=1; 
        if (parse_hex(in+sizeof("I2C-WRITE"),&nxt, &addr) ||
            parse_hex(nxt,&nxt,&reg) ||
            parse_hex(nxt,&nxt,&data))
        {
          printf("#ERR: Failed to interpret #%s\r\n",in); 
        }
        else
        {
          write_task.addr=addr;
          write_task.reg=reg;
          write_task.data=data;
          i2c_enqueue(&write_task); 
          printf("#I2C_WRITE( val=0x%x,reg=0x%x,addr=0x%x)\r\n", data,reg,addr); 
        }
      }
      else if (prefix_matches(in,"I2C-READ"))
      {
        const char *nxt = 0; 
        uint8_t addr;
        uint8_t reg;
        if (parse_hex(in+sizeof("I2C-READ"),&nxt, &addr) ||
            parse_hex(nxt,&nxt,&reg) )
        {
          printf("#ERR: Failed to interpret #%s\r\n",in);
          valid=1; 
        }
        else
        {
          i2c_task_t read_task = {.addr=addr, .write=0, .reg=reg}; 
          i2c_enqueue(&read_task); 
          while (!read_task.done); //busy wait 
          printf("#I2C-READ: addr(%x) reg(%x) = %x\r\n", read_task.addr, read_task.reg, read_task.data); 
          valid=3; //only do one of these tasks per process, since they can take a while 
        }
      }
      else if (!strcmp(in,"I2C-DETECT"))
      {
        i2c_detect(0,127,0); 
        valid=3; 
      }

      else if (prefix_matches(in,"I2C-UNSTICK"))
      {
        const char *nxt = 0; 
        int howmany = 9;
        if (parse_int(in + sizeof("I2C-UNSTICK"), &nxt, &howmany)) howmany=9 ;
        int nones = i2c_unstick(howmany); 
        valid=3; 
        printf("#I2C-UNSTICK(%d): %d\r\n", howmany, nones); 
      }

      else if (prefix_matches(in,"SYS-RESET"))
      {
        const char *nxt = 0; 
        int opt = 0;
        if (parse_int(in + sizeof("SYS-RESET"), &nxt, &opt)) opt=0 ;
        valid=3; 
        printf("#SYS-RESET(%d)!!\r\n", opt); 
        delay_ms(10); 
        reset((boot_option_t) opt); 
      }


      else if (prefix_matches(in,"SET-STATION"))
      {
        const char * nxt = 0; 
        int station; 
        if (!parse_int(in+sizeof("SET-STATION"), &nxt, &station))
        {
          config_block()->app_cfg.station_number= station;
          need_sync = 1; 
          lorawan_init(0);  // reinit lorawan with new id 
          printf("#SET-STATION: %d\r\n", station); 
          valid = 1; 
        }
        else
        {
          printf("#ERR: trouble parsing int"); 
        }
      }
      else if (prefix_matches(in,"SET-GPS-OFFSET"))
      {
        const char * nxt = 0; 
        int offset; 
        if (!parse_int(in+sizeof("SET-GPS-OFFSET"), &nxt, &offset))
        {
          int old_offset = config_block()->app_cfg.gps_offset; 
          if (offset != old_offset) 
          {
            if (get_time() > 150000000) set_time_with_delta(old_offset-offset); 
            config_block()->app_cfg.gps_offset= offset;
            need_sync = 1; 
          }
          printf("#SET-GPS-OFFSET: %d\r\n", offset); 
          valid = 1; 
        }
        else
        {
          printf("#ERR: trouble parsing int"); 
        }
      }
      else if (!strcmp(in,"AM-I-BOOTLOADER"))
      {
        printf("#AM-I-BOOTLOADER: 0\r\n"); 
        valid = 1; 
      }
      else if (!strcmp(in,"GET-STATION"))
      {
        printf("#GET-STATION: %d\r\n", config_block()->app_cfg.station_number); 
        valid = 1; 
      }
      else if (!strcmp(in,"FLUSH"))
      {
        flush_buffers(); 
        printf("#FLUSH: ACK\r\n"); 
        valid = 1; 
      }
      else if (prefix_matches(in,"LORA-SEND "))
      {
        //TODO
        valid = 1; 
      }
 
      else if (!strcmp(in,"NOW"))
      {
        struct calendar_date_time now; 
        calendar_get_date_time(&CALENDAR,&now); 
        valid = 1; 
        printf("#NOW: %d-%02d-%02d %02d:%02d:%02d, UPTIME: %u,  LORA: ", now.date.year, now.date.month, now.date.day, now.time.hour, now.time.min, now.time.sec, uptime());  
        if (lorawan_state()  ==LORAWAN_READY) 
        {

          rno_g_lora_stats_t stats; 
          lorawan_stats(&stats); 
          printf("tx=%d/%d, rx=%d/%d\r\n", stats.tx, stats.tx+stats.tx_dropped, stats.rx, stats.rx+stats.rx_dropped);
        }
        else
        {
          printf("JOINING\r\n"); 
        }
      }

      if (!valid) 
      {
        printf("#ERR don't understand: %s\r\n", sbc.buf); 
      }
      else 
      {
        nvalid+=valid; 
      }

      //consume 
      if (!dontconsume) 
      {
        async_tokenized_buffer_discard(&sbc); 
      }
 

     // only do up to 3 commands at a time! 
      if (dontconsume || nvalid > 3) break; 
    }

    if (need_sync) config_block_sync(); 

    return 0; 
}

int sbc_process (int up) 
{

  if (the_sbc_state == SBC_ON) 
  {
    sbc_io_process(); 
  }

  else if (the_sbc_state == SBC_TURNING_OFF) 
  {
    if (!ready_to_turn_off && noff_tries++ < 5 && up_turning_off > 0  && up > up_turning_off + 3 )
    {
      if (sbc_V() < SBC_CURRENT_THRESH)
      {
        ready_to_turn_off = 1; 
      }
      else
      {
        up_turning_off = up; 
      }
    }

    if (noff_tries >= 5 || ready_to_turn_off) 
    {

      //just in case it was pressed 
      i2c_gpio_expander_t turn_off_sbc = {.sbc=0}; 
      i2c_gpio_expander_t turn_off_mask = {.sbc=1}; 
      set_gpio_expander_state (turn_off_sbc,turn_off_mask); 
      the_sbc_state = SBC_OFF; 
      report_schedule(50); 
      noff_tries = 0; 
    }
  }
  else if (the_sbc_state == SBC_TURNING_ON && up > up_turning_on+1) 
  {
    do_turn_on(1); 
  }

  return 0; 
}


sbc_state_t sbc_get_state() { return the_sbc_state; } 


static void do_turn_off(const struct timer_task * const task)
{
  (void) task;
  gpio_set_pin_direction(SBC_SOFT_RESET, GPIO_DIRECTION_IN); 
  up_turning_off = uptime(); 
}

static struct timer_task sbc_turn_off_task = { .cb  = do_turn_off, .interval = 20, .mode = TIMER_TASK_ONE_SHOT }; 

__attribute__((section (".keepme")))
int sbc_turn_off() 
{

  if (the_sbc_state != SBC_ON) return -1; 

  sbc_io_deinit(); 
  the_sbc_state = SBC_TURNING_OFF; 


  // ONLY HIT POWER BUTTON IF CURRENT IS HIGH ENOUGH WE THINK THE SBC IS ACTUALLY ON... otherwise we'll turn it off then back on 
  up_turning_off = -1; 
  if (sbc_V() > SBC_CURRENT_THRESH) 
  {
    gpio_set_pin_level(SBC_SOFT_RESET,0); 
    gpio_set_pin_direction(SBC_SOFT_RESET, GPIO_DIRECTION_OUT); 
    ready_to_turn_off = 0; 
    noff_tries = 0; 
    timer_add_task(&SHARED_TIMER, &sbc_turn_off_task);
  }
  else
  {
    noff_tries = 0; 
    ready_to_turn_off = 1; 
    int up = uptime(); 
    sbc_process(up); //actually turn it off 
  }

  return 0; 

} 
