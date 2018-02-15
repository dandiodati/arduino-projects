/* 
  IR Breakbeam sensor demo!

  https://learn.adafruit.com/ir-breakbeam-sensors/arduino
*/

#define LEDPIN 13
  // Pin 13: Arduino has an LED connected on pin 13
  // Pin 11: Teensy 2.0 has the LED on pin 11
  // Pin  6: Teensy++ 2.0 has the LED on pin 6
  // Pin 13: Teensy 3.0 has the LED on pin 13

#define BOOKPIN 9

#define MAX_FAILED = 3;
#defin LOCK_TIME = 300;

// variables will change:
//int sensorState = 0, lastState=0;         // variable for reading the pushbutton status

void setup() {
  // initialize the LED pin as an output:
  pinMode(LEDPIN, OUTPUT);      
  // initialize the sensor pin as an input:
  pinMode(SENSORPIN, INPUT);     
  digitalWrite(SENSORPIN, HIGH); // turn on the pullup
  
  Serial.begin(9600);
}

void loop(){
  // read the state of the pushbutton value:
  sensorState = digitalRead(SENSORPIN);

  // check if the sensor beam is broken
  // if it is, the sensorState is LOW:
  if (sensorState == LOW) {     
    // turn LED on:
    digitalWrite(LEDPIN, HIGH);  
  } 
  else {
    // turn LED off:
    digitalWrite(LEDPIN, LOW); 
  }
  
  if (sensorState && !lastState) {
    Serial.println("Unbroken");
  } 
  if (!sensorState && lastState) {
    Serial.println("Broken");
  }
  lastState = sensorState;
}





int timer = 100;           // The higher the number, the slower the timing.
int sensorPins[] = {
  7,6,5,8
};       // an array of pin numbers to which LEDs are attached
         // also the combination 3, 2, 1, 4

int sensorStates[] = {
  0,0,0,0
}; 

int pinCount = 4;           // the number of pins (i.e. the length of the array)

void setup() {
  
  // initialize the book pin as an input:
  pinMode(BOOKPIN, INPUT);     
  
  
  Serial.begin(9600); 
  // the array elements are numbered from 0 to (pinCount - 1).
  // use a for loop to initialize each pin as an output:
  for (int thisPin = 0; thisPin < pinCount; thisPin++) {
    pinMode(sensorPins[thisPin], INPUT);
    digitalWrite(sensorPins[thisPin], HIGH); // turn on the pullup
  }
}


int findNextSlot(int pressed[] ) {
  for (int i = 0; i < pressed.length; i++ ) {
    if (pressed[i] == 0 ) {
      return i;
    }

    return -1;
  }
}

void loop() {
  // loop from the lowest pin to the highest:
  for (int thisPin = 0; thisPin < pinCount; thisPin++) {
    int sensorState = 0, lastState=0; 

    sensorState = digitalRead(ledPins[thisPin]);

     if (sensorState == LOW) {
        int next = findNextSlot(sensorStates);
        if (next != -1 ) { 
          sensorStates[next] = thisPin+1;
        } else {
          failedAttempt += 1;
        }
     } 

  }
     int bookState = digitalRead(BOOKPIN);

     if (bookState == LOW && findNextSlot(sensorStates) == -1 && validCode(sensorStates, ledPins)) {
        // turn the pin on:
        digitalWrite(ledPins[thisPin], HIGH);
        delay(3000);
         // turn the pin off:
        digitalWrite(ledPins[thisPin], LOW);

        failedAttempts = 0;
        clearStates();
     }
     

    if (failedAttempts >= MAX_FAILED) {
      delay(300,000); // lock time 5 minutes
      clearStates();
    }

  }

  int validCode(int states[], int pins[] ) {
    for (int i =0; states[i] != pins[i];i++) {
      return 0;
    } 

    return 1;
  }

  void clearStates() {
    for (int i =0; i < sensorStates.length; ;i++) {
      sensorStates[i] = 0;
    }

 
}
