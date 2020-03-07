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

#include "FidPS2Keyboard.h"
#include "fid_timer.h"
#include "fid_circular_buffer.h"

static void fid_ps2kb_release_pins();
static void fid_ps2kb_attach_read_isr();
static void fid_ps2kb_attach_write_isr();
static void isr_read();
static void isr_write();

static int mClockPin = 3;
static int mDataPin = 5;
static int mIrqNum = 1;

#define KB_CLOCK_SET_MASK 0x08
#define KB_CLOCK_CLEAR_MASK 0xF7
#define KB_CLOCK_INPUT_MASK 0xF7
#define KB_CLOCK_OUTPUT_MASK 0x08
#define KB_DATA_SET_MASK 0x20
#define KB_DATA_CLEAR_MASK 0xDF
#define KB_DATA_INPUT_MASK 0xDF
#define KB_DATA_OUTPUT_MASK 0x20

static int inhibitCount = 0;
static unsigned int writeCount = 0;
static unsigned int writeEndCount = 0;

static CircularBuffer readBuffer;

void fid_ps2kb_init(int dataPin, int clockPin)
{
	mDataPin = dataPin;
	mClockPin = clockPin;
	
	mIrqNum = -1;
	
	if( mClockPin == 2 )
		mIrqNum = 0;
	else if( mClockPin == 3 )
		mIrqNum = 1;
	
	readBuffer.clear();
	
	fid_ps2kb_release_pins();
	
	fid_ps2kb_attach_read_isr();
	
	fid_timer_init();
	//fid_timer_add_function(fid_ps2kb_20us);
}

static inline void fid_ps2kb_release_pin(int pin)
{
	DDRD &= ~(1<<pin); // input
	PORTD |= (1<<pin); // high: pull-up resistor
}

static inline void fid_ps2kb_set_pin_low(int pin)
{
	DDRD |= (1<<pin); // output
	PORTD &= ~(1<<pin); // low
}

static inline void fid_ps2kb_set_clock_high()
{
#ifndef KB_CLOCK_SET_MASK
	fid_ps2kb_release_pin(mClockPin);
#else
	DDRD &= KB_CLOCK_INPUT_MASK;
	PORTD |= KB_CLOCK_SET_MASK;
#endif
}

static inline void fid_ps2kb_set_clock_low()
{
#ifndef KB_CLOCK_SET_MASK
	fid_ps2kb_set_pin_low(mClockPin);
#else
	DDRD |= KB_CLOCK_OUTPUT_MASK;
	PORTD &= KB_CLOCK_CLEAR_MASK;
#endif
}

static inline void fid_ps2kb_set_data_high()
{
#ifndef KB_DATA_SET_MASK
	fid_ps2kb_release_pin(mDataPin);
#else
	DDRD &= KB_DATA_INPUT_MASK;
	PORTD |= KB_DATA_SET_MASK;
#endif
}

static inline void fid_ps2kb_set_data_low()
{
#ifndef KB_DATA_SET_MASK
	fid_ps2kb_set_pin_low(mDataPin);
#else
	DDRD |= KB_DATA_OUTPUT_MASK;
	PORTD &= KB_DATA_CLEAR_MASK;
#endif
}

static inline void fid_ps2kb_release_pins()
{
	fid_ps2kb_release_pin(mDataPin);
	fid_ps2kb_release_pin(mClockPin);
}

static void fid_ps2kb_attach_read_isr()
{
	if( mIrqNum != -1 )
		attachInterrupt(mIrqNum, isr_read, FALLING);
}

static void fid_ps2kb_attach_write_isr()
{
	if( mIrqNum != -1 )
		attachInterrupt(mIrqNum, isr_write, FALLING);
}

static void fid_ps2kb_detach_isr()
{
	if( mIrqNum != -1 )
		detachInterrupt(mIrqNum);
}

static int bit_read_count = 0;
static uint8_t byte_read = 0;
static bool odd_parity = false;
static bool byte_read_correct = true;

