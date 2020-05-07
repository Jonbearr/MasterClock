#include <elapsedMillis.h>
#include <MIDI.h>
using namespace midi;
MIDI_CREATE_INSTANCE(HardwareSerial,Serial,MIDI);

// https://www.sparkfun.com/datasheets/Components/LED/7-Segment/YSD-439AB4B-35.pdf

//////////  7 Segment Display Pins: //////////

// pins for the 4 digits of display: //

int pinLED_dig1 = 22;
int pinLED_dig2 = 24;
int pinLED_dig3 = 26;
int pinLED_dig4 = 28;

// pins for 7 segments of each digit: //

byte pinLED_A = 2;
byte pinLED_B = 3;
byte pinLED_C = 4;
byte pinLED_D = 5;
byte pinLED_E = 6;
byte pinLED_F = 7;
byte pinLED_G = 8;

byte pinLED_arm1 = 43;
byte pinLED_arm2 = 45;
byte pinLED_arm3 = 47;
byte pinLED_arm4 = 49;

byte pinButt_arm1 = 37;
byte pinButt_arm2 = 35;
byte pinButt_arm3 = 33;
byte pinButt_arm4 = 31;

byte pinButt_start = 12;
byte pinPWM1 = 50;
byte pinLED_tempo = 38;
byte pinSwitch1 = 51;
byte pinSwitch2 = 53;
byte pinEnc1 = 23;
byte pinEnc2 = 25;
byte pinSwitch_on = 10;


elapsedMillis dispCount;  // for multiplexing of 7 segment
elapsedMillis clockCount; // for main clock 
elapsedMillis startCount; // for start stop button
elapsedMillis encCount;   // for encoder
elapsedMillis arm1Count;  // 
elapsedMillis arm2Count;  //
elapsedMillis arm3Count;  //
elapsedMillis arm4Count;  //


byte PWMDiv = 8; // state of the division switch for PWM output, 1/8 notes to start 
int PWMStep = 0;
int LEDStep = 0;
int tempo = 120;
boolean playing = 0;

boolean lastA = 0;
boolean lastB = 0;

boolean arm1 = 0;
boolean arm2 = 0;
boolean arm3 = 0;
boolean arm4 = 0;


void setup() {
  // put your setup code here, to run once:
  pinMode(pinLED_dig1,OUTPUT);
  pinMode(pinLED_dig2,OUTPUT);
  pinMode(pinLED_dig3,OUTPUT);
  pinMode(pinLED_dig4,OUTPUT);
  
  pinMode(pinLED_A,OUTPUT);
  pinMode(pinLED_B,OUTPUT);
  pinMode(pinLED_C,OUTPUT);
  pinMode(pinLED_D,OUTPUT);
  pinMode(pinLED_E,OUTPUT);
  pinMode(pinLED_F,OUTPUT);
  pinMode(pinLED_G,OUTPUT);

  pinMode(pinButt_arm1,INPUT_PULLUP);
  pinMode(pinButt_arm2,INPUT_PULLUP);
  pinMode(pinButt_arm3,INPUT_PULLUP);
  pinMode(pinButt_arm4,INPUT_PULLUP);

  pinMode(pinButt_start,INPUT_PULLUP); // start/stop button
  pinMode(pinLED_tempo,OUTPUT);       // tempo LED
  pinMode(pinPWM1,OUTPUT);       // PWM clock output
  
  MIDI.begin(MIDI_CHANNEL_OMNI);  

  pinMode(pinSwitch1,INPUT_PULLUP);
  pinMode(pinSwitch2,INPUT_PULLUP);

  pinMode(pinEnc1,INPUT_PULLUP); // encoder
  pinMode(pinEnc2,INPUT_PULLUP); // encoder
  lastA = digitalRead(pinEnc1);
  lastB = digitalRead(pinEnc2);

  pinMode(pinSwitch_on,INPUT_PULLUP);
  
}

