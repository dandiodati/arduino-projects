#include <noise.h>
#include <bitswap.h>
#include <fastspi_types.h>
#include <pixelset.h>
#include <fastled_progmem.h>
#include <led_sysdefs.h>
#include <hsv2rgb.h>
#include <fastled_delay.h>
#include <colorpalettes.h>
#include <color.h>
#include <fastspi_ref.h>
#include <fastspi_bitbang.h>
#include <controller.h>
#include <fastled_config.h>
#include <colorutils.h>
#include <chipsets.h>
#include <pixeltypes.h>
#include <fastspi_dma.h>
#include <fastpin.h>
#include <fastspi_nop.h>
#include <platforms.h>
#include <lib8tion.h>
#include <cpp_compat.h>
#include <fastspi.h>
#include <FastLED.h>
#include <dmx.h>
#include <power_mgt.h>

//******************************************************************************************
//******************************************************************************************
// SmartThings Library for Arduino Ethernet W5100 Shield
//******************************************************************************************
#include <SmartThingsEthernetW5100.h>    //Library to provide API to the SmartThings Ethernet W5100 Shield

//******************************************************************************************
// ST_Anything Library
//******************************************************************************************
#include <Constants.h>       //Constants.h is designed to be modified by the end user to adjust behavior of the ST_Anything library
#include <Device.h>          //Generic Device Class, inherited by Sensor and Executor classes
#include <Executor.h>        //Generic Executor Class, typically receives data from ST Cloud (e.g. Switch)
#include <Everything.h>      //Master Brain of ST_Anything library that ties everything together and performs ST Shield communications

//Implements an Interrupt Sensor (IS) and Executor to monitor the status of a digital input pin and control a digital output pin
//#include <IS_Button.h>       //Implements an Interrupt Sensor (IS) to monitor the status of a digital input pin for button presses
#include <EX_Switch.h>       //Implements an Executor (EX) via a digital output to a relay

//"RESERVED" pins for W5100 Ethernet Shield - best to avoid
#define PIN_4_RESERVED            4   //reserved by W5100 Shield on both UNO and MEGA
#define PIN_1O_RESERVED           10  //reserved by W5100 Shield on both UNO and MEGA
#define PIN_11_RESERVED           11  //reserved by W5100 Shield on UNO
#define PIN_12_RESERVED           12  //reserved by W5100 Shield on UNO
#define PIN_13_RESERVED           13  //reserved by W5100 Shield on UNO

#define PIN_SWITCH_1              5  //SmartThings Capability "Switch"
#define PIN_SWITCH_2              6  //SmartThings Capability "Switch"
#define PIN_SWITCH_3              7  //SmartThings Capability "Switch"

//******************************************************************************************
//W5100 Ethernet Shield Information ce:4d:c6:02:47:4b
//******************************************************************************************

byte mac[] = {0x06, 0x4d, 0xc6, 0x02, 0x47, 0x4b}; //MAC address, leave first octet 0x06, change others to be unique //  <---You must edit this line!
IPAddress ip(192, 168, 1, 133);               //Arduino device IP Address                   //  <---You must edit this line!
IPAddress gateway(192, 168, 1, 1);            //router gateway                              //  <---You must edit this line!
IPAddress subnet(255, 255, 255, 0);           //LAN sceubnet mask                             //  <---You must edit this line!
IPAddress dnsserver(192, 168, 1, 1);          //DNS server                                  //  <---You must edit this line!
const unsigned int serverPort = 8090;         // port to run the http server on

/// Smartthings hub information
IPAddress hubIp(192, 168, 1, 51);            // smartthings hub ip                         //  <---You must edit this line!
const unsigned int hubPort = 39500;           // smartthings hub port


/* 
  IR Breakbeam sensor demo!

  https://learn.adafruit.com/ir-breakbeam-sensors/arduino
*/

#define LEDPIN 13

#define UNLOCK_PIN A0



  // Pin 13: Arduino has an LED connected on pin 13
  // Pin 11: Teensy 2.0 has the LED on pin 11
  // Pin  6: Teensy++ 2.0 has the LED on pin 6
  // Pin 13: Teensy 3.0 has the LED on pin 13

#define BOOKPIN 2

#define MAX_FAILED 3
//#define LOCKOUT_TIME 300000 // 5 minutes
#define LOCKOUT_TIME 10000


// if successful indicates how long the lock will remain unlocked allowing people to enter.
// defaults to 30 secs
#define UNLOCKED_TIME 30000


