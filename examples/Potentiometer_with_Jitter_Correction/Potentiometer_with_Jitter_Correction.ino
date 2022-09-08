/* =======================================================================
 * Title:  Potentiometer_with_Jitter_Correction
 * Description: Using a Volume Pedal with an internal Potentiometer, 
 * or any potentiometer on the Analogue Input of an Arduino Leonardo. 
 * Author: Gaetano Trombelli
 * Date: 8/9/2022
 * =======================================================================
 */
//  ====================================================== 
//               USB MIDI Library to include
//  ====================================================== 
/* 
 *  MIDI USB Library required for Arduino ATmega32U4 
 *  compatible for example an Arduino Leonardo.
 *  References:
 *  https://github.com/arduino-libraries/MIDIUSB
 *  https://www.arduino.cc/reference/en/libraries/midiusb/  
 */
#include "MIDIUSB.h"


//  =================================================          
//              Variable Declarations
//  ================================================= 
/*
 *  Since MIDI Channels are based on Binary Numbers
 *  0000 = 0 = Channel 1 to 1111 = 15 = Channel 16 
 *  To allocate a Channel 'n': use the number n-1,  
 *  e.g., For Channel 3, use n-1 = 2.
 *  Volume MIDI CC = 7, MIDI CC List from: 
 *  https://professionalcomposers.com/midi-cc-list/
*/
const byte volPedalChannel = 0;// '0' = MIDI channel 1
const byte volPedalCC = 7;     // Volume MIDI CC = 7
int volPedalValue = 0;         // Current Volume Value
int volPedalPrevValue = 0;     // Previous Volume Value


void setup() {          // This Runs once.
  Serial.begin(115200); // MIDI Baud Rate: Use 115200 for USB MIDI
    }


void loop() {
  byte jitterAmount = 3; // Acceptable amount of the variation in potentiometer output.
/* 
 *  The analogue Input has an output of 0-1023, but due to the pot value jittering, 
 *  we need to do some pre filtering, by only allowing the triggerMidiCCvolPedal();
 *  funtion to be called if the value has changed since last time by at least the 'jitterAmount' 
 *  either way, i.e., (Minus 3 OR ( || ) Plus 3), this was obtained by trial and error. 
 *  Only then is the triggerMidiCCvolPedal() function executed, otherwise the pedal 
 *  will constantly stream data out of the MIDI port and use up CPU cycles.
 *  Reference: https://forum.arduino.cc/t/potentiometer-jitter/437767
*/
    volPedalValue = analogRead(4); // Read the Analogue Input (A4).
    
          
    if ((volPedalValue < (volPedalPrevValue - jitterAmount)) || (volPedalValue > (volPedalPrevValue + jitterAmount))) 
        {
          triggerMidiCCvolPedal(); // Execute this only if 'volPedalValue' has changed by at least the value of 'jitterAmount'.
        }
    }
    

void triggerMidiCCvolPedal() {
  byte volPedalOutput; // Actual value sent to MIDI output
/*  Since the Analogue input reads: 0 to 5 volts as 1024 values between 
 *   0 to 1023. Divide volPedalOutput (1024 values) below by 8 to get 
 *   MIDI CC range: 0-127 (128 values), using a '10k LINEAR' pot (10kB). 
 *  A 10k Linear pot is used as it works well and it's a common value.
 */    
     volPedalOutput = (volPedalValue/8); // 1024/8 = 128 values.
     controlChange(volPedalChannel, volPedalCC, volPedalOutput); 
     
/* The following line 'Serial.println(volPedalOutput);' can be removed
 *  once the pot or Pedal which has a pot is determined to function correctly.
 *  If you open the 'serial monitor' it will show the instantaneous 
 *  value of the pot and you can see 'if' the value randomly varies when
 *  it is not being touched (jitters) and adjust the 'jitterAmount'.
*/
  Serial.println(volPedalOutput);
    
  MidiUSB.flush(); // send immediately
      volPedalPrevValue = volPedalValue;
  }


//====================================================================
//     -----     ATmega32U4 (Leonardo) MIDIUSB Library     -----
//====================================================================
//--------------------------------------------------------------------
//                  Control Change / CC Messages
//--------------------------------------------------------------------
// First parameter is the event type (0x0B = control change).
// Second parameter is the event type, combined with the channel.
// Third parameter is the control number number (0-119).
// Fourth parameter is the control value (0-127).

void controlChange(byte channel, byte control, byte value) {
  midiEventPacket_t event = {0x0B, 0xB0 | channel, control, value};
  MidiUSB.sendMIDI(event);
}
