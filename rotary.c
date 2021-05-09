/*  Author: Steve Gunn
 * Licence: This work is licensed under the Creative Commons Attribution License.
 *           View this license at http://creativecommons.org/about/licenses/
 * 
 * (Very) slight adaptions for use in FortunaSays by Daniel Burton, May 2021.
*/
 
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include "rotary.h"

volatile int8_t rotary = 0;

void init_rotary()
{
	/* Ensure all pins are inputs with pull-ups enabled */
	DDRE &= ~_BV(ROTA) & ~_BV(ROTB) & ~_BV(SWC);
	PORTE |= _BV(ROTA) | _BV(ROTB) | _BV(SWC);
	DDRC &= ~_BV(SWN) & ~_BV(SWE) & ~_BV(SWS) & ~_BV(SWW);
	PORTC |= _BV(SWN) | _BV(SWE) | _BV(SWS) | _BV(SWW);
	/* Configure interrupt for any edge on rotary and falling edge for button */
	EICRB |= _BV(ISC40) | _BV(ISC50) | _BV(ISC71);
}

/*
 * The rotary encoder is not used in this program; this library is used for the switches.
int8_t get_rotary()
{
	static uint8_t lastAB = 0x00;
	uint8_t AB = PINE & (_BV(ROTA) | _BV(ROTB));
	if ((AB == 0x00 && lastAB == 0x20) || (AB == 0x30 && lastAB == 0x10))
		rotary--;
	if ((AB == 0x30 && lastAB == 0x20) || (AB == 0x00 && lastAB == 0x10))
		rotary++;
	lastAB = AB;
	return rotary;
}*/

uint8_t get_switch()
{
	uint8_t state = PINC & (_BV(SWN) | _BV(SWE) | _BV(SWS) | _BV(SWW));
	PINC &= ~(_BV(SWN) | _BV(SWE) | _BV(SWS) | _BV(SWW));
	return state;
}

ISR(INT4_vect)
{
	get_switch();
	_delay_us(100);
}

ISR(INT5_vect, ISR_ALIASOF(INT4_vect));

