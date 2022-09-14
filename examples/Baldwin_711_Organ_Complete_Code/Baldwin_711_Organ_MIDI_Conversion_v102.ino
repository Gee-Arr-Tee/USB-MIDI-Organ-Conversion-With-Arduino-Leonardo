/* 
 * Title:   Baldwin_711_Organ_MIDI_Conversion_v102
 * Author:  Gaetano Trombelli
 * Date:    13/9/2022
 * Description: Using 74HC165N Shift Registers to connect a 
 * Baldwin 711 Organ to an Arduino Leonardo to use with 
 * standard MIDI software via USB. It has been tested with,
 * Hauptwek, GrandOrgue, Ableton Live and other DAWs, 
 * and works like a standalone MIDI Workstation.  
 * 
 *  The Organ has:
 *   - Two 61 note keyboards.
 *   - 25 Note Pedal Board.
 *   - 30 Switches.
 *   - Three Preset Buttons.
 *   - One Analogue Pedal.
 */ 

// ============================================================
//                     Libraries to include
// ============================================================
/* 
 *  'ShiftIn.h' Shift Register Library to use with standard 
 *  74HC165N PISO Shift Registers.
 *  Reference:
 *  https://github.com/InfectedBytes/ArduinoShiftIn
 *  Adding: place in....libraries/ShiftIn/ShiftIn.h
 */
#include "ShiftIn.h" // 74HC165N Shift Register Library
/* 
 *  'MIDIUSB.h' MIDI USB Library required for Arduino ATmega32U4 
 *  compatibles, for example an Arduino Leonardo.
 *  References:
 *  https://github.com/arduino-libraries/MIDIUSB
 *  https://www.arduino.cc/reference/en/libraries/midiusb/  
 *  Adding: Sketch > Include Library > Add .ZIP Library 
 */
#include "MIDIUSB.h" // ATmega32U4 (Leonardo) MIDI Library

//========================================================================
//                    Variable Declarations
//========================================================================
/*  
 *  Shift Registers are needed to connect a Baldwin 711 to an Arduino Leonardo
 *  as it does not have enough pins to connect multiple keyboards, even
 *  if you were using diode matrices.
 *  
 *  We need for A Baldwin 711:  
 *  - Upper Swell 61 Note Keyboard:  ('8' x 8 = 64 Input Shift Registers). 
 *  - Three Preset Switches:(Use the 3 left over from the Swell keyboard).
 *  - Lower Great 61 Note Keyboard:  ('8' x 8 = 64 Input Shift Registers). 
 *  The 25 Note Pedal Board and the 30 Rocker Switches need 4 Shift 
 *  Registers each. In the code below however they are combined and use 
 *  8 Shift Registers and therefore only 4 Leonardo pins. This was done 
 *  in order to keep 4 pins free in case someone wanted to add another 
 *  61 Note Keyboard. See '61_Key_Keyboard_using_4_Analogue_Pins' in 
 *  the examples folder.
 *  - A 25 or 32 Note Pedal Board:   ('4' x 8 = 32 Input Shift Registers). 
 *  - 30 Rocker Switches:            ('4' x 8 = 32 Input Shift Registers). 
 *  - Volume / Swell Pedal:          (1 Analog Input).
 *  If you add a third keyboard, there is then one Analogue pin left,
 *  for a second Volume / Swell Pedal.
 *  ----------------------------------------------------------------------
 *  To use the ShiftIn.h Library, ShiftIn<n> <NAME> => 
 *  ShiftIn object called 'NAME' with 'n' 74HC165N Shift Registers 
 *  Daisy Chained n = 8 Maximum. Using the maximum of 8 for the organ
 *  means that I can use the minimum of 3 x 4 pins = 12 pins. 
 *  Just to be clear, 61/64 means 61 out of 64 pins available.
*/
ShiftIn<8> Swell;      // 61/64 Key Upper (Swell) Keyboard -+ (3 spare)
                       // 3 /64 Preset Buttons (Under Swell Keyboard)                           
ShiftIn<8> Great;      // 61/64 Key Lower (Great) Keyboard -+ (3 spare)
ShiftIn<8> PedalsStops;// 25/32 Key Pedalboard -------------+ (7 spare)
                       // 30/32 Stops or Buttons -----------+ (2 spare)