int pinLEDS[] = {
  11,12,14,15,16
}; 

//all pins on correct order to match led lights
int orderedSensorPins[] = {
  5,6,7,8,9
};

int timer = 100;           // The higher the number, the slower the timing.

int sensorPins[] = {
  6,5,7,8,9
};       // an array of pin numbers to which IR switches are attached
         // also the combination 2,1,3,5,4

int sensorStates[] = {
  0,0,0,0,0
}; 

int lastState = 0;
int lastBookState = 0;

int failedAttempts = 0;

int pinCount = 5;       // the number of pins (i.e. the length of the array)

// How many leds are in the strip?
#define NUM_LEDS 5

// Data pin that led data will be written out over
#define DATA_PIN 3

// This is an array of leds.  One item for each led in your strip.
CRGB leds[NUM_LEDS];


int findNextSlot(int pressed[] ) {

  for (int i = 0; i < pinCount; i++ ) {
    
    if (pressed[i] == 0 ) {
      return i;
    }
  }
  Serial.print(": returning " );
  Serial.println(-1);
  return -1;
  
}

int validateCode(int states[], int pins[] ) {

    int i = 0;
    int success = 1;
    
    while (success == 1&& i < pinCount) {

      if(states[i] != pins[i]) {
        success = 0;
      } 
      i += 1;
     
    } 

    Serial.print("validate ");
    Serial.println(i);
    return success;
  }


int validate(int pin , int states[], int pins[] ) {
    if(states[pin] != pins[pin]) {
      return 0;
    } else
       return 1;
  }
  
  void clearStates() {
    for (int i =0; i < (sizeof(sensorStates)/sizeof(int));i++) {
      sensorStates[i] = 0;
    }
 
  }



  /**
   * turn on the matching LED light
   *  type 0 indicates off
   *  type 1 - indicates good or valid combination (green or blue)
   *  type 2 - indicates the wrong combination (yellow)
   *  type 3 - indicates the wrong combination (red)
   */
  void successLED(int success){
    // If success is 1, make all leds green, if not, make them red.
  
    if (success == 1){
      for (int index = 0; index < 5; index++){
        leds[index] = CHSV(175, 250, 240);
        //Green
        //leds[index] = CHSV(79, 248, 250);
        //Violet
      }
    FastLED.show();
    } else if (success = 2) {
      flashLEDS(CHSV(221, 240, 254), 2);
    }
    else {
      for (int j =0; j < 3;j++ ) {
        alltoblack();
        FastLED.show();
        delay(500);
        for (int index = 0; index < NUM_LEDS; index+=1){
          leds[index] = CHSV(0, 255, 255);
          //Red
        }
        FastLED.show();
        delay(500);
        
      }
    
    }
    
  }
  
  void flashLEDS(CHSV color, int count) {
    for (int j =0; j < count;j++ ) {
        alltoblack();
        FastLED.show();
        delay(500);
        for (int index = 0; index < NUM_LEDS; index+=1){
          leds[index] = color;
          //Red
        }
        FastLED.show();
        delay(500);
        
      }
  }

  void resetLeds() {

    alltoblack();
    FastLED.show();
  }

  
  void signalLED(int color) {

  for (int j = 0; j < pinCount; j++) {
    int sensorState = 0;
  
    
    sensorState = digitalRead(orderedSensorPins[j]);
   
    if (sensorState == LOW  && color == 1){
      //Blue
      leds[j] = CHSV(122, 230, 255);
      
    }
    else if (sensorState == LOW && color == 2){
      //Green
      leds[j] = CHSV(175, 250, 240);
    }
    else if (sensorState == LOW && color == 3){
      //Yellow
      leds[j] = CHSV(221, 240, 254);
    }
    else if(sensorState == LOW && color == 4){
      //Orange
      leds[j] = CHSV(243, 240, 240);
    }
    else if(sensorState == LOW && color == 5) {
      //Violet
      leds[j] = CHSV(79, 248, 250);
    }
    else {
      leds[j] = CHSV(255, 0, 0);
    }
  }
    
  FastLED.show();
 
  }


void alltoblack() {
  for (int i = 0; i < NUM_LEDS; i++) {
    //leds[i].nscale8(250);
    leds[i] = CRGB::Black;
  }
}


