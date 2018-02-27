#include "FastLED.h"

//#include <Dhcp.h>
//#include <Dns.h>
//#include <EthernetUdp.h>

#include <string.h>

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
#include <IS_Button.h>       //Implements an Interrupt Sensor (IS) to monitor the status of a digital input pin for button presses
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


// How many leds are in the strip?
#define NUM_LEDS 23

// Data pin that led data will be written out over
#define DATA_PIN 5

// This is an array of leds.  One item for each led in your strip.
CRGB leds[NUM_LEDS];

int turnOnLights = 0;
int colorOffset = 0;


// This function sets up the ledsand tells the controller about them
void setup() {

  //******************************************************************************************
  //  FASTLED setup
  //******************************************************************************************
  // sanity check delay - allows reprogramming if accidently blowing power w/leds
  delay(1000);


//Serial.print("checking turnOnLights in setup:");
 //Serial.println(turnOnLights);

 
  FastLED.addLeds<TM1803, DATA_PIN, RGB>(leds, NUM_LEDS);

  //FastLED.setBrightness(84);


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
  static st::EX_Switch              executor3(F("switch3"), PIN_SWITCH_3, LOW, true);

  //*****************************************************************************
  //  Configure debug print output from each main class
  //*****************************************************************************
  st::Everything::debug = true;
  st::Executor::debug = true;
  st::Device::debug = true;
  //st::PollingSensor::debug=true;
  st::InterruptSensor::debug = true;

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
  st::Everything::addExecutor(&executor3);
  //*****************************************************************************
  //Initialize each of the devices which were added to the Everything Class
  //*****************************************************************************
  st::Everything::initDevices();

}

void fadeall() {
  for (int i = 0; i < NUM_LEDS; i++) {
    leds[i].nscale8(250);
    //leds[i] = CRGB::Black;
  }
}

void alltoblack() {
  for (int i = 0; i < NUM_LEDS; i++) {
    //leds[i].nscale8(250);
    leds[i] = CRGB::Black;
  }
}




void loop() {



  //*****************************************************************************
  //Execute the Everything run method which takes care of "Everything"
  //*****************************************************************************
  st::Everything::run();

 
 //Serial.print("checking turnOnLights:");
 //Serial.println(turnOnLights);

  if (turnOnLights == 1 || turnOnLights == 2 || turnOnLights == 5) {
    // Move a single white led
    //for (int i = 0; i < NUM_LEDS; i = i + 1) {
    for(int i = (NUM_LEDS)-1; i >= 0; i--) {
      // Turn our current led on to white, then show the leds
      //leds[i] = CRGB::Green;
      //leds[i] = CRGB::Yellow;
      leds[i] = CHSV(colorOffset, 255, 255);

// lightning yellow is 215
// yellow 225

      if (turnOnLights == 5 ) {
         if (i > 0 ) {
           leds[i - 1 ] = CHSV(255, 255, 180); // 155 before
         } 
         if (i - 1 > 0 ) {
            leds[i - 2 ] = CHSV(colorOffset, 255, 120); // 155 before
         }
      } else {
         if (i > 0 ) {
           leds[i - 1 ] = CHSV(colorOffset, 255, 155); // 155 before
         } 
        
      }

 

      // Show the leds (only one of which is set to white, from above)
      FastLED.show();


      //fadeall();
      // Wait a little bit
      delay(14);


      // Turn our current led back to black for the next loop around
      leds[i] = CRGB::Black;
      if (i > 0 ) {
        leds[i - 1 ] = CRGB::Black;
      }
    }

  } else if (turnOnLights == 3 || turnOnLights == 4) {
    cylonEffect();
  }
}


//******************************************************************************************
//st::Everything::callOnMsgSend() optional callback routine.  This is a sniffer to monitor
//    data being sent to ST.  This allows a user to act on data changes locally within the
//    Arduino sktech withotu having to rely on the ST Cloud for time-critical tasks.
//******************************************************************************************
void callback(const String &msg)
{

  Serial.println(msg.indexOf("on"));
  if (msg.indexOf("on") > -1) {
    if (msg.indexOf("switch1") > -1 && turnOnLights == 0) {
      colorOffset = 0;
      turnOnLights = 1;
    } else if (msg.indexOf("switch2") > -1 && turnOnLights == 0) {
      colorOffset = 120;
      turnOnLights = 2;
    } else if (msg.indexOf("switch3") > -1 && turnOnLights == 0) {
      turnOnLights = 3;
    } else if (msg.indexOf("switch3") > -1 && turnOnLights == 1) {
      turnOnLights = 4;
    } else if (msg.indexOf("switch2") > -1 && turnOnLights == 1) {
      colorOffset = 225;
      turnOnLights = 5;
    }

    Serial.println("got an on signal :" + msg);
    
  } else if (msg.indexOf("off") > -1) { 
    turnOnLights = 0;
    alltoblack();
    FastLED.show();
  }
    //st::receiveSmartString("switch1 off");
    //st::receiveSmartString("switch2 off");

   //Uncomment if it weould be desirable to using this function
  //Serial.print(F("ST_Anything_Miltiples Callback: Sniffed data = "));
  Serial.println(msg);

  //TODO:  Add local logic here to take action when a device's value/state is changed

  //Masquerade as the ThingShield to send data to the Arduino, as if from the ST Cloud (uncomment and edit following line(s) as you see fit)
  //st::receiveSmartString("Put your command here!");  //use same strings that the Device Handler would send

  //st::receiveSmartString("Put your command here!");  //use same strings that the Device Handler would send

}
     

void cylonEffect() {
  static uint8_t hue = 0;
  Serial.print("x");
  // First slide the led in one direction
  for(int i = 0; i < NUM_LEDS; i++) {
    // Set the i'th led to red 
    leds[i] = CHSV(hue++, 255, 255);
    // Show the leds
    FastLED.show(); 
    // now that we've shown the leds, reset the i'th led to black
    // leds[i] = CRGB::Black;
    if (turnOnLights == 4) {
      alltoblack();
    } else {
       fadeall();
    }
    // Wait a little bit before we loop around and do it again
    delay(10);
  }
  Serial.print("x");

  // Now go in the other direction.  
  for(int i = (NUM_LEDS)-1; i >= 0; i--) {
    // Set the i'th led to red 
    leds[i] = CHSV(hue++, 255, 255);
    // Show the leds
    FastLED.show();
    // now that we've shown the leds, reset the i'th led to black
    // leds[i] = CRGB::Black;
    fadeall();
    // Wait a little bit before we loop around and do it again
    delay(10);
  }
    
}
 

