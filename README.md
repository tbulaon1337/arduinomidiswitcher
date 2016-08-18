# arduinomidiswitcher
Repository for the Arduino Midi Switcher

I understand the struggle. Having to decide whether or not to shell out $400 for a midi switcher or pay off the months bills. I spent weeks trawling the internet for a how-to on building your own midi switcher. This is not to say that there wasn't anything out there, its just that there wasn't anything that was clear, concise and easy to follow. So I decided in the middle of July to make one myself. 

So here it is. My magnum opum. I present to the guitar/bass/keys/music community the OPEN SOURCE Arduino Midi Switcher. It is my hope that by releasing the code that more talented people will add features that I could never add and take it further then I ever could. 

Please go through the BOM (Bill Of Materials) to find out what you need and have a look at the wiring diagram to find out how to wire everything up

In order to ensure that my code will upload to your chosen Arduino platform please head over to https://www.arduino.cc/en/Main/Software and download the latest version. 

Also please ensure that you have downloaded and installed the midi library onto the Arduino IDE located here:
https://www.arduinomidilib.fortyseveneffects.com

Updates

Version 1.0 delyk SOURCE CODE
- Uses one RGB Led rather then 4 individual LEDs
- 4 Banks, 6 Presets per bank
- Up and Down bank footswitches (unlike vanila which has one to scroll through 4)

Version 1.0 VANILLA SOURCE CODE
- 4 Banks, 7 Presets per bank
- Sending Program Changes from 0-26
- Currently sending on Midi Channel 1

FUTURE UPDATES
- Bank LED + 16 Character Crystal LCD Display
- Midi Clock Send with Tap Tempo and BPM read out <- will need some help with this as this is beyond me
- Relay Bypass Switching 
- Real Time Patch Editing -> at the moment if you need to change the banks and sends you need to plug the arduino into a PC and make changes VIA arduino IDE. Real time patch editing would be MASSIVE as it would mean that people with no programming experience can just upload the code and get going. 

DONATE: 
If you love my code and feel like its changed your life and want to donate firstly
- Thank you very much! 
- All proceeds will go into the development and add more features and help me out with daily living expenses (hey at least I'm honest) 
- My paypal address for donations is susakekun12@gmail.com 
- If you would like to send donations via bit coins my address is 1MwvW6F2SygjP2QqQ2r7Rrjgv1DJHFMuYF