void setup() {

    //******************************************************************************************
  //  ST_Anything setup
  //Declare each Device that is attached to the Arduino
  //  Notes: - For each device, there is typically a corresponding "tile" defined in your
  //           SmartThings Device Hanlder Groovy code, except when using new COMPOSITE Device Handler
  //         - For details on each device's constructor arguments below, please refer to the
  //           corresponding header (.h) and program (.cpp) files.
  //         - The name assigned to each device (1st argument below) must match the Groovy
  //           Device Handler names.  (Note: "temphumid" below is the exception to this rule
  //           as the DHT sensors produce both "temperature" and "humidity".  Data from that
  //           particular sensor is sent to the ST Hub in two separate updates, one for
  //           "temperature" and one for "humidity")
  //         - The new Composite Device Handler is comprised of a Parent DH and various Child
  //           DH's.  The names used below MUST not be changed for the Automatic Creation of
  //           child devices to work properly.  Simply increment the number by +1 for each duplicate
  //           device (e.g. contact1, contact2, contact3, etc...)  You can rename the Child Devices
  //           to match your specific use case in the ST Phone Application.
  //******************************************************************************************
  //Executors
  static st::EX_Switch              executor1(F("switch1"), PIN_SWITCH_1, LOW, true);
  static st::EX_Switch              executor2(F("switch2"), PIN_SWITCH_2, LOW, true);
  
  //*****************************************************************************
  //  Configure debug print output from each main class
  //*****************************************************************************
  st::Everything::debug = true;
  st::Executor::debug = true;
  st::Device::debug = true;
  //st::PollingSensor::debug=true;
  //st::InterruptSensor::debug = true;

  //*****************************************************************************
  //Initialize the "Everything" Class
  //*****************************************************************************

  //Initialize the optional local callback routine (safe to comment out if not desired)
  st::Everything::callOnMsgSend = callback;

  //Create the SmartThings EthernetW5100 Communications Object
  //STATIC IP Assignment - Recommended
  st::Everything::SmartThing = new st::SmartThingsEthernetW5100(mac, ip, gateway, subnet, dnsserver, serverPort, hubIp, hubPort, st::receiveSmartString);

  //DHCP IP Assigment - Must set your router's DHCP server to provice a static IP address for this device's MAC address
  //st::Everything::SmartThing = new st::SmartThingsEthernetW5100(mac, serverPort, hubIp, hubPort, st::receiveSmartString);

  //Run the Everything class' init() routine which establishes Ethernet communications with the SmartThings Hub
  st::Everything::init();

  //*****************************************************************************
  //Add each sensor to the "Everything" Class
  //*****************************************************************************

  //*****************************************************************************
  //Add each executor to the "Everything" Class
  //*****************************************************************************
  st::Everything::addExecutor(&executor1);
  st::Everything::addExecutor(&executor2);
 
  //*****************************************************************************
  //Initialize each of the devices which were added to the Everything Class
  //*****************************************************************************
  st::Everything::initDevices();
  
  
  //******************************************************************************************
  //  FASTLED setup
  //******************************************************************************************
  // sanity check delay - allows reprogramming if accidently blowing power w/leds
  delay(1000);
 
  FastLED.addLeds<TM1803, DATA_PIN, RGB>(leds, NUM_LEDS);

  //FastLED.setBrightness(84);
  
  // initialize the book pin as an input:
  pinMode(BOOKPIN, INPUT_PULLUP);     

  // pin that sends unlock signal to mag locks when successful combination
  pinMode(UNLOCK_PIN, OUTPUT);
  
  //Serial.begin(115200); 
  // the array elements are numbered from 0 to (pinCount - 1).
  // use a for loop to initialize each pin as an output:
  for (int thisPin = 0; thisPin < pinCount; thisPin++) {
    pinMode(sensorPins[thisPin], INPUT);
    digitalWrite(sensorPins[thisPin], HIGH); // turn on the pullup

    pinMode(pinLEDS[thisPin], OUTPUT);
  }

  printState();
  successLED(1);

}

void printState() {
      Serial.println("states:");
   for (int thisPin = 0; thisPin < pinCount; thisPin++) {
      Serial.print("Pin Count ");
      Serial.print(thisPin);
      Serial.print(",source pin ");
      Serial.print(sensorPins[thisPin]);
      Serial.print(" = "); 
      Serial.println(digitalRead(sensorPins[thisPin]));
   }

      Serial.print("BOOK PIN STATE:");
      Serial.println(digitalRead(BOOKPIN));
  
    Serial.print("Sensors Read: ");
    for (int thisPin = 0; thisPin < pinCount-1; thisPin++) {
      Serial.print (sensorStates[thisPin]);
      Serial.print(",");
    }
    Serial.println(sensorStates[pinCount-1]);
}

