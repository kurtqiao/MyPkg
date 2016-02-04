/*
  pwcycle utility
  build
  DOS version: wcl -bt=dos pwcycle.c
  UEFI version: add pwcycle.inf into MyPkg.dsc 

  usage: pwcycle -s -t 10 -w 10
  -s    shutdown
  -r    reboot
  -t xx timeout xx seconds
  -w xx wakeup after xx secnods

  only support on intel platform, which assume PMBase address is 0x1800
*/

#ifdef EFI_SPECIFICATION_VERSION
#undef DOS
#pragma message ( "define uefi" ) 
#else
#define DOS
#pragma message ( "define dos" ) 
#endif

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <unistd.h>

#ifdef DOS
#include <libgen.h>
#else
#include <Library/IoLib.h>
#endif

typedef unsigned char         u8;
typedef unsigned short        u16;
typedef unsigned int          u32;
typedef unsigned long long    u64;

#define PMBASE  0x1800

#define BCD_TO_BIN(val) (((val)&15) + ((val)>>4)*10)
#define BIN_TO_BCD(val) ((((val)/10)<<4) + (val)%10)


enum{
  RTC_SECONDS = 0x00,
  RTC_SECONDS_ALARM,
  RTC_MINUTES,
  RTC_MINUTES_ALARM,
  RTC_HOURS,
  RTC_HOURS_ALARM,
  RTC_DAY_OF_WEEK,
  RTC_DAY_OF_MONTH,
  RTC_MONTH,
  RTC_YEAR,
  RTC_STS_REG_A,
  RTC_STS_REG_B,
  RTC_STS_REG_C,
  RTC_STS_REG_D,
  CMOS_INDX = 0x70,
  CMOS_DATA = 0x71
};

#ifdef DOS
enum reg_bits
{
  BIT0 = 1 << 0,
  BIT1 = 1 << 1,
  BIT2 = 1 << 2,
  BIT3 = 1 << 3,
  BIT4 = 1 << 4,
  BIT5 = 1 << 5,
  BIT6 = 1 << 6,
  BIT7 = 1 << 7
};
#endif

#define S5  BIT2|BIT3|BIT4

enum
{
  SHUTDOWN = BIT0,
  REBOOT   = BIT1,
  SLP_EN   = BIT5,
  WAKEUP   = BIT7
};

typedef struct _action
{
  unsigned char help :1;
  unsigned char shutdown :1;
  unsigned char reboot :1;
  unsigned char wakeup :1;
  unsigned char error :1;
}ACTION;

typedef struct rtc_time
{
  u8 tm_sec;
  u8 tm_sec_alarm;
  u8 tm_min;
  u8 tm_min_alarm;
  u8 tm_hour;
  u8 tm_hour_alram;
  u8 tm_day_of_week;
  u8 tm_day_of_month;
  u8 tm_month;
  u8 tm_year;
  u8 tm_sts_reg_a;
  u8 tm_sts_reg_b;
  u8 tm_sts_reg_c;
  u8 tm_sts_reg_d;
}RTC_TIME;

typedef struct _rtc_device
{
  RTC_TIME rtc_time;
  void (*get_time)(RTC_TIME *);
  void (*set_alarm)(RTC_TIME *);
  void (*calc_alarm)(RTC_TIME *, u8 waketime);
}RTC_DEVICE;

#ifdef DOS
u8 in_byte(u16 index)
{
  u8 data=0;
  _asm
  {
    xor dx, dx
    mov dx, index
    in al, dx
    mov data, al
  };
  return data;
}

void
set_int_flag(int option)
{
  if (option)
    _asm { sti };
  else
    _asm { cli };
}
void
out_byte(u16 port, u8 data)
{
  _asm
  {
    xor dx, dx
    mov al, data
    mov dx, port
    out dx, al
  };
}

#else

unsigned char
in_byte(int index)
{
  return IoRead8(index);
}