void loop() {

if( digitalRead(pinSwitch_on)==HIGH )
{
    // set all LEDs off
	digitalWrite(pinLED_dig1,LOW);
	digitalWrite(pinLED_dig2,LOW);
	digitalWrite(pinLED_dig3,LOW);
	digitalWrite(pinLED_dig4,LOW);
	
	digitalWrite(pinLED_tempo,LOW);
	
	digitalWrite(pinLED_arm1,LOW);
	digitalWrite(pinLED_arm2,LOW);
	digitalWrite(pinLED_arm3,LOW);
	digitalWrite(pinLED_arm4,LOW);
}
else
{

  if(digitalRead(pinButt_arm1)==LOW && arm1Count>250)
  {
    arm1 = !arm1;
	arm1Count = 0;//
  }
  if(digitalRead(pinButt_arm2)==LOW && arm2Count>250)
  {
    arm2 = !arm2;
	arm2Count = 0;	//
  }
  if(digitalRead(pinButt_arm3)==LOW && arm3Count>250)
  {
    arm3 = !arm3;
	arm3Count = 0;	//
  }
  if(digitalRead(pinButt_arm4)==LOW && arm4Count>250)
  {
    arm4 = !arm4;
	arm4Count = 0;	//
  }
  
  digitalWrite(pinLED_arm1,arm1);
  digitalWrite(pinLED_arm2,arm2);
  digitalWrite(pinLED_arm3,arm3);
  digitalWrite(pinLED_arm4,arm4);
  
////////////// read encoder & set tempo ///////////////////
  
  boolean A = digitalRead(pinEnc1);
  boolean B = digitalRead(pinEnc2);

  if (A != lastA && encCount>5)
  {
    if(A==B)
    {
      tempo = tempo-1;
    }
    else
    {
      tempo = tempo+1;
    }
	encCount = 0;
    lastA = A;
  }
  if ( B != lastB && encCount>5)
  {
    if(B == A)
    {
      tempo = tempo+1;
    }
    else
    {
      tempo = tempo-1;
    }
    encCount = 0;
    lastB = B;
  }
  
  MakeNum(tempo);

////////////////// do tempo maths ///////////////////////

  float mspb = tempo; // mspb = milliseconds per beat
  mspb = mspb/1.006;     // some correction factor ~ 1.006
  mspb = mspb/60;
  mspb = 1000/mspb;



  float clksync = mspb/24; // send 24 messages per quarter note
  if(clockCount>clksync)
  {
    MIDI.sendRealTime(Clock);
    clockCount = 0;
    LEDStep = LEDStep+1;
    PWMStep = PWMStep+1;
  }
  
/////////// Read the switch for PWM division /////////// 
  
  if(digitalRead(pinSwitch1)==LOW)
  {
    PWMDiv = 2; // quarter note
  }
  else if(digitalRead(pinSwitch2)==LOW)
  {
    PWMDiv = 8; // 16th note
  }
  else
  {
	PWMDiv = 4; // 8th note 
  }

//////////// Set PWM pin: ///////////////

  if(PWMStep<24/PWMDiv*1+1)
  {
	digitalWrite(pinPWM1,HIGH);
  }
  else if(PWMStep<24/PWMDiv*2+1)
  {
	digitalWrite(pinPWM1,LOW);
  }
  else if(PWMStep<24/PWMDiv*3+1)
  {
	digitalWrite(pinPWM1,HIGH);
  }
  else if(PWMStep<24/PWMDiv*4+1)
  {
	digitalWrite(pinPWM1,LOW);
  }
  else if(PWMStep<24/PWMDiv*5+1)
  {
	digitalWrite(pinPWM1,HIGH);
  }
  else if(PWMStep<24/PWMDiv*6+1)
  {
	digitalWrite(pinPWM1,LOW);
  }
  else if(PWMStep<24/PWMDiv*7+1)
  {
	digitalWrite(pinPWM1,HIGH);
  }
  else if(PWMStep<24/PWMDiv*8+1)
  {
	digitalWrite(pinPWM1,LOW);
  }

  if(PWMStep>23)
  {
    PWMStep = 0;
  }
  
/////////////// Set LED pin: /////////////////

   
  if(LEDStep<5)
  {
    digitalWrite(pinLED_tempo,HIGH);
  }
  else if(LEDStep<22)
  {
    digitalWrite(pinLED_tempo,LOW);
  }
  else if(LEDStep>23)
  {
    LEDStep = 0;
  }

/////////// handle start / stop button: ///////////


  if(startCount>250)  
  {
    if(digitalRead(pinButt_start)==LOW)
    {
      if(playing == 0)
      {
        MIDI.sendRealTime(Start);
        playing = 1;
        LEDStep = 0;
      }
      else if(playing == 1)
      {
        MIDI.sendRealTime(Stop);
        playing = 0;
      }
      startCount = 0;
    }
  }
}  
}


