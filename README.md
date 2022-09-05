# USB-MIDI-Organ-Conversion-With-Arduino-Leonardo
How to Convert an Organ to a USB MIDI Controller to use with Hauptwerk / GrandOrgue or any DAW
Since an Arduino Leonardo does not have enough pins to control even a single keyboard without using a diode matrix, shift registers are used as inputs for any extra pins required.
The Project uses a Baldwin 711 Organ for the project.
The organ has two 61 key keyboards, a 25 note pedal board, 30 switches and three preset buttons as well as an Analogue swell pedal.
Using shift registers the Leonardo (using this code), can handle:
 - Three 61 Note Keyboards.
 - 32 Note Pedal Board.
 - 32 Switches or Stops.
 - Two Analogue Pedals.
