
#define irPin 9

/*
This is the decoder itself.
It needs to be called as soon as the first start bit is detected.
It operates using an ACTIVE LOW (inverting) ir receiver
connected to irPin.
*/
int irDecode()
{
  // This is where our IR code is going to be stored
  int data = 0;
  // We are only going to detect the last 13 bits because
  // the first bit is lost as we use it to start the decoder.
  for(int i=12; i>=0; i--)	
  {
    // First we wait 1333 uS (1.5 x 889 uS)
    // This leaves us about 445 uS before the next transition,
    // but after the previous (but not valid) transition between bits.
    delayMicroseconds(1333);
    // Used for timeout
    unsigned long microsOld = micros();
    // Record the current state of the pin
    boolean irOld = digitalRead(irPin);     
    // This is our 'transition detector'
    // It constantly checks the irPin for a change and if it sees one,
    // it writes the corresponding bit in data to a 1 or a 0 if its
    // a rising or a falling edge respectively.
    // There is also a timout, which trips if no
    // transition is detected within 889 uS, 
    // it then returns a -1 for data signifying an invalid code.
    while(1)  
    {
      if(digitalRead(irPin) != irOld)  // Look for transition
      {
        bitWrite(data, i, irOld);      // Write the bit to a 1 or 0
        break;			       // Done for this bit!
      }
			
      if((micros() - microsOld) > 889) // Timeout
      {
        return -1;
      }
    }
  }
  delayMicroseconds(1333);  // Ignore "tail" of transmission
  return data;
}

void setup()
{
  Serial.begin(9600);
  pinMode(irPin, INPUT);
}

void loop()
{
  // We need to keep checking for the first start bit
  // If the first start bit is detected...
  if(!digitalRead(irPin))
  {
    // ...decode the rest!
    int code = irDecode();
    
    Serial.print("Code: ");
    Serial.println(code);
  }
}