void
out_byte(UINTN port, UINT8 data)
{
  IoWrite8(port, data);
}

extern void cdecl set_int_flag(int);
#endif

u8
read_cmos(u8 index)
{
  u8 data;
  set_int_flag(0);
  out_byte(CMOS_INDX, index);
  data = in_byte(CMOS_DATA);
  set_int_flag(1);
  return data;
}

void write_cmos(u8 index, u8 data)
{
  set_int_flag(0);
  out_byte(CMOS_INDX, index);
  out_byte(CMOS_DATA, data);
  set_int_flag(1);
}

int
check_cmos_time_update(void)
{
  u8 data;
  set_int_flag(0);
  out_byte(CMOS_INDX, RTC_STS_REG_A);
  data = (in_byte(CMOS_DATA) & BIT7);
  set_int_flag(1);
  return data;
}

void
set_alarm_interrupt(int option)
{
  u8 data;
  data = read_cmos(RTC_STS_REG_B);
  if (option){
    //enable
    data |= BIT5;
  }else{
    //clear
    data &= ~BIT5;
  }
  write_cmos(RTC_STS_REG_B, data);
}

void
get_rtc_time(RTC_TIME *rtc_tm)
{
  //hold if RTC update in progress.
  while(check_cmos_time_update());
  rtc_tm->tm_sec = read_cmos(RTC_SECONDS);
  rtc_tm->tm_min = read_cmos(RTC_MINUTES);
  rtc_tm->tm_hour = read_cmos(RTC_HOURS);
  rtc_tm->tm_day_of_month = read_cmos(RTC_DAY_OF_MONTH);
  rtc_tm->tm_day_of_week = read_cmos(RTC_DAY_OF_WEEK);
  rtc_tm->tm_month = read_cmos(RTC_MONTH);
  rtc_tm->tm_year = read_cmos(RTC_YEAR);

}

void
set_rtc_alarm(RTC_TIME *rtc_tm)
{
  write_cmos(RTC_HOURS_ALARM, rtc_tm->tm_hour_alram);
  write_cmos(RTC_MINUTES_ALARM, rtc_tm->tm_min_alarm);
  write_cmos(RTC_SECONDS_ALARM, rtc_tm->tm_sec_alarm);
}

void
calc_rtc_alarm(RTC_TIME *rtc_tm, u8 waketime)
{
  u8 hrs, min, sec;
  unsigned long total_secs=0;

  hrs = BCD_TO_BIN(rtc_tm->tm_hour);
  min = BCD_TO_BIN(rtc_tm->tm_min);
  sec = BCD_TO_BIN(rtc_tm->tm_sec);
  
  //waketime only use second, change to total seconds for easy calculate
  total_secs = (unsigned long)hrs*3600 + min*60 + sec;
  total_secs += waketime;
  //printf("total_secs: %lu\n", total_secs);
  hrs = (u8)(total_secs / 3600);
  min = (u8)((total_secs % 3600) / 60);
  sec = (u8)((total_secs % 3600) % 60);
  //fixed: 24hrs issue
  if (hrs >= 24)
    hrs = 0;

  rtc_tm->tm_hour_alram = BIN_TO_BCD(hrs);
  rtc_tm->tm_min_alarm = BIN_TO_BCD(min);
  rtc_tm->tm_sec_alarm = BIN_TO_BCD(sec);

}

void
clear_rtc_sts()
{
  u8 data;
  data = in_byte(PMBASE+1);
  data &= ~BIT2;
  out_byte(PMBASE+1, data);
}

void
set_rtc_event_enable(int option)
{
  u8 data;
  data = in_byte(PMBASE+3);
  if (option){
    data |= BIT2;
  }else{
    data &= ~BIT2;
  }
  out_byte(PMBASE+3, data);
}

void
set_slp_typ(u8 slp_typ)
{
  out_byte(PMBASE+5, slp_typ|SLP_EN);
  sleep(10);
}

