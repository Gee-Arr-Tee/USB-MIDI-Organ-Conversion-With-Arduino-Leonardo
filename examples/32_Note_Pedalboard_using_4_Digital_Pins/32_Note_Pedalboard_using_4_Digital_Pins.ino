/* 
 * Title:   32_Note_Pedalboard_using_4_Digital_Pins
 * Author:  Gaetano Trombelli
 * Date:    8/9/2022
 * Description: Using 4 x 74HC165N Shift Registers to connect a 32 note 
 * (or 25 note) Pedalboard to an Arduino Leonardo to make a MIDI 
 * Pedalboard Controller, using only 4 DIGITAL Pins.
 * You could in fact use anything to switch an input:
 * +5v --> to any shift register pin will trigger a note (see schematic).     
 */ 


// ============================================================
//                     Libraries to include
// ============================================================
/* 
 *  Shift Register Library to use with 74HC165N Shift Registers
 *  Reference:
 *  https://github.com/InfectedBytes/ArduinoShiftIn
 *  Adding: place in....libraries/ShiftIn/ShiftIn.h
 */
#include "ShiftIn.h" // 74HC165N Shift Register Library
/* 
 *  MIDI USB Library required for Arduino ATmega32U4 
 *  compatible for example an Arduino Leonardo.
 *  References:
 *  https://github.com/arduino-libraries/MIDIUSB
 *  https://www.arduino.cc/reference/en/libraries/midiusb/  
 *  Adding: Sketch > Include Library > Add .ZIP Library 
 */
#include "MIDIUSB.h" // ATmega32U4 (Leonardo) MIDI Library

//===========================================================
//                    Variable Declarations
//===========================================================
/*  
 *  Shift Registers needed to connect a 32 Note Pedalboard:
 *  Need (32 Inputs = '4' x 8-Input Shift Registers). 
 *  ShiftIn<n> <NAME> => ShiftIn object called 'NAME' with 'n'      
 *  74HC165N Shift Registers Daisy Chained. 
 *  e.g., PBoard using 8 shift registers, and using
 *  32 out of 32 Inputs for PedalBoard leaves 0 spare Inputs.
 *  OR 25 out of 32 Inputs leaves 7 spare Inputs.
*/
ShiftIn<4> PBoard;

//  =================================================          
//          MIDI Variable Declarations
//  =================================================  
/*  
 *  Declaring Channels and lowest Notes here,
 *  allows you to reallocate a channel or pedal
 *  range for a pedalboard easily.
 *  -------------------------------------------------
 *  Since MIDI Channels are based on Binary Numbers
 *  0000 = 0 = Channel 1 to 1111 = 15 = Channel 16 
 *  To allocate a Channel 'n': use the number n-1,  
 *  e.g., For Channel 3, use n-1 = 2.
*/
//  =================================================          
//            MIDI Channel Declarations
//  ================================================= 
const byte midiChannelPBoard = 0;  // MIDI channel = (1)

/*  
 *  The note here is the Lowest note to start from:
 *  Most Pedalboards start at C, so:
 *  For Reference: A 32 note Pedalboard:
 *  The range is usually      C(2) to  C(4)
 *  equivalent to MIDI NOTES: 36   to  60
 *  Pressing the Lowest  C(2) outputs MIDI Note 36
 *  Pressing the next note C#(2) outputs MIDI Note 37
 *  Pressing the Highest C(4) outputs MIDI Note 60
 *  C(1) = 24, C(2) = 36, C(3) = 48, C(4) = 60 (Middle C) etc.
 *  ---------------------------------------------------------
 *  Setting the start Notes to 36 will give the range above
 *  To make the PBoard an octave higher C(3) to C(5):
 *  const byte lowestNotePBoard = '48'  (+12 notes)
 *  To make the PBoard an octave lower C(1) to C(3):
 *  const byte lowestNotePBoard = '24'  (-12 notes)
 */
//  =================================================          
//       Lowest MIDI Note for Pedalboard 
//  ================================================= 
const byte lowestNotePBoard = 36;


//======================================================================
// Set the Pedalboard 'fixed' MIDI Velocities
//======================================================================
/*  
 *  'velocityStandard' = 100 => Each pedal sends this Velocity when played.
 *  0 <= Velocity <= 127, NORMAL = 64, MAXIMUM = 127. 
 *  If you always want to send the MAXIMUM velocity possible,
 *  then just change this to velocityStandard = 127
 *  'velocityZero = 0' => ZERO Velocity sent when note is released 
*/
const byte velocityStandard = 100;
const byte velocityZero = 0;


void setup() { // Run this once.
//===============================================================
//      Setup MIDI Baud Rate: Use 115200 for USB MIDI
//===============================================================
  Serial.begin(115200);
  
// ===============================================================     
//  Declare the Pins used by the Arduino Leonardo      
//  to controll the 74HC165N Shift Registers
// =============================================================== 
/*  
 *  74HC165N pins: PL-pin1,  CE-pin15,       Q7-pin9, CP-pin2
 *  Declare pins:  pLoadPin, clockEnablePin, dataPin, clockPin
 *  --------------------------------------------------------------
 *  Connects to: 
 *  74HC165N Pins (1, 15, 9, 2)
 *  Leonardo Pins (2, 3,  4, 5)
 */
PBoard.begin(2, 3, 4, 5);
}


void loop() { // Main Program Loop
/* 
 * Constantly checks to see if a pedal has changed, and if true 
 * the executes the function 'triggerMidiNotesPBoard();'
 */
      if(PBoard.update()) { // If there is a change in PBoard
        triggerMidiNotesPBoard(); // Then execute this function
        }
          
	      delay(1); // requirement of ShiftIn Library.
  } 
  
 
void triggerMidiNotesPBoard() { 
//==============================================================
// If a change is detected in the 32 (PBoard) Pedalboard, 
// then this sends the corresponding note to MIDI Channel 1
//==============================================================

byte notePBoard;

  for(byte i = 0; i < PBoard.getDataWidth(); i++) { 

    if (PBoard.pressed(i) == true) { // PBoard pedal is Pushed
      notePBoard = (i + lowestNotePBoard);
      noteOn(midiChannelPBoard, notePBoard, velocityStandard);
      MidiUSB.flush(); // send immediately
    } 
 
    if (PBoard.released(i) == true) { // PBoard pedal is Released
      notePBoard = (i + lowestNotePBoard);
      noteOff(midiChannelPBoard, notePBoard, velocityZero);
      MidiUSB.flush(); // send immediately
    } 
 
  } 
   
} 

//====================================================================
//     -----     ATmega32U4 (Leonardo) MIDIUSB Library     -----
//====================================================================

//--------------------------------------------------------------------
//                        Note ON / Note OFF
//--------------------------------------------------------------------
// First parameter is the event type (0x09 = note on, 0x08 = note off).
// Second parameter is note-on/note-off, combined with the channel.
// Channel can be anything between 0-15. Typically reported to the user as 1-16.
// Third parameter is the note number (48 = middle C).
// Fourth parameter is the velocity (64 = normal, 127 = fastest).

void noteOn(byte channel, byte pitch, byte velocity) {
  midiEventPacket_t noteOn = {0x09, 0x90 | channel, pitch, velocity};
  MidiUSB.sendMIDI(noteOn);
} // noteOn

void noteOff(byte channel, byte pitch, byte velocity) {
  midiEventPacket_t noteOff = {0x08, 0x80 | channel, pitch, velocity};
  MidiUSB.sendMIDI(noteOff);
} // noteOff
