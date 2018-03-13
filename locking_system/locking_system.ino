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

#define UNLOCK_PIN 12



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
  6,5,7,9,8
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

  void blink(int count) {

    for (int i = count; i > 0; i--) {
    // turn the pin on:
        digitalWrite(LEDPIN, HIGH);
        delay(500);
         // turn the pin off:
        digitalWrite(LEDPIN, LOW);
    }
  }


  /**
   * turn on the matching LED light
   *  type 0 indicates off
   *  type 1 - indicates good or valid combination (green or blue)
   *  type 2 - indicates the wrong combination (red or yellow)
   */
  void successLED(int success){
    // If success is 1, make all leds green, if not, make them red.
  
    if (success == 1){
      for (int index = 0; index < 5; index++){
        leds[index] = CHSV(122, 250, 255);
        //Green
      }
    FastLED.show();
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
      leds[j] = CHSV(112, 250, 63);
    }
    else if (sensorState == LOW && color == 3){
      //Yellow
      leds[j] = CHSV(63, 240, 92);
    }
    else if(sensorState == LOW && color == 4){
      //Orange
      leds[j] = CHSV(30, 240, 80);
    }
    else if(sensorState == LOW && color == 4) {
      //Violet
      leds[j] = CHSV(292, 255, 76);
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
  
  Serial.begin(9600); 
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
        successLED(0);
        
        blink(2);
       }
     }

     lastBookState = bookState;
     

    if (failedAttempts >= MAX_FAILED) {
      blink(3);
      clearStates();
      String msg = "Max attempts lockout occured ";
      Serial.println(msg + failedAttempts);
      successLED(0);
      delay(LOCKOUT_TIME); // lock time 5 minutes
      failedAttempts = 0;
      resetLeds();
    }

    
  int ledColor = 1;
  signalLED(ledColor);
  }

  
