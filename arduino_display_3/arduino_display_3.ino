#include <EasyTransfer.h>

// you can use softserial like this
#include <SoftwareSerial.h>

#define NODEID 3
#define NUMNODES 4; 

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
int grid[8][8]; // currently discovered grid of the opponent, 0 -> no try, 1 -> miss, 2 -> ship

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
  for(uint8_t x = 16; x < 32; x++){
    for(uint8_t y = 0; y < 16; y++){
      if (grid[(x-16)/2][y/2] == 1){
        matrix.drawPixel(x, y, matrix.Color333(7, 7, 7));
      }
      else{
        matrix.drawPixel(x, y, matrix.Color333(0, 0, 7));        
      }
    }
  }
  state=1;
  delay(1000);
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
    if (state==0){
    int c2 = mydata.coor / 8;
    int c1 = mydata.coor % 8;
    if (mydata.rise){
      grid[c1][c2] = 1;
    }
    else {
      grid[c1][c2] = 0;
    }
  }
}
