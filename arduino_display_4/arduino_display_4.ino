#include <EasyTransfer.h>

// you can use softserial like this
#include <SoftwareSerial.h>

#include <RGBmatrixPanel.h>

#define NODEID 4
#define NUMNODES 4; 

#define CLK 8
#define OE 9
#define LAT 10
#define A A0
#define B A1
#define C A2

RGBmatrixPanel matrix(A, B, C, CLK, LAT, OE, false);

int state=0;
int notepadgrid[8][8];

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

uint8_t r = 0, g = 0, b = 0;
void timer() {
    for(uint8_t x = 0; x < 32; x++){
      for(uint8_t y = 0; y < 16; y++){
        r = 7 ;
        matrix.drawPixel(x, y, matrix.Color333(r, g, b));
      }
      for (int j = 0; j < 100; j++) {
        multicom_update();
        delay(10);
      }
    }
}


void setup() {
  Serial.begin(9600);
  matrix.begin();
  ET.begin(details(mydata), &Serial);
  multicom_send(1,-1,false,false);
  for (int i = 0; i < 8; i++){
    for(int j = 0; j < 8; j++){ 
      notepadgrid[i][j] = 0 ;
    }
  }
  Serial.println("starting Arduino 4 (display)");
    for(uint8_t x = 0; x < 32; x++){
      for(uint8_t y = 0; y < 16; y++){
        matrix.drawPixel(x, y, matrix.Color333(7, 7, 7));
      }
    }
    timer ();
   for(uint8_t x = 0; x < 32; x++){
      for(uint8_t y = 0; y < 16; y++){
        if (x >= 8 || y >= 8) {
          matrix.drawPixel(x, y, matrix.Color333(4, 4, 4));
        }
        else if (notepadgrid[x][y] == 1){
          matrix.drawPixel(x, y, matrix.Color333(7, 7, 7));
        }
        else {
          matrix.drawPixel(x, y, matrix.Color333(0, 0, 0));
        }
      }
    }
    for(uint8_t x = 0; x < 16; x++){
      for(uint8_t y = 0; y < 16; y++){
        matrix.drawPixel(x, y, matrix.Color333(0, 0, 0));
      }
    }
    for(uint8_t x = 16; x < 32; x++){
      for(uint8_t y = 0; y < 16; y++){
        if (notepadgrid[(x-16)/2][y/2] == 1){
          matrix.drawPixel(x, y, matrix.Color333(7, 7, 7));
        }
        else{
          matrix.drawPixel(x, y, matrix.Color333(0, 0, 7));        
        }
      }
    }
    delay(1000);
    multicom_send(1, -2, false, false);
    delay(20);
    multicom_send(2, -3, false, false);
}

void loop() {
  multicom_update();
}

void multicom_update()
{
 while(ET.receiveData())
  {
     if (mydata.to == NODEID) {
        Serial.println("Node 4 received info");
        multicom_receive();
     }
  }
}

void multicom_send(char to, int coor, bool rise, bool first)
{
  mydata.from = NODEID;
  mydata.to = to;
  mydata.coor = coor ; 
  mydata.rise = rise ;
  mydata.first = first;
  ET.sendData(); 
}

void multicom_receive()
{
  if (state==0){
    int c2 = mydata.coor / 8;
    int c1 = mydata.coor % 8;
    if (mydata.rise){
      notepadgrid[c1][c2] = 1;
    }
    else {
      notepadgrid[c1][c2] = 0;
    }
  }
}
