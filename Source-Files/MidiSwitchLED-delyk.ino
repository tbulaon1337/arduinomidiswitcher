//Arduino MIDI Controller version 1.0. Written by Timothy Bulaon 16/08/2016
//I will try and make this code as easy to read as I possibly can. I once was someone who had no idea how to program and know the struggle. Throughout the code I will do my best to annotate each and every step
//With this program you should be able to build an 8 switch controller with 4 LEDs. The 7 footswitches are what will send MIDIs and the 8th footswitch will toggle through the 4 banks giving a total of 28 presets.
//of course this can be expanded through use of the code and using an arduino with more digital inputs.
//Out of the goodness of my heart I have decided to do all musician/tinkerers/diyers a massive favour and release this as an open source code. Please feel free to use it as your leisure. All I ask is that
//you don't use my code without my permission. Let's not be those guys hey?
//If you scroll through the code you will find similiar code with this -> // I have added these codes just to give you an example if you wish to expand functionality
//Just take out those // and the code will be active again
//Make sure you have the midi library included in your IDE which can be found here http://arduinomidilib.fortyseveneffects.com/
//I have added the project to gitHub. Which means YES! If you want to make your additions official then go ahead! I would love for more talented people to do great things with my code :)
//
//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License.
//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.
//
//You should have received a copy of the GNU General Public License
//along with this program.  If not, see <http://www.gnu.org/licenses/>.

//DON'T FORGET! Sketch -> Include Library -> MIDI
#include <MIDI.h>
#include <midi_Defs.h>
#include <midi_Message.h>
#include <midi_Namespace.h>
#include <midi_Settings.h>
#include "MidiSwitchSettings.h"

//This will setup our bank switching
int bank = 1;

//Set MIDI Output. REMEBER to leave PIN 1 on your arduino for TX out (midi out)
MIDI_CREATE_INSTANCE(HardwareSerial, Serial, midiOut);

void setup()
{
  //Here we will set up switches. If you added more switches just follow the commented out examples
  pinMode(SWITCH1, INPUT);
  pinMode(SWITCH2, INPUT);
  pinMode(SWITCH3, INPUT);
  pinMode(SWITCH4, INPUT);
  pinMode(SWITCH5, INPUT);
  pinMode(SWITCH6, INPUT);
  pinMode(SWITCH7, INPUT);

  //BANK SWITCHES
  pinMode(SBANKUP, INPUT);
#ifdef SBANKDOWN
  pinMode(SBANKDOWN, INPUT);
#endif

  // Setup MIDI sends
  Serial.begin(31250);
}

