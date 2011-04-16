#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdlib.h> 

//PB0 = OC0A = GREEN
//PB1 = OC0B = BLUE
//PB4 = OC1B = RED

// #define LED_ON    PORTB |= (1<<PB0);
// #define LED_OFF   PORTB &= ~(1<<PB0);

#define LED_ON      PORTB |= (1<<PB4);
#define LED_OFF     PORTB &= ~(1<<PB4);

#define RED OCR1B
#define GRN OCR0A
#define BLU OCR0B

//#define BR_9600     //!< Desired baudrate...choose one, comment the others.
// #define BR_19200    //!< Desired baudrate...choose one, comment the others.
#define BR_38400      //!< Desired baudrate...choose one, comment the others.

//This section chooses the correct timer values for the chosen baudrate.
#ifdef  BR_9600
    #define TICKS2COUNT         103  //!< Ticks between two bits.
    #define TICKS2WAITONE       103  //!< Wait one bit period.
    #define TICKS2WAITONE_HALF  155  //!< Wait one and a half bit period.
#endif
#ifdef  BR_19200
    #define TICKS2COUNT          51  //!< Ticks between two bits.
    #define TICKS2WAITONE        51  //!< Wait one bit period.
    #define TICKS2WAITONE_HALF   77  //!< Wait one and a half bit period.
#endif
#ifdef  BR_38400
    #define TICKS2COUNT          25  //!< Ticks between two bits.
    #define TICKS2WAITONE        25  //!< Wait one bit period.
    #define TICKS2WAITONE_HALF   38  //!< Wait one and a half bit period.
#endif

#define INTERRUPT_EXEC_CYCL   9       //!< Cycles to execute interrupt rutine from interrupt.

//Some IO, timer and interrupt specific defines.

// #define ENABLE_TIMER_INTERRUPT( )       ( TIMSK |= ( 1<< OCIE0A ) )
// #define DISABLE_TIMER_INTERRUPT( )      ( TIMSK &= ~( 1<< OCIE0A ) )
// #define CLEAR_TIMER_INTERRUPT( )        ( TIFR |= ((1 << OCF0A) ) )
// 
// #define ENABLE_EXTERNAL0_INTERRUPT( )   ( GIMSK |= ( 1<< INT0 ) )
// #define DISABLE_EXTERNAL0_INTERRUPT( )  ( GIMSK &= ~( 1<< INT0 ) )

#define RX_PIN           PB2               //!< Receive data pin, must be INT0
#define TCCR             TCCR0A             //!< Timer/Counter Control Register
#define TCCR_P           TCCR0B             //!< Timer/Counter Control (Prescaler) Register
#define OCR              OCR0A              //!< Output Compare Register
#define EXT_IFR          GIFR              //!< External Interrupt Flag Register
#define EXT_ICR          MCUCR             //!< External Interrupt Control Register
#define TIMER_COMP_VECT  TIMER0_COMP_vect  //!< Timer Compare Interrupt Vector

#define TRXDDR  DDRB
#define TRXPORT PORTB
#define TRXPIN  PINB

#define GET_RX_PIN( )    ( TRXPIN & ( 1 << RX_PIN ) )

/*! \brief  Type defined enumeration holding software UART's state.
 *
 */
typedef enum {
		IDLE,                                       //!< Idle state, both transmit and receive possible.
		TRANSMIT,                                   //!< Transmitting byte.
		TRANSMIT_STOP_BIT,                          //!< Transmitting stop bit.
		RECEIVE,                                    //!< Receiving byte.
		DATA_PENDING                                //!< Byte received and ready to read.
		
}AsynchronousStates_t;

typedef struct _color { 
    uint8_t r; uint8_t g; uint8_t b;
} color;

static volatile AsynchronousStates_t state;     //!< Holds the state of the UART.
static volatile unsigned char SwUartRXData;     //!< Storage for received bits.
static volatile unsigned char SwUartRXBitCount; //!< RX bit counter.

static volatile uint8_t red_val;
static volatile uint8_t blu_val;
static volatile uint8_t grn_val;

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

void setup_serial_timer( void ) {
    TIMSK &= ~( 1<< OCIE0A );
    
    TCCR0A = 0;
    TCCR0A |= (1 << WGM01);          // Timer in CTC mode.
    
    TCCR0B = 0;
    TCCR0B |=  ( 1 << CS01 );        // Divide by 8 prescaler.
}

void setup_pwm_timer( void ) {
    TCCR0A = 0;
    TCCR0A |= (1<<WGM00) | (1<<COM0A1) | (1<<COM0B1);       // PWMA/B on PB0/1, clear on compare match, Phase correct PWM Mode
    
    TCCR0B = 0;
    TCCR0B |= (1<<CS00);                                    // No Prescaler
}

