
/******************************************************************************************
 * A simple IR decoder for Arduino
 * 
 * Copyright (c) Robert Bakker 2013
 *
 * This file is free software; you can redistribute it and/or modify
 * it under the terms of either the GNU General Public License version 2
 * or the GNU Lesser General Public License version 2.1, both as
 * published by the Free Software Foundation.
 ******************************************************************************************
 * This library handles polling and decoding of the signal from an IR remote.
 * It is non-blocking code that executes in the background while your sketch
 * does it's thing.
 * The ir decoder works only for the Phillips RC-5 protocol.
 *****************************************************************************************/

#ifndef irDecoder_h		// Include guard
#define irDecoder_h

#include "Arduino.h"	// Contains just about everything

#define IR_NUM_BITS 13	// Used to reset ir.dataIndex
#define IR_DELAY 3		// Number of "ticks" to wait after receiving a bit from ir
#define IR_TIMEOUT 2	// Number of "ticks" before timing out on ir

/*Global Variables************************************************************************/

// dust...


/*Functions Prototypes********************************************************************/
		  
// This function sets up the library to decode IR on irPin.
void ir_begin(uint8_t irPin);

// This function returns the decoded IR if a valid code was received,
// and a 0 if there is no code.
// If an invalid code was received, it returns a -1.
int16_t ir_data(void);


#endif	// Part of the include guard