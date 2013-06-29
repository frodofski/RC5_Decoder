
/*****************************************************************************************/
/* A simple IR decoder for Arduino
/*****************************************************************************************/
/* Copyright (c) Robert Bakker 2013
/*
/* This file is free software; you can redistribute it and/or modify
/* it under the terms of either the GNU General Public License version 2
/* or the GNU Lesser General Public License version 2.1, both as
/* published by the Free Software Foundation.
/*****************************************************************************************/
/* This library handles polling and decoding of the signal from an IR remote.
/* It is non-blocking code that executes in the background while your sketch
/* does it's thing.
/* The ir decoder works only for the Phillips RC-5 protocol.
/*****************************************************************************************/

#include "irDecoder.h"
#include "Arduino.h"

// Used by state machines in the ISR
#define STANDBY 0
#define DELAYING 1
#define WAITING 2
#define DELAYING_END 3
#define IDLE 4


/*Declare Variables***********************************************************************/

struct irVariables
{
	volatile uint8_t pin;
	volatile uint8_t state;
	volatile int16_t data;

	volatile uint8_t pinState;
	volatile uint8_t pinStateOld;
	volatile uint8_t counter;
	volatile uint8_t dataIndex;
};

// Initialize variables (start all state machines in STANDBY state)
irVariables ir = { 0, STANDBY, 0,  0, 0, 0, IR_NUM_BITS };


/*Start of Functions**********************************************************************/

// This function sets up timer2 to trigger an ISR every 300 us.
// It also sets up the input pin.
void ir_begin(uint8_t irPin)
{
	ir.pin = irPin;
	
	pinMode(ir.pin, INPUT);
	
	// Configure timer 2
	cli();					// Disable global interrupts
	
	TCCR2A = 0;				// Clear timer2's control registers
	TCCR2B = 0;
	TIMSK2 = 0;				// ...and interrupt mask register (just in case)
	TCNT2 = 0;				// Pre-load the timer to 0
	OCR2A = 149;			// Set output compare register to 149
	TCCR2A |= _BV(WGM21);	// Turn on CTC mode (Clear Timer on Compare match)
	TCCR2B |= 0b011;		// Set prescaler to 32 (starts timer) 
	TIMSK2 |= _BV(OCIE2A);	// Enable timer compare interrupt 

	sei();					// Re-enable global interrupts
}

// This function returns the decoded IR if a valid code was received, 
// and a 0 if there is no code.
// If an invalid code was received, it returns a -1.
// It then resets the decoder state machine to start decoding again.
int16_t ir_data(void)
{	
	if(ir.state == IDLE)
	{
		int16_t dataTemp = ir.data;
		
		// These two variables aren't always reset by the state machine to save time
		ir.dataIndex = IR_NUM_BITS;
		ir.counter = 0;

		ir.data = 0;
		ir.state = STANDBY;
		
		return dataTemp;		
	}
	
	else
	{
		return 0;
	}	
}


/*ISR*************************************************************************************/

// This is the interrupt itself
// It is only active if ir_begin() is called
// This polls the input pin and decodes the incoming signal.
// "Decoding" is done using a state machine.
ISR(TIMER2_COMPA_vect)
{	
	// Read state of irPin (output of ir receiver is inverting)
	// This is done at the start to ensure timing accuracy
	ir.pinState = !digitalRead(ir.pin);	

	// State machine for ir decoding
	// It helps to have a diagram of the RC-5 protocol
	// when trying to understand how this works
	//
	// The steps below are described as if the receiver is not inverting
	switch(ir.state)
	{
		case STANDBY:
			// Do nothing until the pin goes high
			if(ir.pinState)
			{
				// Next step
				ir.state = DELAYING;
			}
			break;

		case DELAYING:
			// Wait for 1200 uS
			ir.counter++;
			if(ir.counter > IR_DELAY)
			{
				// Capture the pin state
				ir.pinStateOld = ir.pinState;
				// The counter variable is re-used so it needs to be reset
				ir.counter = 0;
				// Next step
				ir.state = WAITING;
			}
			break;

		case WAITING:
			// Wait for the pin to change state
			if(ir.pinState != ir.pinStateOld)
			{
				// If it does...
				// ...step to the next bit of the data variable
				ir.dataIndex--;
				// Again, reset the counter variable
				ir.counter = 0;
				// Get ready to receive another bit
				ir.state = DELAYING;

				// The data variable is always reset to 0 before attempting to receive ir
				// As a result, if the incoming bit is a 0, nothing needs to be done to 
				// the corresponding bit in the data variable
				// But if its a 1...
				if(ir.pinState)
				{
					// ...write the corresponding bit to 1 using a bit mask generated
					// with the data index variable
					ir.data |= _BV(ir.dataIndex);
				}

				// If we have received all of the bits without timing out
				if(ir.dataIndex == 0)
				{
					// Next step
					ir.state = DELAYING_END;
				}
			}

			// If nothing happens before the timeout (900 uS)...
			else
			{
				ir.counter++;
				if(ir.counter > IR_TIMEOUT)
				{
					// Flag invalid data
					ir.data = -1;
					// Next step (do nothing until further notice)
					ir.state = IDLE;
				}
			}
			break;

		case DELAYING_END:
			// This very last step just waits for a short while after a full set of bits
			// has been received. This is to prevent the decoder from "misfiring" on
			// the "tail" of the very last bit, caused by the state machine being reset
			// too soon after decoding a full ir code
			ir.counter++;
			if(ir.counter > IR_DELAY)
			{
				// Do nothing until further notice
				ir.state = IDLE;
			}
			break;
	}
}