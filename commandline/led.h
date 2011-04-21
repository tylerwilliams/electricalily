#ifndef __LED_H_INCLUDED__
#define __LED_H_INCLUDED__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <usb.h>        /* this is libusb */
#include "opendevice.h" /* common code moved to separate module */

#include "../firmware/requests.h"   /* custom request numbers */
#include "../firmware/usbconfig.h"  /* device's VID/PID and names */

usb_dev_handle *handle = NULL;

void print_status(void);

void test_working(void);

void test(void);

void toggle_led(int state);

void get_pwm_status(void);

void set_pwm_status(char* led_arg, char* hue_arg, char* brt_arg);

void open_handle(void);

void close_handle(void);

#endif __LED_H_INCLUDED__
