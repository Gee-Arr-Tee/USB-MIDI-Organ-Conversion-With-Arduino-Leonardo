# USB-MIDI-Organ-Conversion-With-Arduino-Leonardo
How to Convert an Organ to a USB MIDI Controller to use with Hauptwerk / GrandOrgue or any DAW using 74HC165N Shift Registers.

All the Code and Eagle PCB and Schematic Files are provided, as well as connection diagrams and a Build of Materials (BOM).

Added the Gerber Files so anyone can get their own boards made.

Since an Arduino Leonardo does not have enough pins to control even a single 61 Note keyboard without using a diode matrix, shift registers are used as inputs for any extra pins required. 

A single board provides 32 Inputs which can be used for a Pedalboard and controlled by the Arduino with only 4 pins. 

Two boards can be linked to provide 64 Inputs, and controlled by only 4 pins also. 

Any object that can act as a switch can be used to trigger the Shift Registers and therefore play notes with, so it can be used on a hand made keyboard or group of switches etc., to construct a MIDI controller.

Using the boards to connect to an Arduino Leonardo, allows you to connect the devices directly to MIDI via the USB port, without having to change the bootloader. The Leonardo USB MIDI is detected by both Windows and Linux (if someone tries these on a Mac, I can update it on here). 

As a basis for for Project I used an old Baldwin 711 Organ, which has:
- Two 61 note keyboards.
- 25 Note Pedal Board.
- 30 Switches.
- Three Preset Buttons.
- One Analogue Pedal.

Using the Shift Registers PCBs, and this code with an Arduino Leonardo, it can handle:
 - Three 61 Note Keyboards.
 - 32 Note Pedal Board.
 - 32 Switches or Stops.
 - Nine Preset Buttons.
 - Two Analogue Pedals.
