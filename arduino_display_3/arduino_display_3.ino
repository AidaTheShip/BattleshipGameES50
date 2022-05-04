#include <EasyTransfer.h>

// you can use softserial like this
#include <SoftwareSerial.h>
#include <RGBmatrixPanel.h>

#define NODEID 3
#define NUMNODES 4; 

#define CLK  8 
#define OE   9
#define LAT 10
#define A   A0
#define B   A1
#define C   A2

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
int grid[8][8]; //my grid, 0 -> sea, 1 -> ship, 2 -> miss, 3 -> hit 

RGBmatrixPanel matrix(A, B, C, CLK, LAT, OE, false);

void timer() {
  for(uint8_t x = 0; x < 32; x++){
    for(uint8_t y = 0; y < 16; y++){
      matrix.drawPixel(x, y, matrix.Color333(7, 0, 0));
    }
    for (int j = 0; j < 100; j++) {
      multicom_update();
      delay(10);
    }
  }
}

void drawgrid() {
  for(uint8_t x = 16; x < 32; x++){
    for(uint8_t y = 0; y < 16; y++){
      if (grid[(x-16)/2][y/2] == 1){
        matrix.drawPixel(x, y, matrix.Color333(7, 7, 7));
      }
      else if (grid[(x-16)/2][y/2] == 0){
        matrix.drawPixel(x, y, matrix.Color333(0, 0, 7));        
      }
      else if (grid[(x-16)/2][y/2] == 2){
        matrix.drawPixel(x, y, matrix.Color333(0, 0, 0));        
      }
      else if (grid[(x-16)/2][y/2] == 3){
        matrix.drawPixel(x, y, matrix.Color333(7, 0, 0));        
      }
    }
  }
}

void setup() {
  state=0;
  Serial.begin(9600);
  matrix.begin();
  ET.begin(details(mydata), &Serial);
  Serial.println("starting Arduino 3 (display)");
  for (int i = 0; i < 8; i++){
    for (int j = 0; j < 8; j++){
      grid[i][j]=0;
    }
  }
    for(uint8_t x = 0; x < 32; x++){
      for(uint8_t y = 0; y < 16; y++){
        matrix.drawPixel(x, y, matrix.Color333(7, 7, 7));
      }
  }
  timer ();
  for(uint8_t x = 0; x < 16; x++){
    for(uint8_t y = 0; y < 16; y++){
      matrix.drawPixel(x, y, matrix.Color333(0, 0, 0));
    }
  }
  drawgrid();
  state=1;
}

void loop() {
  multicom_update();
}

void multicom_update()
{
 while(ET.receiveData())
  {
     if (mydata.to == NODEID) {
        Serial.println("Node 3 received info");
        multicom_receive();
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

void multicom_receive()
{
  delay(100);
  if (mydata.state != -1){
    state=mydata.state;
    // draw state here
  }
  else {
    if (state==0){
      if (mydata.push){
        grid[mydata.c2][mydata.c1] = 1;
      }
      else {
        grid[mydata.c2][mydata.c1] = 0;
      }
    }
    if (state==1){
      if (grid[mydata.c2][mydata.c1] == 1){
        for (int i = 0; i < 10; i++){
          multicom_send(-1, 1, mydata.c1, mydata.c2, true);
          delay(10);
        }
        Serial.println("Sent to node 1 true");
        grid[mydata.c2][mydata.c1] = 3;
      }
      else if (grid[mydata.c2][mydata.c1] == 0){
         for (int i = 0; i < 10; i++){
          multicom_send(-1, 1, mydata.c1, mydata.c2, false);
          delay(5);
        }
        grid[mydata.c2][mydata.c1] = 2;
      }
      else {
        Serial.println("wtf");
      }
      drawgrid();
    }
  }
}
