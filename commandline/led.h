#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <usb.h>        /* this is libusb */
#include "opendevice.h" /* common code moved to separate module */

#include "../firmware/requests.h"   /* custom request numbers */
#include "../firmware/usbconfig.h"  /* device's VID/PID and names */

void print_status(usb_dev_handle *handle);

void test(usb_dev_handle *handle);

void toggle_led(usb_dev_handle *handle, int state);

void get_pwm_status(usb_dev_handle *handle);

void set_pwm_status(usb_dev_handle *handle, char* led_arg, char* hue_arg, char* brt_arg);

usb_dev_handle *my_init(usb_dev_handle *handle);

