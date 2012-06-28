//teensy++ code for rotary encoder controller
//tomash ghz
//www.tomashg.com

#include "ledRings.h" // library for LED rings from http://code.google.com/p/ledrings/
#include <Encoder.h>
#include <Bounce.h>

/********| BEGIN CONFIGURATION |********/
#define NUM_LED_RINGS  4 // The number of LED Ring boards

#define LED_RING_OE    10 // Output Enable pin of the board
#define LED_RING_SDI   7 // Data pin of the board
#define LED_RING_CLK   8 // Clock pin of the board
#define LED_RING_LE    9 // Latch pin of the board

#define LED_RING_1   0 // This is entirely up to you.
#define LED_RING_2   1 // It's used to refer to the individual led boards by name.
#define LED_RING_3   2 // Use whatever works for your purpose, or none at all :)
#define LED_RING_4   3 // Use whatever works for your purpose, or none at all :)
/*********| END CONFIGURATION |*********/

//debugging will print out in serial instead of midi
boolean debugging=false;

//channel number for midi messages
int midiChannel=5;

ledRings ring(LED_RING_SDI, LED_RING_CLK, LED_RING_LE, LED_RING_OE, NUM_LED_RINGS);
Encoder knob1(0, 5);
Encoder knob2(1, 14);
Encoder knob3(18, 23);
Encoder knob4(19, 20);

int knobShift[4]={0,0,0,0};
int ledValues[8];

int buttons[4]={6,15,22,21};
Bounce *buttonState[4];

//values for the encoder leds
unsigned int sequence[3][16] = {
    {0x0, 0x1,    0x2,    0x4,    0x8,   0x10,  0x20,  0x40,  0x80, 0x100, 0x200, 0x400, 0x800, 0x1000, 0x2000, 0x4000},
    {0x0, 0x1,    0x3,    0x7,    0xf,   0x1f,  0x3f,  0x7f,  0xff, 0x1ff, 0x3ff, 0x7ff, 0xfff, 0x1fff, 0x3fff, 0x7fff},
    {0x0, 0x7fff, 0x3ffe, 0x1ffc, 0xff8, 0x7f0, 0x3e0, 0x1c0, 0x80, 0x1c0, 0x3e0, 0x7f0, 0xff8, 0x1ffC, 0x3ffe, 0x7fff},
  };

long knobPosition[4]={-999,-999,-999,-999};

void setup() {
  
    //DEBUG
  if(debugging){
    Serial.begin(9600);//open serail port
  }
  else{
    Serial.begin(31250);//open midi
    usbMIDI.setHandleControlChange(myCC);
  }
  
  //set button pins to input
  for(int i=0;i<4;i++){
    pinMode(buttons[i], INPUT_PULLUP);
    buttonState[i]= new Bounce(buttons[i],10);
  }
  //digitalWrite(6, LOW);
  pinMode(6, OUTPUT);
  //play led animation
  ledInitialize();
}

void loop(){

  long newKnob[4];
  
  //recieve MIDI messages
  if(!debugging){
    usbMIDI.read();
  }
  
  //read the new knob values
  newKnob[0]=knob1.read();
  newKnob[1]=knob2.read();
  newKnob[2]=knob3.read();
  newKnob[3]=knob4.read();
  
  for(int i=0; i<4;i++){
    
  //recieve MIDI messages
  if(!debugging){
    usbMIDI.read();
  }
  
    //read buttons
    if(buttonState[i]->update()){//state changed
      
      if(buttonState[i]->read()==LOW){//is pressed

      }else{ // was released
        ring.blinkRing(i,2,40);
        
        //flip the shift state
        if(knobShift[i]==0)
          knobShift[i]=1;
        else
          knobShift[i]=0;
      }
    } 
  
    //knob moved
    if(newKnob[i]!=knobPosition[i]){
      
      //did it move forward?
      if(newKnob[i]>knobPosition[i]){
         midiCC(0,i);
        }
      else{
        midiCC(1,i);
      }
    }
    
    knobPosition[i]=newKnob[i];
  }
  
    //recieve MIDI messages
  if(!debugging){
    usbMIDI.read();
  }
  
  //update the led displays
  printLeds();
  
  
  
}

void ledInitialize(){
  
  ring.allRingsOff();
  
  for(int i=0;i<4;i++){
    for (byte j=0;j<ring.num_leds_arc;j++){
      ring.setRingLed(i, j, HIGH);
      delay(25);
      ring.allRingsOff();
    }
  }
  
}

void printLeds(){
  
  ring.allRingsOff();
  
  for (int i=0;i<4;i++){
    ring.setRingState(i, sequence[1][ledValues[i*2+knobShift[i]]]);
    
    if(knobShift[i]==1)
      ring.setRingBottomLed(i,HIGH);
  }
}

void midiCC(int n, int v){
  //send a midi cc message
  if(n==0){
  
    if(debugging){
      //Serial.print(v*2+knobShift[v]);
      //Serial.println(" >>");
    }else{
      usbMIDI.sendControlChange(v*2+knobShift[v], 65, midiChannel);
    }
  }else{
    if(debugging){
      //Serial.print(v*2+knobShift[v]);
      //Serial.println(" <<");
    }else{
      usbMIDI.sendControlChange(v*2+knobShift[v], 63, midiChannel);
    }
  }
}

void myCC(byte channel,byte  number,byte value){
  
  if(channel==midiChannel){
    if ((number>=0)&&(number<=7)){
      ledValues[number%8]=map(value,0,127,0,15);
    }
  }
  
}