void loop() {

  
  //*****************************************************************************
  //Execute the Everything run method which takes care of "Everything"
  //*****************************************************************************
  st::Everything::run();

    
  // loop from the lowest pin to the highest:
  for (int thisPin = 0; thisPin < pinCount; thisPin++) {
    int sensorState = 0; 
    
    delay(30);
    
    sensorState = digitalRead(sensorPins[thisPin]);

     if (sensorState == LOW && lastState == 0  ) {
        int next = findNextSlot(sensorStates);
        //Serial.print("debug: ");
        //Serial.println(next);
        if (next != -1 ) { 
          sensorStates[next] = sensorPins[thisPin];
        } else {
          failedAttempts += 1;
          String msg = "Invalid code too many, failed attempts ";
          Serial.println(msg + failedAttempts); 
          successLED(2);
          clearStates();
          //blink(2);
        }

        printState();

        lastState = sensorPins[thisPin];
     } else if (sensorState == HIGH && lastState == sensorPins[thisPin]){
      lastState = 0;
      Serial.println("Changing state to 0");
     }

  }
     int bookState = digitalRead(BOOKPIN);

     // can also check for bypass here then just always allow unlocking.
     if (bookState == HIGH && lastBookState == 0) {
      
       if (findNextSlot(sensorStates) == -1 && validateCode(sensorStates, sensorPins)) {
       
          // turn the pin on:
          //blink(1);
          Serial.println("Got the code unlocked !!!");
          digitalWrite(UNLOCK_PIN, HIGH);
          successLED(1);
          // wait for 30 seconds then lock mag locks again.
          delay(UNLOCKED_TIME);
          digitalWrite(UNLOCK_PIN, LOW);
          resetLeds();
          failedAttempts = 0;
          clearStates();
          
       } else {
  
        failedAttempts +=1;
        String msg = " Invalid code, failed attempts ";
        
        Serial.println(msg + failedAttempts);
        clearStates();
        successLED(2);
        
        //blink(2);
       }
     }

     lastBookState = bookState;
     

    if (failedAttempts >= MAX_FAILED) {
      //blink(3);
      clearStates();
      String msg = "Max attempts lockout occured ";
      Serial.println(msg + failedAttempts);
      successLED(3);
      delay(LOCKOUT_TIME); // lock time 5 minutes
      failedAttempts = 0;
      resetLeds();
      
    }

    
  int ledColor = 1;
  signalLED(ledColor);
  }

//******************************************************************************************
//st::Everything::callOnMsgSend() optional callback routine.  This is a sniffer to monitor
//    data being sent to ST.  This allows a user to act on data changes locally within the
//    Arduino sktech withotu having to rely on the ST Cloud for time-critical tasks.
//******************************************************************************************
void callback(const String &msg)
{

  //Serial.println(msg.indexOf("on"));
//  if (msg.indexOf("on") > -1) {
//    if (msg.indexOf("switch1") > -1 && turnOnLights == 0) {
//      colorOffset = 0;
//      turnOnLights = 1;
//    } else if (msg.indexOf("switch2") > -1 && turnOnLights == 0) {
//      colorOffset = 120;
//      turnOnLights = 2;
//    } else if (msg.indexOf("switch3") > -1 && turnOnLights == 0) {
//      turnOnLights = 3;
//    } else if (msg.indexOf("switch3") > -1 && turnOnLights == 1) {
//      turnOnLights = 4;
//    } else if (msg.indexOf("switch2") > -1 && turnOnLights == 1) {
//      colorOffset = 225;
//      turnOnLights = 5;
//    }

    Serial.println("got an on signal :" + msg);
    
  //} 
    //st::receiveSmartString("switch1 off");
    //st::receiveSmartString("switch2 off");

   //Uncomment if it weould be desirable to using this function
  //Serial.print(F("ST_Anything_Miltiples Callback: Sniffed data = "));


  //TODO:  Add local logic here to take action when a device's value/state is changed

  //Masquerade as the ThingShield to send data to the Arduino, as if from the ST Cloud (uncomment and edit following line(s) as you see fit)
  //st::receiveSmartString("Put your command here!");  //use same strings that the Device Handler would send

  //st::receiveSmartString("Put your command here!");  //use same strings that the Device Handler would send

}
  