void loop()
{
  //BANK SWITCH SETUP
  if (digitalRead(SBANKUP) == HIGH)
  {
    delay(250);
    bank++;
    if (bank == MAX_BANKS + 1)
    {
      bank = 1;
    }
  }
  //if(digitalRead(SBANKDOWN) == HIGH)
  //{
  //  delay(250);
  //  bank--;
  //    if(bank == 0)
  //      {
  //        bank = MAX_BANKS + 1;
  //      }
  //}

  //BANK LED INDICATOR SETUP
  //Okay, so whats happening here is that your arduino will monitor the value of bank. everytime you step on a footswitch you are adding + 1 (or -1 if you have a bank down)
  //to the value of bank e.g. if value of bank = 1 then stepping on a footswitch will add one giving it a value of 2. The arduino will look at the value of bank and then choose
  //what to do next by looking at the value next to case. So if bank = 3 then the arduino will look for case 3 and execute code according to what is in the case (in our case turn
  //on a certain LED whilst turning off the rest)
  switch (bank)
  {
    //BANK LED 1
    case 1:
#ifdef USE_COMMON_CATHODE_RGB_LED
      bankLed(25, 0, 0);
#else
      bankLed(25, 0, 0, 0);
#endif
      break;

    //BANK LED 2
    case 2:
#ifdef USE_COMMON_CATHODE_RGB_LED
      bankLed(0, 25, 0);
#else
      bankLed(0, 25, 0, 0);
#endif
      break;

    //BANK LED 3
    case 3:
#ifdef USE_COMMON_CATHODE_RGB_LED
      bankLed(0, 0, 25);
#else
      bankLed(0, 0, 25, 0);
#endif
      break;

    //BANK LED 4
    case 4:
#ifdef USE_COMMON_CATHODE_RGB_LED
      bankLed(0, 25, 25);
#else
      bankLed(0, 25, 25, 0);
#endif
      break;
  }

  //This is where our switches start to send MIDI program changes. If you have added more banks check EXAMPLE 1 for the example. If you have added more switches just copy the first bank
  //code starting from switch(bank) finishing on the 2nd }. Just remember to change the SWITCH number
  if (digitalRead(SWITCH1) == HIGH)
  {
    //BANK BEGIN
    switch (bank)
    {
      //BANK 1
      case 1:
        midiOut.sendProgramChange(0, 1); //midiOut.sendProgramChange(Program(or patch)number, Midi Channel) For this footswitch if you step on this and you are on bank 1 it will send Program Change 0 on midi channel 1
        delay(250);
        break;
      //BANK2
      case 2:
        midiOut.sendProgramChange(7, 1);
        delay(250);
        break;
      //BANK3
      case 3:
        midiOut.sendProgramChange(14, 1);
        delay(250);
        break;
      //BANK4
      case 4:
        midiOut.sendProgramChange(21, 1);
        delay(250);
        break;
    }
  }
  //SWITCH 2
  if (digitalRead(SWITCH2) == HIGH)
  {
    //BANK BEGIN
    switch (bank)
    {
      //BANK1
      case 1:
        midiOut.sendProgramChange(1, 1);
        delay(250);
        break;
      //BANK2
      case 2:
        midiOut.sendProgramChange(8, 1);
        delay(250);
        break;
      //BANK3
      case 3:
        midiOut.sendProgramChange(15, 1);
        delay(250);
        break;
      //BANK4
      case 4:
        midiOut.sendProgramChange(22, 1);
        delay(250);
        break;
    }
  }
  //SWITCH 3
  if (digitalRead(SWITCH3) == HIGH)
  {
    //BANK BEGIN
    switch (bank)
    {
      //BANK1
      case 1:
        midiOut.sendProgramChange(2, 1);
        delay(250);
        break;
      //BANK2
      case 2:
        midiOut.sendProgramChange(9, 1);
        delay(250);
        break;
      //BANK3
      case 3:
        midiOut.sendProgramChange(16, 1);
        delay(250);
        break;
      //BANK4
      case 4:
        midiOut.sendProgramChange(23, 1);
        delay(250);
        break;
    }
  }
  //SWITCH 4
  if (digitalRead(SWITCH4) == HIGH)
  {
    //BANK BEGIN
    switch (bank)
    {
      //BANK1
      case 1:
        midiOut.sendProgramChange(3, 1);
        delay(250);
        break;
      //BANK2
      case 2:
        midiOut.sendProgramChange(10, 1);
        delay(250);
        break;
      //BANK3
      case 3:
        midiOut.sendProgramChange(17, 1);
        delay(250);
        break;
      //BANK4
      case 4:
        midiOut.sendProgramChange(24, 1);
        delay(250);
        break;
    }
  }
  //SWITCH 5
  if (digitalRead(SWITCH5) == HIGH)
  {
    //BANK BEGIN
    switch (bank)
    {
      //BANK1
      case 1:
        midiOut.sendProgramChange(4, 1);
        delay(250);
        break;
      //BANK2
      case 2:
        midiOut.sendProgramChange(11, 1);
        delay(250);
        break;
      //BANK3
      case 3:
        midiOut.sendProgramChange(18, 1);
        delay(250);
        break;
      //BANK4
      case 4:
        midiOut.sendProgramChange(25, 1);
        delay(250);
        break;
    }
  }
  //SWITCH 6
  if (digitalRead(SWITCH6) == HIGH)
  {
    //BANK BEGIN
    switch (bank)
    {
      //BANK1
      case 1:
        midiOut.sendProgramChange(5, 1);
        delay(250);
        break;
      //BANK2
      case 2:
        midiOut.sendProgramChange(12, 1);
        delay(250);
        break;
      //BANK3
      case 3:
        midiOut.sendProgramChange(19, 1);
        delay(250);
        break;
      //BANK4
      case 4:
        midiOut.sendProgramChange(26, 1);
        delay(250);
        break;
    }
  }
  //SWITCH 7
  if (digitalRead(SWITCH7) == HIGH)
  {
    //BANK BEGIN
    switch (bank)
    {
      //BANK1
      case 1:
        midiOut.sendProgramChange(6, 1);
        delay(250);
        break;
      //BANK2
      case 2:
        midiOut.sendProgramChange(13, 1);
        delay(250);
        break;
      //BANK3
      case 3:
        midiOut.sendProgramChange(20, 1);
        delay(250);
        break;
      //BANK4
      case 4:
        midiOut.sendProgramChange(27, 1);
        delay(250);
        break;
    }
  }
}

#ifdef USE_COMMON_CATHODE_RGB_LED
/**
  Sets the brightness of the four LEDs.

  @param led1 LED1's brightness.  0 - 255
  @param led2 LED2's brightness.  0 - 255
  @param led3 LED3's brightness.  0 - 255
*/
void bankLed(int led1, int led2, int led3)
{
  // Use this function to determine how you want to light up your LEDs.
  analogWrite(BLED1, led1);
  analogWrite(BLED2, led2);
  analogWrite(BLED3, led3);
}
#else
/**
  Sets the brightness of the four LEDs.

  @param led1 LED1's brightness.  0 - 255
  @param led2 LED2's brightness.  0 - 255
  @param led3 LED3's brightness.  0 - 255
  @param led4 LED4's brightness.  0 - 255
*/
void bankLed(int led1, int led2, int led3, int led4)
{
  // Use this function to determine how you want to light up your LEDs.
  analogWrite(BLED1, led1);
  analogWrite(BLED2, led2);
  analogWrite(BLED3, led3);
  analogWrite(BLED4, led4);
}
#endif

