/**
* This Arduino Project counts pulses from a power Meter using an S0 Interface and converts them to an analog value
* that can be used on an analouge kW meter connected to an analog pin
*
* For more information about the S0 interface: https://de.wikipedia.org/wiki/S0-Schnittstelle
* There are two different input pins for positive and negative values coming from the power meter.
*/


#define debounceDelay 100   //pulses may not occure faster than the debounce time


#define LEDPIN1 7  //pin where led is connected
#define OUTPIN 6   //analog pin for output

const byte ledPin = 13;

const byte interruptPinOne = 2;
const byte interruptPinTwo = 3;
volatile byte state = LOW;




volatile int pulseCount[2] = {0};                // Number of pulses, used to measure energy.
volatile int tempPulseCount[2] = {0};
volatile unsigned long pulseTime[2] = {0};       // Time between pulses, used to calculate power
volatile unsigned long lastTime[2] = {0};;       // Used to measure power.
volatile double power[2] = {0};                  // Used to store calculated power
boolean mutex = false;

int ppwh = 1;                                    // 1000 pulses/kwh = 1
//pulse per wh - Number of pulses per wh - found or set on the meter.


// This Sketch uses interrupts to count pulses Interrupt stuff

int lastInputPin;
uint8_t latest_interrupted_pin;
volatile long lastDebounceTime[2] = {0};     // last time the pin at [index] was triggered, used for debouncing


// led contais a mapping between which input pin must flash which led
// ins contains a mapping between which input pin must be used to
calculate which power value

//A0 A1 A2 A3 A4 A5
int led[20] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 3, 5, 6, 7, 8};
int ins[20] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 2, 3, 4, 5};


void setup() {
   pinMode(ledPin, OUTPUT);
   pinMode(OUTPIN, OUTPUT);

  // not needed if you have a pull down resistor
  //pinMode(interruptPinOne, INPUT_PULLUP);
  //pinMode(interruptPinTwo, INPUT_PULLUP);
   attachInterrupt(digitalPinToInterrupt(interruptPinOne), onPulse2, RISING);
   attachInterrupt(digitalPinToInterrupt(interruptPinTwo), onPulse3, RISING);
   // initialize serial communications at 9600 bps:
   Serial.begin(9600);
   Serial.println("Reset...");
   pinMode(LEDPIN1, OUTPUT); digitalWrite(LEDPIN1, HIGH);

   delay(500);
   digitalWrite(LEDPIN1, LOW);
}



void loop() {
  // there is no code here because everything this script does is triggerd by interrupts

}

// Functions


void onPulse2() {
   onPulse(2);
   lastInputPin = 2;
   digitalWrite(ledPin, LOW);
   digitalWrite(LEDPIN1, LOW);
}


void onPulse3() {
   onPulse(3);
   lastInputPin = 3;
   digitalWrite(ledPin, HIGH);
   digitalWrite(LEDPIN1, HIGH);

}


/*Callback function for a pulse event*/
void onPulse(int interruptedPin) {
   latest_interrupted_pin = interruptedPin -2 ;//we use pin 2 and 3 only atm
   int i = ins[latest_interrupted_pin];
   if ((millis() - lastDebounceTime[latest_interrupted_pin]) > debounceDelay)  //ensure the debounce delay was reached, only then this interrupt is counted
   { 
     lastDebounceTime[latest_interrupted_pin] = millis();
     lastTime[i] = pulseTime[i] ;
     pulseTime[i] = micros();
     if (mutex) {
       tempPulseCount[i]++;
     } else {
       pulseCount[i]++;
     }
     power[i] = int((3600000000.0 / (pulseTime[i] - lastTime[i])) / ppwh);
     outputToAnalog(power[i]);
   }

};

void  outputToAnalog(double pwr) {
   Serial.print("Power in kw is " );
   Serial.println(pwr );
//at 10000kw we want to provide 5v. 
   double currentPower =min(pwr,10000);
   double pwmValue =  255.0 / 10000 * currentPower;

   analogWrite(OUTPIN, pwmValue);

}