void MakeNum(int input)
{

  byte dispWait = 7;
  
  if(input<10)
  {
    digitalWrite(pinLED_dig4,HIGH);
    digitalWrite(pinLED_dig3,LOW);
    digitalWrite(pinLED_dig2,LOW);
    digitalWrite(pinLED_dig1,LOW);    
  }
  else if(input<100)
  {
    if(dispCount<dispWait)
    {
      digitalWrite(pinLED_dig4,LOW);
      digitalWrite(pinLED_dig3,HIGH);
      digitalWrite(pinLED_dig2,LOW);
      digitalWrite(pinLED_dig1,LOW);
      input = input/10;
    }
    else if(dispCount<dispWait*2)
    {
      digitalWrite(pinLED_dig4,HIGH);
      digitalWrite(pinLED_dig3,LOW);
      digitalWrite(pinLED_dig2,LOW);
      digitalWrite(pinLED_dig1,LOW);
      input = input-input/10*10;
    }
    else if(dispCount>dispWait*2)
    {
      dispCount = 0;
    }
  }
  else if(input<1000)
  {
    if(dispCount<dispWait)
    {
      digitalWrite(pinLED_dig4,LOW);
      digitalWrite(pinLED_dig3,LOW);
      digitalWrite(pinLED_dig2,HIGH);
      digitalWrite(pinLED_dig1,LOW);
      input = input/100;
    }
    else if(dispCount<dispWait*2)
    {
      digitalWrite(pinLED_dig4,LOW);
      digitalWrite(pinLED_dig3,HIGH);
      digitalWrite(pinLED_dig2,LOW);
      digitalWrite(pinLED_dig1,LOW);
      int foo = input-input/100*100;
      foo = foo/10;
      input = foo;
    }
    else if(dispCount<dispWait*3)
    {
      digitalWrite(pinLED_dig4,HIGH);
      digitalWrite(pinLED_dig3,LOW);
      digitalWrite(pinLED_dig2,LOW);
      digitalWrite(pinLED_dig1,LOW);
      int foo = input-input/100*100;
      foo = foo/10;
      foo = input-input/100*100;
      foo = foo-foo/10*10;
      input = foo;
    }
    else if(dispCount>dispWait*3)
    {
      dispCount = 0;
    }
  }

  if (input == 0)
  {
    digitalWrite(pinLED_A,LOW);
    digitalWrite(pinLED_B,LOW);
    digitalWrite(pinLED_C,LOW);
    digitalWrite(pinLED_D,LOW);
    digitalWrite(pinLED_E,LOW);
    digitalWrite(pinLED_F,LOW);
    digitalWrite(pinLED_G,HIGH);
  }
  else if (input == 1)
  {
    digitalWrite(pinLED_A,HIGH);
    digitalWrite(pinLED_B,LOW);
    digitalWrite(pinLED_C,LOW);
    digitalWrite(pinLED_D,HIGH);
    digitalWrite(pinLED_E,HIGH);
    digitalWrite(pinLED_F,HIGH);
    digitalWrite(pinLED_G,HIGH);
  }
  else if (input == 2)
  {
    digitalWrite(pinLED_A,LOW);
    digitalWrite(pinLED_B,LOW);
    digitalWrite(pinLED_C,HIGH);
    digitalWrite(pinLED_D,LOW);
    digitalWrite(pinLED_E,LOW);
    digitalWrite(pinLED_F,HIGH);
    digitalWrite(pinLED_G,LOW);
  }
  else if (input == 3)
  {
    digitalWrite(pinLED_A,LOW);
    digitalWrite(pinLED_B,LOW);
    digitalWrite(pinLED_C,LOW);
    digitalWrite(pinLED_D,LOW);
    digitalWrite(pinLED_E,HIGH);
    digitalWrite(pinLED_F,HIGH);
    digitalWrite(pinLED_G,LOW);
  }
  else if (input == 4)
  {
    digitalWrite(pinLED_A,HIGH);
    digitalWrite(pinLED_B,LOW);
    digitalWrite(pinLED_C,LOW);
    digitalWrite(pinLED_D,HIGH);
    digitalWrite(pinLED_E,HIGH);
    digitalWrite(pinLED_F,LOW);
    digitalWrite(pinLED_G,LOW);
  }
  else if (input == 5)
  {
    digitalWrite(pinLED_A,LOW);
    digitalWrite(pinLED_B,HIGH);
    digitalWrite(pinLED_C,LOW);
    digitalWrite(pinLED_D,LOW);
    digitalWrite(pinLED_E,HIGH);
    digitalWrite(pinLED_F,LOW);
    digitalWrite(pinLED_G,LOW);
  }
  else if (input == 6)
  {
    digitalWrite(pinLED_A,LOW);
    digitalWrite(pinLED_B,HIGH);
    digitalWrite(pinLED_C,LOW);
    digitalWrite(pinLED_D,LOW);
    digitalWrite(pinLED_E,LOW);
    digitalWrite(pinLED_F,LOW);
    digitalWrite(pinLED_G,LOW);
  }
  else if (input == 7)
  {
    digitalWrite(pinLED_A,LOW);
    digitalWrite(pinLED_B,LOW);
    digitalWrite(pinLED_C,LOW);
    digitalWrite(pinLED_D,HIGH);
    digitalWrite(pinLED_E,HIGH);
    digitalWrite(pinLED_F,HIGH);
    digitalWrite(pinLED_G,HIGH);
  }
  else if (input == 8)
  {
    digitalWrite(pinLED_A,LOW);
    digitalWrite(pinLED_B,LOW);
    digitalWrite(pinLED_C,LOW);
    digitalWrite(pinLED_D,LOW);
    digitalWrite(pinLED_E,LOW);
    digitalWrite(pinLED_F,LOW);
    digitalWrite(pinLED_G,LOW);
  }
  else if (input == 9)
  {
    digitalWrite(pinLED_A,LOW);
    digitalWrite(pinLED_B,LOW);
    digitalWrite(pinLED_C,LOW);
    digitalWrite(pinLED_D,LOW);
    digitalWrite(pinLED_E,HIGH);
    digitalWrite(pinLED_F,LOW);
    digitalWrite(pinLED_G,LOW);
  }
}