static void isr_read()
{
	static uint32_t prev_ms=0;
	uint32_t now_ms;
	
	if( writeCount > 0 )
		return;
	
	now_ms = millis();
	if (now_ms - prev_ms > 100) {
		bit_read_count = 0;
	}
	prev_ms = now_ms;

#ifdef KB_DATA_SET_MASK
	int bit = PIND & (1<<mDataPin);
#else
	int bit = PIND & KB_DATA_SET_MASK;
#endif
	
	if( bit_read_count == 0 )
	{
		byte_read = 0;
		byte_read_correct = true;
		odd_parity = false;
		
		// start bit must be zero
		if( bit != 0 )
		{
			byte_read_correct = false;
		}
	}
	else if( bit_read_count <= 8 )
	{
		if( bit != 0 )
		{
			byte_read |= 1 << (bit_read_count - 1);
			odd_parity = !odd_parity;
		}
	}
	else if( bit_read_count == 9 )
	{
		// parity bit
		if( bit != 0 )
		{
			odd_parity = !odd_parity;
		}
		
		// must be odd parity
		if( !odd_parity )
		{
			byte_read_correct = false;
		}
	}
	else if( bit_read_count == 10 )
	{
		// stop bit must be one
		if( bit == 0 )
		{
			byte_read_correct = false;
		}
	}
	
	bit_read_count++;
	
	if( bit_read_count > 10 )
	{
		// frame is over
		if( !byte_read_correct )
		{
			fid_ps2kb_write(0xFE);
		}
		else
		{
			readBuffer.put(byte_read);
		}
		
		bit_read_count = 0;
	}
}

static int bit_write_count = 0;
static uint8_t byte_to_write = 0;
static bool write_odd_parity = false;
 
static void isr_write()
{
	if( writeEndCount > 0 )
		return;
	
	if( bit_write_count < 8 )
	{
		uint8_t bit = (byte_to_write & 0x01);
		byte_to_write >>= 1;
		if( bit )
		{
			write_odd_parity = !write_odd_parity;
			fid_ps2kb_set_data_high();
		}
		else
		{
			fid_ps2kb_set_data_low();
		}
	}
	else if( bit_write_count == 8 )
	{
		// send parity bit
		if( write_odd_parity )
		{
			fid_ps2kb_set_data_low(); // already odd
		}
		else
		{
			fid_ps2kb_set_data_high();
		}
	}
	else if( bit_write_count == 9 )
	{
		// stop bit = data line released
		fid_ps2kb_set_data_high();
	}
	else
	{
		// ack bit from device
#ifndef KB_DATA_SET_MASK
		int ack = PIND & (1<<mDataPin);
#else
		int ack = PIND & KB_DATA_SET_MASK;
#endif
		if( ack == 0 )
		// TODO react to ack, and wait for lines release
		writeEndCount = 4;
		fid_ps2kb_set_clock_low();
	}
	
	bit_write_count++;
}

static unsigned int count = 0;
void fid_ps2kb_20us()
{
	
	if( inhibitCount > 0 )
	{
		if( --inhibitCount > 0 )
		{
			if( (PIND & (1<<mClockPin)) != 0 )
				fid_ps2kb_set_clock_low();
		}
		else
			fid_ps2kb_set_clock_high();
	}
	else if( writeCount > 0 )
	{
		if( --writeCount > 0 )
		{
			if( (PIND & (1<<mClockPin)) != 0 )
				fid_ps2kb_set_clock_low();
		}
		else
		{
			fid_ps2kb_set_data_low();
			bit_read_count = 0;
			bit_write_count = 0;
			write_odd_parity = false;
			fid_ps2kb_attach_write_isr();
			fid_ps2kb_set_clock_high();
		}
	}
	else if( writeEndCount > 0 )
	{
		if( --writeEndCount > 0 )
		{
			if( (PIND & (1<<mClockPin)) != 0 )
				fid_ps2kb_set_clock_low();
		}
		else
		{
			fid_ps2kb_attach_read_isr();
			fid_ps2kb_set_clock_high();
		}
	}
}

void fid_ps2kb_write(uint8_t b)
{
	byte_to_write = b;
	writeCount = 11;
}

bool fid_ps2kb_read(uint8_t* b)
{
	noInterrupts();
	bool r = readBuffer.read(b);
	interrupts();
	return r;
}
