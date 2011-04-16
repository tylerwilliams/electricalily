#define LED_PORT_DDR        DDRC
#define LED_PORT_OUTPUT     PORTC
#define LED_BIT             1

#include <avr/io.h>
#include <avr/wdt.h>
#include <avr/interrupt.h>  /* for sei() */
#include <util/delay.h>     /* for _delay_ms() */
#include "oddebug.h"        /* This is also an example for using debug macros */

#include <avr/pgmspace.h>   /* required by usbdrv.h */
#include "usbdrv.h"
#include "requests.h"       /* The custom request numbers we use */

typedef uint8_t byte;

typedef struct _color { 
    byte r; byte g; byte b;
} color;

#define CHMAX 9

byte compare[CHMAX];
volatile byte compbuff[CHMAX];

/* global hue/brt variables; set these to change color! */
static volatile uint8_t led0_hue;
static volatile uint8_t led0_brt;

static volatile uint8_t led1_hue;
static volatile uint8_t led1_brt;

static volatile uint8_t led2_hue;
static volatile uint8_t led2_brt;

#define CH0_CLEAR (pinlevelB &= ~(1 << PB0)) // map CH0 to PB0 // MIDDLE RED
#define CH1_CLEAR (pinlevelB &= ~(1 << PB1)) // map CH1 to PB1 // MIDDLE GRN
#define CH2_CLEAR (pinlevelB &= ~(1 << PB2)) // map CH1 to PB  // MIDDLE BLU

#define CH3_CLEAR (pinlevelB &= ~(1 << PB3)) // map CH0 to PB0 // TOP RED
#define CH4_CLEAR (pinlevelB &= ~(1 << PB4)) // map CH1 to PB1 // TOP GRN
#define CH5_CLEAR (pinlevelB &= ~(1 << PB5)) // map CH1 to PB  // TOP BLUE

#define CH6_CLEAR (pinlevelD &= ~(1 << PD5)) // map CH0 to PB0 // BOTTOM RED
#define CH7_CLEAR (pinlevelD &= ~(1 << PD6)) // map CH1 to PB1 // BOTTOM GRN
#define CH8_CLEAR (pinlevelD &= ~(1 << PD7)) // map CH1 to PB  // BOTTOM BLU

/*
 * Given a variable hue 'h', that ranges from 0-252,
 * set RGB color value appropriately.
 * Assumes maximum Saturation & maximum Value (brightness)
 * Performs purely integer math, no floating point.
 */
void h_to_rgb(uint8_t h, color* c) {
    uint8_t hd = h / 42;   // 42 == 252/6,  252 == H_MAX
    uint8_t hi = hd % 6;   // gives 0-5
    uint8_t f = h % 42; 
    uint8_t fs = f * 6;
    switch( hi ) {
        case 0:
            c->r = 252;     c->g = fs;      c->b = 0;
            break;
        case 1:
            c->r = 252-fs;  c->g = 252;     c->b = 0;
            break;
        case 2:
            c->r = 0;       c->g = 252;     c->b = fs;
            break;
        case 3:
            c->r = 0;       c->g = 252-fs;  c->b = 252;
            break;
        case 4:
            c->r = fs;      c->g = 0;       c->b = 252;
            break;
        case 5:
            c->r = 252;     c->g = 0;       c->b = 252-fs;
            break;
    }
}


void hset(int led, byte h) {
    color c;
    h_to_rgb(h,&c);

    // avoid multiplication :/
    int index=0, pos=0;
    for (pos = 0; pos < led; pos++) {
      index += 3;
    }

    if (h == 0) {
      compbuff[index] = 0;
      compbuff[index+1] = 0;
      compbuff[index+2] = 0;
    }
    else {
      compbuff[index] = c.r;
      compbuff[index+1] = c.g;
      compbuff[index+2] = c.b;
    }
}

void hvset(int led, byte h, byte v) 
{
    color c;
    h_to_rgb(h,&c);
    // avoid multiplication :/
    int index=0, pos=0;
    for (pos = 0; pos < led; pos++) {
      index += 3;
    }

    if (h == 0) {
      compbuff[index] = 0;
      compbuff[index+1] = 0;
      compbuff[index+2] = 0;
    }
    else {
      compbuff[index] = ((c.r * v) / 255);
      compbuff[index+1] = ((c.g * v) / 255);
      compbuff[index+2] = ((c.b * v) / 255);
    }
}

void pwm_loop(void)
{
  static byte pinlevelB = (1 << PB0)|(1 << PB1)|(1 << PB2)|(1 << PB3)|(1 << PB4)|(1 << PB5);
  static byte pinlevelD = (1 << PD5)|(1 << PD6)|(1 << PD7);
  static byte softcount = 0xFF;
  
  // common cathode (GND) means positive logic
  // update outputs
  PORTB = pinlevelB;
  PORTD = pinlevelD;

  // increment modulo 256 counter and update
  if(++softcount == 0) {         
    compare[0] = compbuff[0];  
    compare[1] = compbuff[1];
    compare[2] = compbuff[2];

    compare[3] = compbuff[3];  
    compare[4] = compbuff[4];
    compare[5] = compbuff[5];
    
    compare[6] = compbuff[6];  
    compare[7] = compbuff[7];
    compare[8] = compbuff[8];

    pinlevelB = (1 << PB0)|(1 << PB1)|(1 << PB2)|(1 << PB3)|(1 << PB4)|(1 << PB5);
    pinlevelD = (1 << PD5)|(1 << PD6)|(1 << PD7);
  }
  // clear port pin on compare match (executed on next interrupt)
  if(compare[0] == softcount) CH0_CLEAR;
  if(compare[1] == softcount) CH1_CLEAR;
  if(compare[2] == softcount) CH2_CLEAR;

  if(compare[3] == softcount) CH3_CLEAR;
  if(compare[4] == softcount) CH4_CLEAR;
  if(compare[5] == softcount) CH5_CLEAR;

  if(compare[6] == softcount) CH6_CLEAR;
  if(compare[7] == softcount) CH7_CLEAR;
  if(compare[8] == softcount) CH8_CLEAR;
}

