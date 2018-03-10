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

/* 
  IR Breakbeam sensor demo!

  https://learn.adafruit.com/ir-breakbeam-sensors/arduino
*/

#define LEDPIN 13

#define UNLOCK_PIN 3



  // Pin 13: Arduino has an LED connected on pin 13
  // Pin 11: Teensy 2.0 has the LED on pin 11
  // Pin  6: Teensy++ 2.0 has the LED on pin 6
  // Pin 13: Teensy 3.0 has the LED on pin 13

#define BOOKPIN 2

#define MAX_FAILED 3
//#define LOCKOUT_TIME 300000 // 5 minutes
#define LOCKOUT_TIME 3000

// variables will change:
//int sensorState = 0, lastState=0;         // variable for reading the pushbutton status

//void setup() {
//  // initialize the LED pin as an output:
//  pinMode(LEDPIN, OUTPUT);      
//  // initialize the sensor pin as an input:
//  pinMode(SENSORPIN, INPUT);     
//  digitalWrite(SENSORPIN, HIGH); // turn on the pullup
//  
//  Serial.begin(9600);
//}
//
//void loop(){
//  // read the state of the pushbutton value:
//  sensorState = digitalRead(SENSORPIN);
//
//  // check if the sensor beam is broken
//  // if it is, the sensorState is LOW:
//  if (sensorState == LOW) {     
//    // turn LED on:
//    digitalWrite(LEDPIN, HIGH);  
//  } 
//  else {
//    // turn LED off:
//    digitalWrite(LEDPIN, LOW); 
//  }
//  
//  if (sensorState && !lastState) {
//    Serial.println("Unbroken");
//  } 
//  if (!sensorState && lastState) {
//    Serial.println("Broken");
//  }
//  lastState = sensorState;
//}


#define LED0 11
#define LED1 12
#define LED2 14
#define LED3 15
#define LED4 16

int pinLEDS[] = {
  11,12,14,15,16
}; 

int timer = 100;           // The higher the number, the slower the timing.

int sensorPins[] = {
  6,5,7,9,8
};       // an array of pin numbers to which IR switches are attached
         // also the combination 2,1,3,5,4

int sensorStates[] = {
  0,0,0,0,0
}; 

int lastState = 0;
int lastBookState = 0;

int failedAttempts = 0;

int pinCount = sizeof(sensorStates)/sizeof(int);       // the number of pins (i.e. the length of the array)

// How many leds are in the strip?
#define NUM_LEDS 5

// Data pin that led data will be written out over
#define DATA_PIN 2

// This is an array of leds.  One item for each led in your strip.
CRGB leds[NUM_LEDS];


int findNextSlot(int pressed[] ) {
 //Serial.print("finding next slot: ");
 //Serial.print(pinCount);
 //Serial.print(":");

  for (int i = 0; i < pinCount; i++ ) {
    //Serial.print (pressed[i]);
    //  Serial.print(",");
    if (pressed[i] == 0 ) {
     // Serial.print(": returning " );
     // Serial.print(i);
      //Serial.print(" = ");
      //Serial.println(pressed[i]);
      return i;
    }
  }
  //Serial.print(": returning " );
  //Serial.println(-1);
  return -1;
  
}

int validateCode(int states[], int pins[] ) {
    for (int i =0; states[i] != pins[i];i++) {
      return 0;
    } 

    return 1;
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

  void blink(int count) {

    for (int i = count; i > 0; i--) {
    // turn the pin on:
        digitalWrite(LEDPIN, HIGH);
        delay(500);
         // turn the pin off:
        digitalWrite(LEDPIN, LOW);
    }
  }

  void resetLEDs() {
     for (int i =0; i < (sizeof(pinLEDS)/sizeof(int));i++) {
      //turn off
    }
  }
  /**
   * turn on the matching LED light
   *  type 0 indicates off
   *  type 1 - indicates good or valid combination (green or blue)
   *  type 2 - indicates the wrong combination (red or yellow)
   */
  void signalLED (int count, int type) {

  CRGB color = CRGB::Black;
  if (type = 1) {
     color  = CHSV(120, 255, 255); // blue
  } else {
    color = CHSV(225, 255, 255); //yellow
  }

  leds[count] = color;

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
  //  FASTLED setup
  //******************************************************************************************
  // sanity check delay - allows reprogramming if accidently blowing power w/leds
  delay(1000);
 
  FastLED.addLeds<TM1803, DATA_PIN, RGB>(leds, NUM_LEDS);

  //FastLED.setBrightness(84);
  
  // initialize the book pin as an input:
  pinMode(BOOKPIN, INPUT_PULLUP);     
 
  
  Serial.begin(9600); 
  // the array elements are numbered from 0 to (pinCount - 1).
  // use a for loop to initialize each pin as an output:
  for (int thisPin = 0; thisPin < pinCount; thisPin++) {
    pinMode(sensorPins[thisPin], INPUT);
    digitalWrite(sensorPins[thisPin], HIGH); // turn on the pullup

    pinMode(pinLEDS[thisPin], OUTPUT);
  }

  printState();

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

  
    
  // loop from the lowest pin to the highest:
  for (int thisPin = 0; thisPin < pinCount; thisPin++) {
    int sensorState = 0; 

    sensorState = digitalRead(sensorPins[thisPin]);

     if (sensorState == LOW && lastState == 0  ) {
        int next = findNextSlot(sensorStates);
        Serial.print("debug: ");
        Serial.println(next);
        if (next != -1 ) { 
          sensorStates[next] = sensorPins[thisPin];
        } else {
          failedAttempts += 1;
          String msg = "Invalid code too many, failed attempts ";
          Serial.println(msg + failedAttempts);
          //clearStates();
          blink(2);
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
          blink(1);
           Serial.println("Got the code unlocked !!!");
          failedAttempts = 0;
          clearStates();
       } else {
  
        failedAttempts +=1;
        String msg = " Invalid code, failed attempts ";
        
        Serial.println(msg + failedAttempts);
        clearStates();
        blink(2);
       }
     }

     lastBookState = bookState;
     

    if (failedAttempts >= MAX_FAILED) {
      blink(3);
      clearStates();
      String msg = "Max attempts lockout occured ";
      Serial.println(msg + failedAttempts);
      delay(LOCKOUT_TIME); // lock time 5 minutes
      failedAttempts = 0;
      
    }

  }

  
