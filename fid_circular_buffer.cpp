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

#include "fid_circular_buffer.h"

CircularBuffer::CircularBuffer()
{
	tail = NULL;
	head = NULL;
}

void CircularBuffer::put(uint8_t value)
{
	//noInterrupts();
	if( head == NULL )
	{
		head = buffer;
		*head = value;
		tail = head + 1;
	}
	else if( tail != head )
	{
		*tail = value;
		
		if( ++tail >= (buffer + FID_CIRCULAR_BUFFER_SIZE) )
			tail = buffer;
	}
	//interrupts();
}

bool CircularBuffer::read(uint8_t* value)
{
	bool notempty;
	//noInterrupts();
	if( head == NULL )
	{
		notempty = false;
	}
	else
	{
		*value = *head;
		
		head++;
		
		if( head == tail )
		{
			head = tail = NULL;
		}
		else if( head >= (buffer + FID_CIRCULAR_BUFFER_SIZE) )
		{
			head = buffer;
		}
		
		notempty = true;
	}
	//interrupts();
	return notempty;
}

void CircularBuffer::clear()
{
	//noInterrupts();
	head = tail = NULL;
	//interrupts();
}