void ioinit( void ) {
    DDRB = 0;                                               //all input
    DDRB |= (1<<PB0)|(1<<PB1)|(1<<PB4);                     //output on these pins
    
    TCCR0A |= (1<<WGM00) | (1<<COM0A1) | (1<<COM0B1);       // PWMA/B on PB0/1, clear on compare match, Phase correct PWM Mode
    TCCR0B |= (1<<CS00);                                    // No Prescaler

    // TCCR1 |= (1<<CS10)|(1<<CS12);                           // Phase correct PWM Mode, No Prescaler
    // GTCCR |= (1<<PWM1B)|(1<<COM1B1);                        // PWM1B on PB4, clear on compare match
    
    //blink 3 times when we turn on
	uint8_t blinkTimes = 3;
	while (blinkTimes > 0) {
        LED_ON;
		_delay_ms(80);
        LED_OFF;
		_delay_ms(80);
		blinkTimes--;
	}
}

static void initSoftwareUart( void ) {	
    // TRXPORT |= ( 1 << RX_PIN );       // RX_PIN is input, tri-stated.
	
	// // This timer will get setup when the int0 interrupt triggers
    // // Timer0
    // TIMSK &= ~( 1<< OCIE0A );
    // 
    // TCCR0A |= (1 << WGM01);          // Timer in CTC mode.
    // TCCR0B |=  ( 1 << CS01 );        // Divide by 8 prescaler.
    //  
	//External interrupt
	EXT_ICR = 0x00;                   // Init.
	EXT_ICR |= ( 1 << ISC01 );        // Interrupt sense control: falling edge.
	GIMSK |= ( 1<< INT0 );    // Turn external interrupt on.
	
	//Internal State Variable
	state = IDLE;
}

int main( void )
{
    cli();                      //disable interrupts
	ioinit();
    initSoftwareUart();
	sei();                      // enable interrupts
	
	color c;
	for( ; ; )
	{
	    if( state == DATA_PENDING ) {
            state = IDLE;
            
            PORTB ^= (1<<PB4);
            // h_to_rgb(SwUartRXData,&c);
            // RED = c.r;
            // GRN = c.g; 
            // BLU = c.b;
        }
	}
}

/*
 * This interrupt gets triggered when the RX pin goes high
 */
ISR (INT0_vect) {
    // fix timers for serial
    setup_serial_timer();
	
	state = RECEIVE;                        // Change state
	GIMSK |= ( 1<< INT0 );                  // Disable interrupt during the data bits.
	TIMSK &= ~( 1<< OCIE0A );               // Disable timer to change its registers.
	
	TCCR_P &= ~( 1 << CS01 );               // Reset prescaler counter.
	TCNT0 = INTERRUPT_EXEC_CYCL;            // Clear counter register. Include time to run interrupt rutine.
	TCCR_P |= ( 1 << CS01 );                // Start prescaler clock. TODO: check this
	
	OCR = TICKS2WAITONE_HALF;               // Count one and a half period into the future.
	
	SwUartRXBitCount = 0;                   // Clear received bit counter.
	TIFR |= ((1 << OCF0A) );                // Clear interrupt bits
	TIMSK |= ( 1<< OCIE0A );                // Enable timer0 interrupt on again
}

/*
 * This interrupt gets triggered when the RX pin goes high
 */
ISR(TIM0_COMPA_vect) {
	switch (state) {
		//Receive Byte.
		case RECEIVE:
			OCR = TICKS2WAITONE;                  // Count one period after the falling edge is trigged.
			//Receiving, LSB first.
			if( SwUartRXBitCount < 8 ) {
				SwUartRXBitCount++;
				SwUartRXData = (SwUartRXData>>1);   // Shift due to receiving LSB first.
				if( GET_RX_PIN( ) != 0 ) {
					SwUartRXData |= 0x80;           // If a logical 1 is read, let the data mirror this.
				}
			}
			
			//Done receiving
			else {
			    // fix timers for PWM
                setup_pwm_timer();
				state = DATA_PENDING;               // Enter DATA_PENDING when one byte is received.
				TIMSK &= ~( 1<< OCIE0A );         // Disable this interrupt.
				EXT_IFR |= (1 << INTF0 );           // Reset flag not to enter the ISR one extra time.
				GIMSK |= ( 1<< INT0 );      // Enable interrupt to receive more bytes.
			}
			break;
			
			// Unknown state.
		default:        
			state = IDLE;                           // Error, should not occur. Going to a safe state.
	}
}