void ioinit( void )
{
  cli();

  DDRB = 0;
  DDRB |= (1<<PB0)|(1<<PB1)|(1<<PB2)|(1<<PB3)|(1<<PB4)|(1<<PB5);

  DDRC = 0;
  DDRC |= (1<<PC1);
    
  DDRD = 0;
  DDRD |= (1<<PD5)|(1<<PD6)|(1<<PD7);

  sei();                          // enable interrupts
}

/* ------------------------------------------------------------------------- */
/* ----------------------------- USB interface ----------------------------- */
/* ------------------------------------------------------------------------- */

usbMsgLen_t usbFunctionSetup(uchar data[8])
{
  usbRequest_t    *rq = (void *)data;
  static uchar    dataBuffer[6];  /* buffer must stay valid when usbFunctionSetup returns */

    if(rq->bRequest == CUSTOM_RQ_ECHO){ /* echo -- used for reliability tests */
        dataBuffer[0] = rq->wValue.bytes[0];
        dataBuffer[1] = rq->wValue.bytes[1];
        dataBuffer[2] = rq->wIndex.bytes[0];
        dataBuffer[3] = rq->wIndex.bytes[1];
        usbMsgPtr = dataBuffer;         /* tell the driver which data to return */
        return 4;
    }else if(rq->bRequest == CUSTOM_RQ_SET_STATUS){
        if(rq->wValue.bytes[0] & 1){    /* set LED */
	  LED_PORT_OUTPUT |= _BV(LED_BIT);
        }else{                          /* clear LED */
	  LED_PORT_OUTPUT &= ~_BV(LED_BIT);
        }
    }else if(rq->bRequest == CUSTOM_RQ_GET_STATUS){
        dataBuffer[0] = ((LED_PORT_OUTPUT & _BV(LED_BIT)) != 0);
        usbMsgPtr = dataBuffer;         /* tell the driver which data to return */
        return 1;                       /* tell the driver to send 1 byte */
    }else if(rq->bRequest == CUSTOM_RQ_GET_PWM){
      dataBuffer[0] = led0_hue;
      dataBuffer[1] = led0_brt;
      dataBuffer[2] = led1_hue;
      dataBuffer[3] = led1_brt;
      dataBuffer[4] = led2_hue;
      dataBuffer[5] = led2_brt;
      usbMsgPtr = dataBuffer;         /* tell the driver which data to return */
      return 6;                       /* tell the driver to send 6 bytes */
    }else if(rq->bRequest == CUSTOM_RQ_SET_PWM){
      /* which LED are we setting */
      byte index = rq->wIndex.bytes[0];
      if (index == 0) {
	led0_hue = rq->wValue.bytes[0];
	led0_brt = rq->wValue.bytes[1];
	hvset(0, led0_hue, led0_brt);
      }else if (index == 1) {
	led1_hue = rq->wValue.bytes[0];
	led1_brt = rq->wValue.bytes[1];
	hvset(1, led1_hue, led1_brt);
      }else if (index == 2) {
	led2_hue = rq->wValue.bytes[0];
	led2_brt = rq->wValue.bytes[1];
	hvset(2, led2_hue, led2_brt);
      }
    }
    return 0;   /* default for not implemented requests: return no data back to host */
}

/* ------------------------------------------------------------------------- */


int __attribute__((noreturn)) main(void)
{
uchar   i;

    wdt_enable(WDTO_1S);
    /* Even if you don't use the watchdog, turn it off here. On newer devices,
     * the status of the watchdog (on/off, period) is PRESERVED OVER RESET!
     */
    /* RESET status: all port bits are inputs without pull-up.
     * That's the way we need D+ and D-. Therefore we don't need any
     * additional hardware initialization.
     */
    //    odDebugInit();
    //    DBG1(0x00, 0, 0);       /* debug output: main starts */

    usbInit();
    usbDeviceDisconnect();  /* enforce re-enumeration, do this while interrupts are disabled! */
    i = 0;
    while(--i){             /* fake USB disconnect for > 250 ms */
        wdt_reset();
        _delay_ms(1);
    }
    usbDeviceConnect();
    ioinit();

    //    sei();
    //    DBG1(0x01, 0, 0);       /* debug output: main loop starts */
    LED_PORT_DDR |= _BV(LED_BIT);   /* make the LED bit an output */

    uint16_t counter = 0;
    for(;;){                /* main event loop */
        wdt_reset();
        usbPoll();
	//	DBG1(0x02, 0, 0);   /* debug output: main loop iterates */
	pwm_loop();
    }
}

/* ------------------------------------------------------------------------- */

