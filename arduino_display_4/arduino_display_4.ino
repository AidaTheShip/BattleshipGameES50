#include <EasyTransfer.h>

// you can use softserial like this
#include <SoftwareSerial.h>

#define NODEID 4
#define NUMNODES 4; 

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

void setup() {
  Serial.begin(9600);
  ET.begin(details(mydata), &Serial);
  Serial.println("starting Arduino 4 (display)");
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
  Serial.println(mydata.from);
  Serial.println(mydata.to);
  Serial.println(mydata.coor);
  Serial.println(mydata.rise);
  Serial.println(mydata.first);
}
