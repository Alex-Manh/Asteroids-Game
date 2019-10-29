/*
 * timer0.c
 *
 * Author: Peter Sutton
 *
 * We setup timer0 to generate an interrupt every 1ms
 * We update a global clock tick variable - whose value
 * can be retrieved using the get_clock_ticks() function.
 */

#include <avr/io.h>
#include <avr/interrupt.h>

#include "timer0.h"
#include "score.h"
#include "health.h"

/* Our internal clock tick count - incremented every 
 * millisecond. Will overflow every ~49 days. */
static volatile uint32_t clockTicks;

//Variables for seven segment display
volatile uint8_t seven_seg_cc = 0;
volatile uint8_t start = 0;
/* Seven segment display segment values for 0 to 9 */
uint8_t left_digit[10] = {0,6,91,79,102,109,125,7,127,111};
uint8_t right_digit[10] = {63,6,91,79,102,109,125,7,127,111};
	
/* Set up timer 0 to generate an interrupt every 1ms. 
 * We will divide the clock by 64 and count up to 124.
 * We will therefore get an interrupt every 64 x 125
 * clock cycles, i.e. every 1 milliseconds with an 8MHz
 * clock. 
 * The counter will be reset to 0 when it reaches it's
 * output compare value.
 */
void init_timer0(void) {
	/* Reset clock tick count. L indicates a long (32 bit) 
	 * constant. 
	 */
	clockTicks = 0L;
	
	/* Clear the timer */
	TCNT0 = 0;

	/* Set the output compare value to be 124 */
	OCR0A = 124;
	
	/* Set the timer to clear on compare match (CTC mode)
	 * and to divide the clock by 64. This starts the timer
	 * running.
	 */
	TCCR0A = (1<<WGM01);
	TCCR0B = (1<<CS01)|(1<<CS00);

	/* Enable an interrupt on output compare match. 
	 * Note that interrupts have to be enabled globally
	 * before the interrupts will fire.
	 */
	TIMSK0 |= (1<<OCIE0A);
	
	/* Make sure the interrupt flag is cleared by writing a 
	 * 1 to it.
	 */
	TIFR0 &= (1<<OCF0A);
	
	/* Make all bits of port C output bits for seven segment display A to DP. 
	Make D2 the output bit for seven segment display CC.
	*/
	DDRC = 0xFF;
	DDRD = 1<<2;
	
	/* Make A0 to A3 the output bit for the Health LEDs*/
	DDRA = 0x0F;
}

uint32_t get_current_time(void) {
	uint32_t returnValue;

	/* Disable interrupts so we can be sure that the interrupt
	 * doesn't fire when we've copied just a couple of bytes
	 * of the value. Interrupts are re-enabled if they were
	 * enabled at the start.
	 */
	uint8_t interruptsOn = bit_is_set(SREG, SREG_I);
	cli();
	returnValue = clockTicks;
	if(interruptsOn) {
		sei();
		//As the game begins, start displaying the score on ssd
		start = 1;
	}
	return returnValue;
}

ISR(TIMER0_COMPA_vect) {
	/* Increment our clock tick count */
	clockTicks++;
	
	//Display health LEDs
	//L5 to L2: A0 to A3
	//Disappearing order: L2, L5, L3, L4
	//Hence, A3, A0, A2, A1
	volatile uint8_t health = get_health();
	if (health==4){
		PORTA = 0x0F; 
	}
	else if (health==3){
		PORTA = 0x07; //Lose A3
		}
	else if (health==2){
		PORTA = 0x06; //Lose A0
	}
	else if (health==1){
		PORTA = 0x02; //Lose A2
	}
	else if (health==0){
		 PORTA = 0x00; 
		//Game over
	}

	//Display the score in seven segment
	volatile uint32_t current_score=get_score();
	seven_seg_cc = 1 ^ seven_seg_cc;
	if(start) {
		if (current_score >= 100)
		{
			/* Score >= 100 - Decimal point is on */
			if(seven_seg_cc == 0) {
				PORTC = right_digit[(current_score/10)%10];
				} else {
				PORTC = left_digit[(current_score/100)%10] | 0x80;
			}
			}else{
			/* Score < 100 - Decimal point is off */
			if(seven_seg_cc == 0) {
				/* Display right digit */
				PORTC = right_digit[(current_score)%10];
				} else {
				/* Display left digit */
				PORTC = left_digit[(current_score/10)%10];
			}
		}
		/* Output the digit selection (CC) bit */
		PORTD = seven_seg_cc<<2;
	}
	else {
		/* No digits displayed -  display is blank */
		PORTC = 0;
	}
}


