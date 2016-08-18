//Here is where we assign our footswitches to our pins. Make sure you start on PIN 2. PIN 1 will be our MIDI Out
//If you are adding more switches, just add more #define SWITCH
#define SWITCH1 2
#define SWITCH2 3
#define SWITCH3 4
#define SWITCH4 5
#define SWITCH5 7
#define SWITCH6 8
#define SWITCH7 11

//Here is our BANK switch. I will comment out code to include a second DOWN footswitch
#define SBANKUP 13
//#define SBANKDOWN 14

#define MAX_BANKS 4

#define USE_COMMON_CATHODE_RGB_LED

//Here are our Bank indicator LEDs. If you need more Bank LEDS just add more #define BLED
#define BLED1 6
#define BLED2 9
#define BLED3 10
#define BLED4 12
