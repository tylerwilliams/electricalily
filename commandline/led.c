#include "led.h"

void print_status(void)
{
  char                buffer[4];
  int cnt = usb_control_msg(handle, USB_TYPE_VENDOR | USB_RECIP_DEVICE | USB_ENDPOINT_IN, CUSTOM_RQ_GET_STATUS, 0, 0, buffer, sizeof(buffer), 5000);
  if(cnt < 1){
    if(cnt < 0){
      fprintf(stderr, "USB error: %s\n", usb_strerror());
    }else{
      fprintf(stderr, "only %d bytes received.\n", cnt);
    }
  }else{
    printf("LED is %s\n", buffer[0] ? "on" : "off");
  }
}


void test(void)
{
  int i;
  char                buffer[4];
  srandom(time(NULL));
  for(i = 0; i < 50000; i++){
    int value = random() & 0xffff, index = random() & 0xffff;
    int rxValue, rxIndex;
    if((i+1) % 100 == 0){
      fprintf(stderr, "\r%05d", i+1);
      fflush(stderr);
    }
    int cnt = usb_control_msg(handle, USB_TYPE_VENDOR | USB_RECIP_DEVICE | USB_ENDPOINT_IN, CUSTOM_RQ_ECHO, value, index, buffer, sizeof(buffer), 5000);
    if(cnt < 0){
      fprintf(stderr, "\nUSB error in iteration %d: %s\n", i, usb_strerror());
      break;
    }else if(cnt != 4){
      fprintf(stderr, "\nerror in iteration %d: %d bytes received instead of 4\n", i, cnt);
      break;
    }
    rxValue = ((int)buffer[0] & 0xff) | (((int)buffer[1] & 0xff) << 8);
    rxIndex = ((int)buffer[2] & 0xff) | (((int)buffer[3] & 0xff) << 8);
    if(rxValue != value || rxIndex != index){
      fprintf(stderr, "\ndata error in iteration %d:\n", i);
      fprintf(stderr, "rxValue = 0x%04x value = 0x%04x\n", rxValue, value);
      fprintf(stderr, "rxIndex = 0x%04x index = 0x%04x\n", rxIndex, index);
    }
  }
  fprintf(stderr, "\nTest completed.\n");
}

void toggle_led(int state)
{
  char                buffer[4];
  int cnt = usb_control_msg(handle, USB_TYPE_VENDOR | USB_RECIP_DEVICE | USB_ENDPOINT_OUT, CUSTOM_RQ_SET_STATUS, state, 0, buffer, 0, 5000);
  if(cnt < 0){
    fprintf(stderr, "USB error: %s\n", usb_strerror());
  }  
}

void get_pwm_status(void)
{

  char buffer[6];
  /* 6 8bit values, pwm0_hue, pwm0_brt , pwm1_hue, pwm1_brt, pwm2_hue, pwm2_brt */
  int cnt = usb_control_msg(handle, USB_TYPE_VENDOR | USB_RECIP_DEVICE | USB_ENDPOINT_IN, CUSTOM_RQ_GET_PWM, 0, 0, buffer, sizeof(buffer), 5000);
  if(cnt < 0){
    fprintf(stderr, "\nUSB error. in iteration %s\n", usb_strerror());
    exit(1);
  }else if(cnt != 6){
    fprintf(stderr, "\nerror %d bytes received instead of 6\n", cnt);
    exit(1);
  }

  int pwm0_hue, pwm0_brt , pwm1_hue, pwm1_brt, pwm2_hue, pwm2_brt;
  pwm0_hue = (int)buffer[0] & 0xff;
  pwm0_brt = (int)buffer[1] & 0xff;

  pwm1_hue = (int)buffer[2] & 0xff;
  pwm1_brt = (int)buffer[3] & 0xff;

  pwm2_hue = (int)buffer[4] & 0xff;
  pwm2_brt = (int)buffer[5] & 0xff;
  
  fprintf(stderr, "\nLED   HUE   BRT\n");
  fprintf(stderr, "%3d%6d%6d\n", 0, pwm0_hue, pwm0_brt);
  fprintf(stderr, "%3d%6d%6d\n", 1, pwm1_hue, pwm1_brt);
  fprintf(stderr, "%3d%6d%6d\n", 2, pwm2_hue, pwm2_brt);

}

void set_pwm_status(char* led_arg, char* hue_arg, char* brt_arg)
{
  char* endptr;
  unsigned int led = strtoul(led_arg, &endptr, 0);
  unsigned int hue = strtoul(hue_arg, &endptr, 0);
  unsigned int brt = strtoul(brt_arg, &endptr, 0);
  //  fprintf(stderr, "\n setting %d to hue: %d, brt: %d\n", led, hue, brt);
  if (led < 0 || led > 2) {
    fprintf(stderr, "\nled index must be 0,1, or 2. was: %d", led);
    exit(1);
  }

  char                buffer[6];
  int cnt = usb_control_msg(handle, USB_TYPE_VENDOR | USB_RECIP_DEVICE | USB_ENDPOINT_IN, CUSTOM_RQ_SET_PWM, (brt<<8|hue), led, buffer, sizeof(buffer), 5000);
  
  if(cnt < 0){
    fprintf(stderr, "\nUSB error. in iteration %s\n", usb_strerror());
    exit(1);
  }
}

void open_handle(void)
{
  const unsigned char rawVid[2] = {USB_CFG_VENDOR_ID}, rawPid[2] = {USB_CFG_DEVICE_ID};
  char                vendor[] = {USB_CFG_VENDOR_NAME, 0}, product[] = {USB_CFG_DEVICE_NAME, 0};
  int                 vid, pid;
  usb_init();
  /* compute VID/PID from usbconfig.h so that there is a central source of information */
  vid = rawVid[1] * 256 + rawVid[0];
  pid = rawPid[1] * 256 + rawPid[0];
  /* The following function is in opendevice.c: */
  if(usbOpenDevice(&handle, vid, vendor, pid, product, NULL, NULL, NULL) != 0){
    fprintf(stderr, "Could not find USB device \"%s\" with vid=0x%x pid=0x%x\n", product, vid, pid);
    exit(1);
  }
}

void close_handle(void)
{
  usb_close(handle);
}