void
sys_reboot()
{
  out_byte(0xcf9, 0x6);
  sleep(10);
}

int
print_usage(char *prog_name, FILE *channel, int ret_val)
{
  fprintf(channel, "\nusage: %s [-s[-r] | -t xxx | -w xxx]\n", prog_name);
  fprintf(channel, "Options\n");
  fprintf(channel, "-s      shutdown\n");
  fprintf(channel, "-r      reboot\n");
  fprintf(channel, "-t xxx  timeout xxx seconds\n");
  fprintf(channel, "-w xxx  wakeup after xxx seconds\n");
  return ret_val;
}

int
main(
  int argc, 
  char * argv[]
	)
{

  RTC_DEVICE *rtc_dev;
  int option, timeout, waketime;
  ACTION action = {0};
  int i;
  char *c = NULL;
  
  option=timeout=waketime=0;

  rtc_dev = malloc(sizeof(RTC_DEVICE));
  rtc_dev->get_time = get_rtc_time;
  rtc_dev->set_alarm = set_rtc_alarm;
  rtc_dev->calc_alarm = calc_rtc_alarm;

  while ((option = getopt (argc, argv, ":hsrt:w:")) !=-1)
{
  switch(option)
  {
    default:
//      fprintf(stderr, "internal error: unknown case %c\n", optopt);
      return EXIT_FAILURE;
    #ifdef DOS
    case '?':
      fprintf(stderr, "error: unknown option: -%c\n", optopt);
      action.error = 1; 
      break;
    case ':':
      fprintf(stderr, "error: missing argument for option -%c\n", optopt);
      action.error = 1; 
      break;
    #endif
    case 'h':
      action.help = 1;
      break;
    case 's':
      action.shutdown = 1; 
      break;
    case 'r':
      action.reboot = 1; 
      break;
    case 't':
      sscanf(optarg, "%d", &timeout);
      break;
    case 'w':
      action.wakeup = 1;
      sscanf(optarg, "%d", &waketime);
      break;
  }
}

if (action.error || optind < argc)
  return print_usage(basename(argv[0]), stderr, EXIT_FAILURE);

if (action.help)
  return print_usage(basename(argv[0]), stderr, EXIT_SUCCESS);

  //count number?
  if (timeout){
  if (action.shutdown)
    c="shutdown";
  if (action.reboot)
    c="reboot";

    for(i=timeout; i>=1; i--){
      fprintf(stderr, "time to %s %d \r", c, i);
      sleep(1);
    }
  }


if (action.wakeup){

  if (waketime == 0){
    fprintf(stderr, "error: wake time zero!\n");
    return EXIT_FAILURE;
  }
  //clear RTC status
  set_alarm_interrupt(0);
  set_rtc_event_enable(0);
  clear_rtc_sts();
  
  rtc_dev->get_time(&rtc_dev->rtc_time);
  rtc_dev->calc_alarm(&rtc_dev->rtc_time, (u8)waketime);
  rtc_dev->set_alarm(&rtc_dev->rtc_time);

  set_alarm_interrupt(1);
  set_rtc_event_enable(1);

  printf("time: %d:%d:%d\n", BCD_TO_BIN(rtc_dev->rtc_time.tm_hour), \
          BCD_TO_BIN(rtc_dev->rtc_time.tm_min), BCD_TO_BIN(rtc_dev->rtc_time.tm_sec));
  printf("time alarm: %d:%d:%d\n", BCD_TO_BIN(rtc_dev->rtc_time.tm_hour_alram), \
          BCD_TO_BIN(rtc_dev->rtc_time.tm_min_alarm), BCD_TO_BIN(rtc_dev->rtc_time.tm_sec_alarm));
}
  free(rtc_dev);

if (action.shutdown){
  //shutdown
  set_slp_typ(S5);
}

if (action.reboot){
  sys_reboot();
}

  return 0;
}