//  =================================================          
//          MIDI Variable Declarations
//  =================================================  
/*  
 *  Declaring Channels and lowest Notes here,
 *  allows you to reallocate a channel or key
 *  range for a keyboard easily.
 *  -------------------------------------------------
 *  Since MIDI Channels are based on Binary Numbers
 *  0000 = 0 = Channel 1 to 1111 = 15 = Channel 16 
 *  To allocate a Channel 'n': use the number n-1,  
 *  e.g., For Channel 3, use n-1 = 2.
 *  ------------------------------------------------------------
 *  Note: uint8_t = 'byte' = 8bit unsigned (non-negative) 
 *  integer, where    0 <= uint8_t <= 255
 *  Makes it easier to use this code for a different Arduino etc.
*/
//  =================================================          
//            MIDI Channel Declarations
//  ================================================= 
const uint8_t midiChannelSwell = 0;  // MIDI channel = (1)
const uint8_t midiChannelGreat = 1;  // MIDI channel = (2)
const uint8_t midiChannelPedals = 2; // MIDI channel = (3)
const uint8_t midiChannelStops = 3;  // MIDI channel = (4)
const uint8_t midiChannelPresets = 3;// MIDI channel = (4) also

/* 
 *  The note here is the Lowest note to start from:
 *  Most Keyboards start at C, so:
 *  For Reference: A 61 Note Keyboard:
 *  The range is usually      C(2) to  C(7)
 *  equivalent to MIDI NOTES: 36   to  96
 *  Pressing the Lowest  C(2) outputs MIDI Note 36
 *  Pressing the next note C#(2) outputs MIDI Note 37
 *  Pressing the Highest C(7) outputs MIDI Note 96
 *  C(1) = 24, C(2) = 36, C(3) = 48, C(4) = 60 (Middle C) etc.
 *  ---------------------------------------------------------
 *  Setting the start Notes to 36 will give the range above
 *  To make the Great an octave higher:
 *  const uint8_t lowestNoteGreat = 48  (+12 notes)
 *  To make the Pedal Board an octave lower:
 *  const uint8_t lowestNotePedals = 24 (-12 notes)
 */
//  =================================================          
//       Lowest MIDI Note for Keyboards & Pedals 
//  ================================================= 
const uint8_t lowestNoteSwell = 36;
const uint8_t lowestNoteGreat = 36;
const uint8_t lowestNotePedals = 36;

//======================================================================
// Set the Keyboard and Pedal Board 'fixed' MIDI Velocities
//======================================================================
/* 
 *  'velocityStandard' = 100 => Each key or Pedal sends this Velocity always.
 *  0 <= Velocity <= 127, NORMAL = 64, MAXIMUM = 127. 
 *  If you always want to send the MAXIMUM velocity possible,
 *  then just change this to velocityStandard = 127
 *  'velocityZero = 0' => ZERO Velocity sent when note is released 
*/
const uint8_t velocityStandard = 100;
const uint8_t velocityZero = 0;
/*
 *  Volume MIDI Control Change = 7, 
 *  MIDI CC List from: 
 *  https://professionalcomposers.com/midi-cc-list/
 *  Note that: uint16_t is used for volumePedal as Analogue read gives, 
 *  a value from 0 to 1023 = 1024 values, i.e. larger than uint8_t
*/
const uint8_t volPedalChannel = 0; // MIDI channel (1)
const uint8_t volPedalCC = 7; // Volume MIDI CC to be used = 7
uint16_t volPedalValue = 0;     // Current Volume Value
uint16_t volPedalPrevValue = 0; // Previous Volume Value


void setup() { // This runs once.
//===============================================================
//      Setup MIDI Baud Rate: Use 115200 for USB MIDI
//===============================================================
  Serial.begin(115200);
  
// ===============================================================     
//  Declare the Pins used by the Arduino Leonardo      
//  to controll the 74HC165N Shift Registers
// =============================================================== 
/*  
 *  74HC165N pins: PL-pin1,  CE-pin15,       OUT(Q7)-pin9, CP-pin2
 *  Declare pins:  pLoadPin, clockEnablePin, dataPin,      clockPin
 *  Connects to 74HC165N Shift Register Board OUTPUT:
 *  LD, CE, OUT, CLK in Order Below
 */
Swell.begin(2, 3, 4, 5);
Great.begin(6, 7, 8, 9);
PedalsStops.begin(10, 11, 12, 13);
}


