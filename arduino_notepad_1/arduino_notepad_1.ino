#include <EasyTransfer.h>
#include "Adafruit_NeoTrellis.h"
#include <SoftwareSerial.h>

#define Y_DIM 8 //number of rows of key
#define X_DIM 8 //number of columns of keys

#define NODEID 1
#define NUMNODES 4

// create object
EasyTransfer ET;

int state;
// 0 -> choose warships, 1 -> the player 1 hits the opponent, 2 -> the player 2 hits the opponent, 3 -> end of the game
// normal grid
int grid[8][8];
// your ships to keep track of whether your opponent hits you 
int checker_grid[10][10];
// grid that stores where the other opponent hit you
int choose_grid[8][8];

struct RECEIVE_DATA_STRUCTURE{
  int state;
  char from;
  char to;
  int c1;
  int c2;
  bool push;
};

RECEIVE_DATA_STRUCTURE mydata;


//create a matrix of trellis panels
Adafruit_NeoTrellis t_array[Y_DIM/4][X_DIM/4] = {
  
  { Adafruit_NeoTrellis(0x2F), Adafruit_NeoTrellis(0x2E) },

  { Adafruit_NeoTrellis(0x31), Adafruit_NeoTrellis(0x30) }
  
};

//pass this matrix to the multitrellis object
Adafruit_MultiTrellis trellis((Adafruit_NeoTrellis *)t_array, Y_DIM/4, X_DIM/4);

// Input a value 0 to 255 to get a color value.
// at the very beginning starting
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

// checks if there is the correct number of ships - only during the countdown
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
      if (choose_grid[i-1][j-1] != 0){
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
  // if we are choosing ships
  if (state == 0){
    if(evt.bit.EDGE == SEESAW_KEYPAD_EDGE_RISING) {
        int c1 = evt.bit.NUM / 8;
        int c2 = evt.bit.NUM % 8;
        // if the player has already chosen the pixel, it will turn of again, and the information 
        // will be transfered
        if (choose_grid[c1][c2] == 1){
          choose_grid[c1][c2] = 0;
          trellis.setPixelColor(evt.bit.NUM, 0);
          Serial.println("Sending info");
          // information sent to the Display
          multicom_send(-1, 4, c1, c2, false);
        }
        else {
          if (checker(c1,c2)) {
            choose_grid[c1][c2] = 1;
            trellis.setPixelColor(evt.bit.NUM, 0xFFFFFF);
            Serial.println("Sending info");
            // state -1 means that we are just sending the coordinates!
            multicom_send(-1, 4, c1, c2, true);
            }
          else{
            trellis.setPixelColor(evt.bit.NUM, 0x000000);
          }
        }
      }
  }
  // if it is player ones turn to hit
  else if (state == 1) {
    if(evt.bit.EDGE == SEESAW_KEYPAD_EDGE_RISING) {
        int c1 = evt.bit.NUM / 8;
        int c2 = evt.bit.NUM % 8;
        if (grid[c1][c2] == 0) {
          trellis.setPixelColor(evt.bit.NUM, 0x00FF00);
          multicom_send(-1, 3, c1, c2, true);
        }
        else if (grid[c1][c2]==1){
          trellis.setPixelColor(evt.bit.NUM, 0xFFFFFF);
        }
        else if (grid[c1][c2]==2){
          trellis.setPixelColor(evt.bit.NUM, 0xFF0000);
        } 
    }
  }
  // showing the trellis after everything is done
  trellis.show();
  return 0;
}

// draws on the pad depending if the opponent was hit or not
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
  Serial.println("redrawn");
  trellis.show();
}

void offpad() {
  for(int y=0; y<Y_DIM; y++){
    for(int x=0; x<X_DIM; x++){
      trellis.setPixelColor(x, y, 0x000000);
    }
  }
  Serial.println("set off");
  trellis.show();
}

void setup() {
  state = 0;
  // general set upeof the trellis
  Serial.begin(9600);
  //while(!Serial) delay(1);
  ET.begin(details(mydata), &Serial);
  Serial.println("Starting Arduino 1 (notepad)");

  if(!trellis.begin()){
    Serial.println("failed to begin trellis");
    while(1) delay(1);
  }

  /* the array can be addressed as x,y or with the key number, drawing the beginning sequence */
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

// constantly looping to update between the data
void loop() {
  if (state!=3){
  multicom_update();
  trellis.read();
  delay(20);
  }
  
}

// receiving the data, based oon the state, we do different things -- going to the functions we wrote
void multicom_receive()
{
  delay(100);
// if there is something supposed to happen
  if (mydata.state != -1){
    // updating the state with what is received
    Serial.println("Change of state");
    state = mydata.state;
    Serial.println(state);
    // if the player is supposed to be hitting
    if (state == 1) {
      drawpad();
      Serial.println("Drawpad");
    }
    // if the other player is supposed to hit, our lights turn off
    if (state == 2) {
      offpad();
      Serial.println("Offpad");
    }
  }
  // if there is nothing supposed to happen
  else {
   if (state == 1) {
      Serial.println("lmao");
      if (mydata.push){
        // we are updating the grid based on whether we were hit or not
        grid[mydata.c2][mydata.c1] = 2;
      }
      else{
        grid[mydata.c2][mydata.c1] = 1;
      }
      // drawing the pad if the state is not 
      drawpad();
      delay(1000);
      for (int i = 0; i < 10; i++){
      multicom_send(2, 2, 0, 0, false);
      delay(5);
      }
      for (int i = 0; i < 10; i++){
      multicom_send(2, 3, 0, 0, false);
      delay(5);
      }
      for (int i = 0; i < 10; i++){
      multicom_send(2, 4, 0, 0, false);
      delay(5);
      }
      state = 2;
      Serial.println("all states changed to 2");
      offpad();
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
