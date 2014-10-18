
//  Author: avishorp@gmail.com
//
//  This library is free software; you can redistribute it and/or
//  modify it under the terms of the GNU Lesser General Public
//  License as published by the Free Software Foundation; either
//  version 2.1 of the License, or (at your option) any later version.
//
//  This library is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
//  Lesser General Public License for more details.
//
//  You should have received a copy of the GNU Lesser General Public
//  License along with this library; if not, write to the Free Software
//  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

extern "C" {
  #include <stdlib.h>
  #include <string.h>
  #include <inttypes.h>
}

#include <TM1637Display.h>

#define TM1637_I2C_COMM1    0x40
#define TM1637_I2C_COMM2    0xC0
#define TM1637_I2C_COMM3    0x80


TM1637Display::TM1637Display(uint8_t pinClk, uint8_t pinDIO)
{
	// Copy the pin numbers
	m_pinClk = pinClk;
	m_pinDIO = pinDIO;
	
	// Set the pin direction and default value.
	// Both pins are set as inputs, allowing the pull-up resistors to pull them up
    pinMode(m_pinClk, INPUT);
    pinMode(m_pinDIO,INPUT);
	digitalWrite(m_pinClk, LOW);
	digitalWrite(m_pinDIO, LOW);
}

void TM1637Display::setBrightness(uint8_t brightness)
{
	m_brightness = brightness;
}


//-- api for displaying various types

void TM1637Display::showNumber(float number,int decPlaces){
   char buffer[7];  
   dtostrf(number, 0, decPlaces, buffer);
   showText(buffer);
}

void TM1637Display::showText(String text){
   int textLength = text.length() + 1;
   char char_array[textLength];
   text.toCharArray(char_array, textLength);
   showText(char_array);
}

void TM1637Display::showText(char* text)
{
  uint8_t segmentArr[4];
  int arrSize = 4;
  int segmentIndex = -1;
  for(int i=0; i < arrSize; i++)
     segmentArr[i] = 0;
  for(int i=0; i< strlen(text) && segmentIndex < arrSize; i++)
  {
    char c = text[i];
    if(isSpecialSegmentCharacter(c)){
       if(segmentIndex>=0)
         segmentArr[segmentIndex] |= 0b10000000;
    }
    else{
       segmentIndex++;
       segmentArr[segmentIndex] = charToSegment(c);
    }
  }
  setSegments(segmentArr);
}

//TODO: change to some hashtable
uint8_t TM1637Display::charToSegment(char c)
{
  //
  //      A
  //     ---
  //  F |   | B
  //     -G-
  //  E |   | C
  //     ---
  //      D
  if(c == '0') return 0b00111111;    // 0
  if(c == '1') return 0b00000110;    // 1
  if(c == '2') return 0b01011011;   // 2
  if(c == '3') return 0b01001111;    // 3
  if(c == '4') return 0b01100110;    // 4
  if(c == '5') return 0b01101101;    // 5
  if(c == '6') return 0b01111101;   // 6
  if(c == '7') return 0b00000111;    // 7
  if(c == '8') return 0b01111111;    // 8
  if(c == '9') return 0b01101111;    // 9
  if(c == '-') return 0b01000000;
  if(c == 'A') return 0b01110111;    // A
  if(c == 'b') return 0b01111100;    // b
  if(c == 'C') return 0b00111001;   // C
  if(c == 'd') return 0b01011110;    // d
  if(c == 'E') return 0b01111001;    // E
  if(c == 'F') return 0b01110001;    // F
  if(isSpecialSegmentCharacter(c))
     return 0b10000000;
  return 0;
}

boolean TM1637Display::isSpecialSegmentCharacter(char c)
{
  return c == '.' || c == ',' || c == ':' || c == ';';
}

//-- low level driver

void TM1637Display::setSegments(const uint8_t segments[], uint8_t length, uint8_t pos)
{
    // Write COMM1
	start();
	writeByte(TM1637_I2C_COMM1);
	stop();
	
	// Write COMM2 + first digit address
	start();
	writeByte(TM1637_I2C_COMM2 + (pos & 0x03));
	
	// Write the data bytes
	for (uint8_t k=0; k < length; k++) 
	  writeByte(segments[k]);
	  
	stop();

	// Write COMM3 + brightness
	start();
	writeByte(TM1637_I2C_COMM3 + (m_brightness & 0x0f));
	stop();
}
 
void TM1637Display::bitDelay()
{
	delayMicroseconds(50);
}
   
void TM1637Display::start()
{
  pinMode(m_pinDIO, OUTPUT);
  bitDelay();
}
   
void TM1637Display::stop()
{
	pinMode(m_pinDIO, OUTPUT);
	bitDelay();
	pinMode(m_pinClk, INPUT);
	bitDelay();
	pinMode(m_pinDIO, INPUT);
	bitDelay();
}
  
bool TM1637Display::writeByte(uint8_t b)
{
  uint8_t data = b;

  // 8 Data Bits
  for(uint8_t i = 0; i < 8; i++) {
    // CLK low
    pinMode(m_pinClk, OUTPUT);
    bitDelay();
    
	// Set data bit
    if (data & 0x01)
      pinMode(m_pinDIO, INPUT);
    else
      pinMode(m_pinDIO, OUTPUT);
    
    bitDelay();
	
	// CLK high
    pinMode(m_pinClk, INPUT);
    bitDelay();
    data = data >> 1;
  }
  
  // Wait for acknowledge
  // CLK to zero
  pinMode(m_pinClk, OUTPUT);
  pinMode(m_pinDIO, INPUT);
  bitDelay();
  
  // CLK to high
  pinMode(m_pinClk, INPUT);
  bitDelay();
  uint8_t ack = digitalRead(m_pinDIO);
  if (ack == 0)
    pinMode(m_pinDIO, OUTPUT);
	
	
  bitDelay();
  pinMode(m_pinClk, OUTPUT);
  bitDelay();
  
  return ack;
}


   

