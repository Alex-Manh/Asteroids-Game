
/*
 * joystick.S
 *
 * Created: 25/05/2019 11:49:33 PM
 *  Author: Minh Anh Bui
 */ 
#include "serialio.h"
#include "joystick.h"

#include <stdio.h>
#include <avr/interrupt.h>

uint16_t value;
uint8_t move_ment = 0;/* L/r = 0, U/D = 1 */
char joystick(void)
{	
	char returnChar=' ';
	/* Turn on global interrupts */
	sei();
	
	// Set ADC - AVCC reference, right adjust
	ADMUX = (1<<REFS0)|(1<<MUX2)|(1<<MUX1);
	// Turn on the ADC
	ADCSRA = (1<<ADEN)|(1<<ADPS2)|(1<<ADPS1);

	// Set the ADC mux to choose ADC6 if x_or_y is 0, ADC7 if x_or_y is 1
	// 	ADC6 is  for L/R
	// 	ADC7 is  for U/D
	if(move_ment == 0) {
		ADMUX &= ~1;
		} else {
		ADMUX |= 1;
	}
	// Start the ADC conversion
	ADCSRA |= (1<<ADSC);
	
	while(ADCSRA & (1<<ADSC)) {
		; /* Wait until conversion finished */
	}
	value = ADC; // read the value
	//Neutral value of the ADC is ~500
	//Absolute order will only be taken if the value is <400 or >600
		if(move_ment == 0) {
			if (value<=400)
			{
				returnChar = 'L';
			}
			else if (value>=600)
			{
				returnChar = 'R';
			}
		} else {
 			if (value<=400 || value>=600)
 			{
				 returnChar = 'A';
 			}
		}
	move_ment ^= 1;
	return returnChar;
}