void loop() { // Main Loop
    byte jitterAmount = 3; // Acceptable amount of the variation in potentiometer output.
 /*
  * Check to see if a keyboard key or a pedalboard note or button has changed:
  * Returns true if any key, pedal or button has, and goes to the function.
  */
if(Swell.update()) {    // If there is a change in Swell Keyboard or 3 Presets
   triggerMidiNotesSwell(); // Then execute this function
      }
   
  if(Great.update()) { // If there is a change in Great Keyboard
     triggerMidiNotesGreat(); // Then execute this function
        }
   
     if(PedalsStops.update()) { // If there is a change in the Pedals or Stops
        triggerMidiNotesPedalsStops(); // Then execute this function
           }
        	    delay(1); // requirement of ShiftIn.h Library
/* 
 *  The analogue Input has an output of 0-1023, but due to the pot value jittering, 
 *  we need to do some pre filtering, by only allowing the triggerMidiCCvolPedal();
 *  funtion to be called if the value has changed since last time by at least the 
 *  'jitterAmount' either way (Minus jitterAmount OR ( || ) Plus jitterAmount), 
 *  this was obtained by trial and error. Only then execute the 
 *  triggerMidiCCvolPedal() function, otherwise the pedal will constantly stream 
 *  data out of the MIDI port and use up CPU cycles for nothing!
 *  Reference: https://forum.arduino.cc/t/potentiometer-jitter/437767
*/
  volPedalValue = analogRead(4); // Read the Analogue Input.
  /* 
   *  Needed to add the '(volPedalValue > jitterAmount) &&' as the 
   *  Pedal fluctuates too much around the lowest Pedal position.
   */ 
  if ( (volPedalValue > jitterAmount) && ((volPedalValue < (volPedalPrevValue - jitterAmount)) || (volPedalValue > (volPedalPrevValue + jitterAmount))) )
    {
      triggerMidiCCvolPedal(); // Execute this only if 'volPedalValue' has changed by at least jitterAmount.
    }
   
 } // Main

 
void triggerMidiNotesSwell() {
    uint8_t noteSwell;
 /*
  * If a change is detected in the Upper 61 Note (Swell) Keyboard, 
  * then this sends the corresponding note to MIDI channel 1
  * If a change on the 3 Preset Buttons is detected
  * then this sends the corresponding note to MIDI channel 4
  */


  for(uint8_t i = 0; i < Swell.getDataWidth(); i++) {

      if(i < 61) { // Check if (i < 61)
      //====================================================================
      // Swell Shift Register PINS 01-61   ( i = 0 to 60 ) MIDI Channel 1
      //====================================================================
    
        if (Swell.pressed(i) == true) { // Swell keyboard key is Pressed
           noteSwell = (i + lowestNoteSwell);
           noteOn(midiChannelSwell, noteSwell, velocityStandard);  
           MidiUSB.flush(); // send immediately
            } // Note ON
 
        if (Swell.released(i) == true) { // Swell keyboard key is Released
           noteSwell = (i + lowestNoteSwell);
           noteOff(midiChannelSwell, noteSwell, velocityZero);  
           MidiUSB.flush(); // send immediately
            } // NOTE OFF

      } // i < 61
      
    else { // else (i > 61)
    //============================================================================
    // 3 Preset Buttons Shift Register PINS 62-64 ( i = 61 to 63 ) MIDI Channel 4
    // Same channel as the Stops / Buttons so all switches on MIDI Channel 4
    // Preset Buttons are mutually exclusive:'1' or '2' or '3' or 'C'(Cancel).
    //============================================================================

       if (Swell.pressed(i) == true) { // Preset Button is Pressed
          noteSwell = (i + lowestNoteSwell);
          noteOn(midiChannelPresets, noteSwell, velocityStandard);  
          MidiUSB.flush(); // send immediately
          } // Button ON
    
       if (Swell.released(i) == true) { // Preset Button is Cancelled
          noteSwell = (i + lowestNoteSwell);
          noteOff(midiChannelPresets, noteSwell, velocityZero);  
          MidiUSB.flush(); // send immediately
          } // Button OFF

    } // else i > 61
 
  } // for
   
} // triggerMidiNotesSwell



void triggerMidiNotesGreat() { 
    uint8_t noteGreat;
  /*
   * If a change is detected in the Lower 61 key (Great) Keyboard, 
   * then this sends the corresponding note to MIDI Channel 2
   */

  for(uint8_t i = 0; i < Great.getDataWidth(); i++) { 

   if (Great.pressed(i) == true) { // Great Keyboard key is Pressed
      noteGreat = (i + lowestNoteGreat);
      noteOn(midiChannelGreat, noteGreat, velocityStandard);
      MidiUSB.flush();
      }
 
   if (Great.released(i) == true) { // Great Keyboard key is Released
      noteGreat = (i + lowestNoteGreat);
      noteOff(midiChannelGreat, noteGreat, velocityZero);
      MidiUSB.flush();
      }
 
  } // for
   
} // triggerMidiNotesGreat


