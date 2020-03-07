/*
	PS/2 library for Arduino

	Copyright (c) 2014 Freedelity

	Permission is hereby granted, free of charge, to any person obtaining a copy
	of this software and associated documentation files (the "Software"), to deal
	in the Software without restriction, including without limitation the rights
	to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
	copies of the Software, and to permit persons to whom the Software is
	furnished to do so, subject to the following conditions:

	The above copyright notice and this permission notice shall be included in all
	copies or substantial portions of the Software.

	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
	IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
	FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
	AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
	LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
	OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
	SOFTWARE.
 */

#include "fid_timer.h"
#include "FidPS2Host.h"
#include "FidPS2Keyboard.h"

static int has_been_init=0;

ISR(TIMER1_CAPT_vect)
{
	// call each functions
	fid_ps2kb_20us();
	fid_ps2h_clk_20us();
}

/*
 * Init the timer to overflow after 20 microsecond and
 * attach the ISR.
 * Can be called several times, it will be init only
 * the first time, so call it at the beginning of your
 * project, or at the beginning of each code that needs
 * to set a repeated functions.
 */
void fid_timer_init()
{
	if( has_been_init )
	{
		return;
	}
	
	cli(); // disable interrupts
	
	TCCR1B = 0; //stop timer
	
	// clear timer
	TCCR1A = 0;
	TCNT1 = 0;
	TIMSK1 = 0;
	TIFR1 = 0xFF;
	
	// with no prescale, 20us is reached after 160 clocks
	ICR1 = 160;
	//ICR1 = 150;
	
	TIMSK1 = 0x20; // enable ICF1 interrupt
	
	// start timer with no prescale, CTC mode
	TCCR1B = 0x11;
	
	has_been_init = 1;
	
	sei(); // enable interrupts
}
