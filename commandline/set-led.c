#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <usb.h>        /* this is libusb */
#include "opendevice.h" /* common code moved to separate module */

#include "led.h"

static void usage(char *name)
{
    fprintf(stderr, "usage:\n");
    fprintf(stderr, "  %s on ....... turn on LED\n", name);
    fprintf(stderr, "  %s off ...... turn off LED\n", name);
    fprintf(stderr, "  %s status ... ask current status of LED\n", name);
    fprintf(stderr, "  %s test ..... run driver reliability test\n", name);
    fprintf(stderr, "  %s get_pwm .. get the color and brightness of the leds\n", name);
    fprintf(stderr, "  %s set_pwm <led> <hue> <brightness> ... set the color and brightness of an led\n", name);
}

int main(int argc, char **argv)
{
  int isOn;

  if(argc < 2){   /* we need at least one argument */
    usage(argv[0]);
    exit(1);
  }
  open_handle();

  /* ok, actually do stuff */
  if(strcasecmp(argv[1], "status") == 0){
    print_status();
  }else if((isOn = (strcasecmp(argv[1], "on") == 0)) || strcasecmp(argv[1], "off") == 0){
    toggle_led(isOn);
  }else if(strcasecmp(argv[1], "test") == 0){
    test();
  }else if(strcasecmp(argv[1], "get_pwm") == 0){
    get_pwm_status();
  }else if(strcasecmp(argv[1], "set_pwm") == 0){
    set_pwm_status(argv[2], argv[3], argv[4]);
  }else{
    usage(argv[0]);
    exit(1);
  }
  close_handle();
  return 0;
}
