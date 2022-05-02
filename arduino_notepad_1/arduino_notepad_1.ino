#include <EasyTransfer.h>
#include "Adafruit_NeoTrellis.h"
#include <SoftwareSerial.h>

#define Y_DIM 8 //number of rows of key
#define X_DIM 8 //number of columns of keys

#define NODEID 1
#define NUMNODES 4

// create object
EasyTransfer ET;

struct RECEIVE_DATA_STRUCTURE{
  //put your variable definitions here for the data you want to receive
  //THIS MUST BE EXACTLY THE SAME ON THE OTHER ARDUINO
  char from;
  char to;
  int coor;
  bool rise;
  bool first;
};

RECEIVE_DATA_STRUCTURE mydata;


//create a matrix of trellis panels
Adafruit_NeoTrellis t_array[Y_DIM/4][X_DIM/4] = {
  
  { Adafruit_NeoTrellis(0x2E), Adafruit_NeoTrellis(0x2F) },

  { Adafruit_NeoTrellis(0x31), Adafruit_NeoTrellis(0x30) }
  
};

//pass this matrix to the multitrellis object
Adafruit_MultiTrellis trellis((Adafruit_NeoTrellis *)t_array, Y_DIM/4, X_DIM/4);

// Input a value 0 to 255 to get a color value.
// The colors are a transition r - g - b - back to r.
uint32_t Wheel(byte WheelPos) {
  if(WheelPos < 85) {
   return seesaw_NeoPixel::Color(WheelPos * 3, 255 - WheelPos * 3, 0);
  } else if(WheelPos < 170) {
   WheelPos -= 85;
   return seesaw_NeoPixel::Color(255 - WheelPos * 3, 0, WheelPos * 3);
  } else {
   WheelPos -= 170;
   return seesaw_NeoPixel::Color(0, WheelPos * 3, 255 - WheelPos * 3);
  }
  return 0;
}

//define a callback for key presses
TrellisCallback blink(keyEvent evt){
  
  if(evt.bit.EDGE == SEESAW_KEYPAD_EDGE_RISING) {
    trellis.setPixelColor(evt.bit.NUM, Wheel(map(evt.bit.NUM, 0, X_DIM*Y_DIM, 0, 255))); //on rising
     multicom_send(4, evt.bit.NUM, true);
  }
  else if(evt.bit.EDGE == SEESAW_KEYPAD_EDGE_FALLING) {
    trellis.setPixelColor(evt.bit.NUM, 0); //off falling
    multicom_send(4, evt.bit.NUM, false);
  }
  trellis.show();
  return 0;
}

void setup() {
  Serial.begin(9600);
  //while(!Serial) delay(1);
  ET.begin(details(mydata), &Serial);
  Serial.println("starting Arduino 1 (notepad)");

  if(!trellis.begin()){
    Serial.println("failed to begin trellis");
    while(1) delay(1);
  }

  /* the array can be addressed as x,y or with the key number */
  for(int i=0; i<Y_DIM*X_DIM; i++){
      trellis.setPixelColor(i, Wheel(map(i, 0, X_DIM*Y_DIM, 0, 255))); //addressed with keynum
      trellis.show();
      delay(50);
  }
  
  for(int y=0; y<Y_DIM; y++){
    for(int x=0; x<X_DIM; x++){
      //activate rising and falling edges on all keys
      trellis.activateKey(x, y, SEESAW_KEYPAD_EDGE_RISING, true);
      trellis.activateKey(x, y, SEESAW_KEYPAD_EDGE_FALLING, true);
      trellis.registerCallback(x, y, blink);
      trellis.setPixelColor(x, y, 0x000000); //addressed with x,y
      trellis.show(); //show all LEDs
      delay(50);
    }
  }

}

void loop() {
  multicom_update();
  trellis.read();
  delay(20);
  
}

void multicom_receive()
{
  Serial.println(mydata.from);
  Serial.println(mydata.to);
  Serial.println(mydata.coor);
  Serial.println(mydata.rise);
  Serial.println(mydata.first);
}

void multicom_send(char to, int coor, bool rise)
{
  mydata.from = NODEID;
  mydata.to = to;
  mydata.coor = coor ; 
  mydata.rise = rise ;
  mydata.first = true ;
  ET.sendData(); 
}

void multicom_update()
{
 while(ET.receiveData())
  {
     if (mydata.to == NODEID) {
        Serial.println("Node 1 received info");
        multicom_receive();
     }
  }
}