void triggerMidiNotesPedalsStops() {
    uint8_t notePedals;
    uint8_t noteStops;
 /*
  *           If a change is detected in the PedalBoard, 
  *     then this sends the corresponding note to MIDI channel 3
  *           If a change is detected in the Buttons / Stops, 
  *     then this sends the corresponding note to MIDI channel 4
  */


  for(uint8_t i = 0; i < PedalsStops.getDataWidth(); i++) { // MAIN for loop

      if(i < 32) { // Check if (i < 32)
      //====================================================================
      // Pedals Shift Register PINS 01-32   ( i = 0 to 31 ) MIDI Channel 3
      //====================================================================
       if (PedalsStops.pressed(i) == true) {  // PEDAL note is pressed
          notePedals = (i + lowestNotePedals);
          noteOn(midiChannelPedals, notePedals, velocityStandard); 
          MidiUSB.flush();
          }

       if (PedalsStops.released(i) == true) { // PEDAL note is released
          notePedals = (i + lowestNotePedals);
          noteOff(midiChannelPedals, notePedals, velocityZero);
          MidiUSB.flush();
          }
        
   } // Check if (i < 32)

    else { // else (i > 32)
    //============================================================================
    // 30 STOPS/Buttons Shift Register PINS 33-64 ( i = 32 to 63 ) MIDI Channel 4
    //============================================================================
        if (PedalsStops.pressed(i) == true) {  // STOP Pulled or Button pressed
           noteStops = (i +4); // if i = 32 => Lowest Note = C(2)
           noteOn(midiChannelStops, noteStops, velocityStandard); 
           MidiUSB.flush();
           }
         
        if (PedalsStops.released(i) == true) { // STOP Pushed or Button released       
           noteStops = (i +4); // if i = 32 => Lowest Note = C(2)
           noteOff(midiChannelStops, noteStops, velocityZero); 
           MidiUSB.flush();
           }

    }  // else (i > 32)

  } // for
   
} // triggerMidiNotesPedalsStops
    

void triggerMidiCCvolPedal() {
    uint8_t volPedalOutput; // Actual value sent to MIDI output
 /* 
  *  Usually divide by 8 to get MIDI CC range 0-127 if pot values are 
  *  0-1023 for 0 to 10k pot. But the 10k pot in the Baldwin 711 is 
  *  restricted by the organ pedal design to 30% rotation (0 to 3k only). 
  *  So we only divide by 2.55 to get the full range from ~ 0 to 126 
  *  (close enough). You get the '2.55' by trial and error, as it's 
  *  not exactly 30%. (30% of 1024)/2.55 = (0.3 * 1024)/2.55 is Approx 
  *  128 values. A 10k Linear pot works well and it's a common value.
  */ 
    
   volPedalOutput = (volPedalValue/2.55); 
   controlChange(volPedalChannel, volPedalCC, volPedalOutput); 
     
 /* 
  *  The following line 'Serial.println(volPedalOutput);' can be removed
  *  once the pot (or Pedal which has a pot) is determined to function 
  *  correctly. If you open the 'serial monitor' it will show the 
  *  instantaneous values of the pot, and you can see 'if' the values 
  *  randomly vary when untouched (jitters) and adjust the 'jitterAmount'.
 */

  Serial.println(volPedalOutput); // REMOVE AFTER CALIBRATION OF THE Pot/Pedal.
    
  MidiUSB.flush(); // send immediately
  volPedalPrevValue = volPedalValue;
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

void noteOn(uint8_t channel, uint8_t pitch, uint8_t velocity) {
  midiEventPacket_t noteOn = {0x09, 0x90 | channel, pitch, velocity};
  MidiUSB.sendMIDI(noteOn);
} // noteOn

void noteOff(uint8_t channel, uint8_t pitch, uint8_t velocity) {
  midiEventPacket_t noteOff = {0x08, 0x80 | channel, pitch, velocity};
  MidiUSB.sendMIDI(noteOff);
} // noteOff

//--------------------------------------------------------------------
//                  Control Change / CC Messages
//--------------------------------------------------------------------
// First parameter is the event type (0x0B = control change).
// Second parameter is the event type, combined with the channel.
// Third parameter is the control number number (0-119).
// Fourth parameter is the control value (0-127).

void controlChange(uint8_t channel, uint8_t control, uint8_t value) {
  midiEventPacket_t event = {0x0B, 0xB0 | channel, control, value};
  MidiUSB.sendMIDI(event);
} // controlChange
