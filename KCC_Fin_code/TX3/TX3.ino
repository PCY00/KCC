#include <RH_RF95.h>
#include <SPI.h>
#include <PMS.h>
#include <Arduino.h>   // required before wiring_private.h
#include "wiring_private.h" // pinPeripheral() function

#define RFM95_CS 8
#define RFM95_RST 4
#define RFM95_INT 3

#define RF95_FREQ_1 920.9

RH_RF95 rf95(RFM95_CS, RFM95_INT);
PMS pms(Serial1);
PMS::DATA data;

uint8_t msg[2] = {0,};
boolean post = false;

void dataProcessing(int16_t pm2_5data);

void setup() {
  pinMode(RFM95_RST, OUTPUT);
  digitalWrite(RFM95_RST, HIGH);
  
  Serial.begin(115200);
  Serial1.begin(9600);
  
  pinPeripheral(1, PIO_SERCOM_ALT);
  pinPeripheral(0, PIO_SERCOM_ALT);

  pms.passiveMode();
//pms.activeMode();  
  
  while(!Serial){
    delay(1);
  }
  delay(100);

  Serial.println("Tx test");
  
  digitalWrite(RFM95_RST, LOW);
  delay(10);
  digitalWrite(RFM95_RST, HIGH);
  delay(10);
  
  // 첫 번째 드라이버 설정
  while(!rf95.init()){
    Serial.println("LoRa radio init failed");
    while(1);
  }
  Serial.println("LoRa radio init success");
  
  if (!rf95.setFrequency(RF95_FREQ_1)) {
    Serial.println("setFrequency failed");  
    while(1);
  }
  Serial.print("Using F"); Serial.println(RF95_FREQ_1);
  
  rf95.setTxPower(23, false);
}

void loop(){
  if(rf95.available()){
    uint8_t buf[RH_RF95_MAX_MESSAGE_LEN];
    uint8_t len = sizeof(buf);
    if(rf95.recv(buf, &len)){
      if(buf[0] == 0x03){
        post = true;
      }
    }
  }

  if(post = true){
    //if post
    pms.requestRead();
    if(pms.readUntil(data)){
      int16_t pm2_5 = data.PM_AE_UG_2_5;
      dataProcessing(pm2_5);

      rf95.send(msg, 2);
      rf95.waitPacketSent();
    }
  }
  post = false;
}

//data processing -> 16bit -> two 8bit (up byte, down byte)
void dataProcessing(int16_t pm2_5data){
  msg[0] = pm2_5data & 0xFF;
  msg[1] = (0xff & (pm2_5data >> 8));
}
