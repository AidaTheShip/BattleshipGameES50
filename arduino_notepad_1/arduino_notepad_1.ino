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
  int state;
  char from;
  char to;
  int c1;
  int c2;
  bool push;
};

RECEIVE_DATA_STRUCTURE mydata;

int state;
// 0 -> choose warships, 1 -> the player 1 hits the opponent, 2 -> the player 2 hits the opponent, 3 -> end of the game
int choose_grid[8][8]; // grid of warships I am choosing, 0 -> no ship, 1 -> ship
int checker_grid[10][10]; // helper grid
int grid[8][8]; // currently discovered grid of the opponent, 0 -> no try, 1 -> miss, 2 -> ship


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

// checker function
bool checker(int c1, int c2){
  choose_grid[c1][c2] = 1;
  for (int i = 0; i < 10; i++){
    for (int j = 0; j < 10; j++){
      checker_grid[i][j] = 0;
    }
  }
  int num = 1;
  for (int i = 1; i < 9; i++){
    for (int j = 1; j < 9; j++){
      if (grid[i-1][j-1] != 0){
        checker_grid[i][j] = max(checker_grid[i-1][j], checker_grid[i][j-1]);
        if (checker_grid[i][j] == 0){
          checker_grid[i][j] = num;
          num ++;
        }
      }
    }
  }
  int cells[num];
  for (int i = 0; i < num; i++){
    cells[i] = 0;
  }
  int pix = 0;
  for (int i = 1; i < 9; i++){
      for (int j = 1; j < 9; j++){
        if (checker_grid[i][j] != 0) {
          cells[checker_grid[i][j]-1] ++;
          pix++;
        }
      }
  }
  choose_grid[c1][c2] = 0;
  return (num-1 <= 6 && pix <= 13);
}

//define a callback for key presses
TrellisCallback blink(keyEvent evt){
  if (state == 0){
    if (evt.bit.EDGE == SEESAW_KEYPAD_EDGE_RISING) {
      trellis.setPixelColor(evt.bit.NUM, Wheel(map(evt.bit.NUM, 0, X_DIM*Y_DIM, 0, 255))); //on rising
      int c1 = evt.bit.NUM / 8;
      int c2 = evt.bit.NUM % 8;
      if (choose_grid[c1][c2] == 1){
        choose_grid[c1][c2] = 0;
        trellis.setPixelColor(evt.bit.NUM, 0);
        Serial.println("Sending info");
        multicom_send(-1, 4, c1, c2, false);
      }
      else {
        if (checker (c1,c2)) {
          choose_grid[c1][c2] = 1;
          trellis.setPixelColor(evt.bit.NUM, 0xFFFFFF);
          Serial.println("Sending info");
          multicom_send(-1, 4, c1, c2, true);
          }
        else{
          trellis.setPixelColor(evt.bit.NUM, 0x000000);
        }
      }
    }
  }

  trellis.show();
  return 0;
}

void drawpad() {
  for(int y=0; y<Y_DIM; y++){
    for(int x=0; x<X_DIM; x++){
      if (grid[x][y] == 0)
        trellis.setPixelColor(x, y, 0x0000FF);
      if (grid[x][y] == 1)
        trellis.setPixelColor(x, y, 0xFFFFFF);
      if (grid[x][y] == 2)
        trellis.setPixelColor(x, y, 0xFF0000);
    }
  }
}

void offpad() {
  for(int y=0; y<Y_DIM; y++){
    for(int x=0; x<X_DIM; x++){
      trellis.setPixelColor(x, y, 0x000000);
    }
  }
}

void setup() {
  state=0;
  Serial.begin(9600);
  //while(!Serial) delay(1);
  ET.begin(details(mydata), &Serial);
  Serial.println("Starting Arduino 1 (notepad)");

  if(!trellis.begin()){
    Serial.println("failed to begin trellis");
    while(1) delay(1);
  }

  /* the array can be addressed as x,y or with the key number */
  for(int i=0; i<Y_DIM*X_DIM; i++){
      trellis.setPixelColor(i, Wheel(map(i, 0, X_DIM*Y_DIM, 0, 255))); //addressed with keynum
      trellis.show();
      delay(10);
  }
  
  for(int y=0; y<Y_DIM; y++){
    for(int x=0; x<X_DIM; x++){
      choose_grid[x][y] = 0;
      grid[x][y] = 0;
      //activate rising and falling edges on all keys
      trellis.activateKey(x, y, SEESAW_KEYPAD_EDGE_RISING, true);
      trellis.activateKey(x, y, SEESAW_KEYPAD_EDGE_FALLING, true);
      trellis.registerCallback(x, y, blink);
      trellis.setPixelColor(x, y, 0x000000); //addressed with x,y
      trellis.show(); //show all LEDs
      delay(10);
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
  if (mydata.state != -1){
    state = mydata.state;
    if (state == 1)
      drawpad();
    if (state == 2)
      offpad();
  }
  else {
    if (state == 1) {
      if (mydata.push){
        grid[c1][c2] = 2;
      }
      else{
        grid[c1][c2] = 1;
      }
      drawpad();
    }
    else{
      Serial.println("Unexpected input");
    }
  }
}

void multicom_send(int state, char to, int c1, int c2, bool push)
{
  mydata.state = state ;
  mydata.from = NODEID ;
  mydata.to = to ;
  mydata.c1 = c1 ; 
  mydata.c2 = c2 ;
  mydata.push = push ;
  ET.sendData() ; 
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
