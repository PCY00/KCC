//https://www.airspayce.com/mikem/arduino/RadioHead/classRHGenericDriver.html#aa1c0a94ba6414b5bbab0133c4d74e646
#include <RH_RF95.h>
#include <SPI.h>
#include <PMS.h>
#include <Arduino.h>   // required before wiring_private.h
#include "wiring_private.h" // pinPeripheral() function

#define RFM95_CS 8
#define RFM95_RST 4
#define RFM95_INT 3

#define RF95_FREQ 920.9

PMS pms(Serial1);
PMS::DATA data;

RH_RF95 rf95(RFM95_CS, RFM95_INT);

uint16_t saveDust[3] = {0,};

uint8_t startCode[3] = {0x01,0x02,0x03};

boolean Tx1_status = true;
boolean Tx2_status = false;
boolean Tx3_status = false;

boolean runEvery(unsigned long interval);
void dataProcessing(uint16_t pm2_5data);
void receiveDust(uint8_t* msg, uint8_t num);

void setup() {
  pinMode(RFM95_RST, OUTPUT);
  digitalWrite(RFM95_RST, HIGH);
  
  Serial.begin(115200);
  Serial1.begin(9600);
  while (!Serial) {
    delay(1);
  }
  delay(100);

  Serial.println("Feather LoRa RX Test!");
  
  pinPeripheral(1, PIO_SERCOM_ALT);
  pinPeripheral(0, PIO_SERCOM_ALT);

  pms.passiveMode();
  
  digitalWrite(RFM95_RST, LOW);
  delay(10);
  digitalWrite(RFM95_RST, HIGH);
  delay(10);
  
  while (!rf95.init()) {
    Serial.println("LoRa radio init failed");
    while (1);
  }
  
  if (!rf95.setFrequency(RF95_FREQ)) {
    Serial.println("setFrequency failed");
    while (1);
  }
  Serial.print("Set Freq to: "); Serial.println(RF95_FREQ);
  rf95.setTxPower(23, false);
}

void loop() {
  if(runEvery(5000)){
    uint16_t pm_2_5 = 0;
    
    pms.requestRead();
    if(pms.readUntil(data)){
      pm_2_5 = data.PM_AE_UG_2_5;
    }
    
    if(Tx1_status == true){
      
      rf95.send(&startCode[0], 1);
      rf95.waitPacketSent();
  
      if(rf95.available()){
        uint8_t buf[RH_RF95_MAX_MESSAGE_LEN];
        uint8_t len = sizeof(buf);
  
        if(rf95.recv(buf, &len)){
          receiveDust(buf, 1);
          delay(10);
        }
      }
      Serial.print("1 : ");
      Serial.println(saveDust[0]);
      Tx1_status = false;
      Tx2_status = true;
    }
    delay(100);
    
    if(Tx2_status == true){
      rf95.send(&startCode[1], 1);
      rf95.waitPacketSent();
  
      if(rf95.available()){
        uint8_t buf[RH_RF95_MAX_MESSAGE_LEN];
        uint8_t len = sizeof(buf);
  
        if(rf95.recv(buf, &len)){
          receiveDust(buf, 2);
          delay(10);
        }
      }
      Serial.print("2 : ");
      Serial.println(saveDust[1]);
      Tx2_status = false;
      Tx3_status = true;
    }
    delay(100);
  
    if(Tx3_status == true){
      rf95.send(&startCode[2], 1);
      rf95.waitPacketSent();
  
      if(rf95.available()){
        uint8_t buf[RH_RF95_MAX_MESSAGE_LEN];
        uint8_t len = sizeof(buf);
  
        if(rf95.recv(buf, &len)){
          receiveDust(buf, 3);
          delay(10);
        }
      }
      Serial.print("3 : ");
      Serial.println(saveDust[2]);
  
      Tx3_status = false;
      Tx1_status = true;
    }
    String str_data = "";
    for(int i = 0; i < 3; i++){
      str_data += ' ';
      str_data += saveDust[i];
    }

    str_data += ' ';
    str_data += pm_2_5;

    Serial.println(str_data);
    delay(100);
  }
  
}

void receiveDust(uint8_t* msg, uint8_t num){
  //Serial.print("Dust_");
  //Serial.print(num);
  //Serial.print(": ");
  
  int16_t pm2_5;
  pm2_5 = msg[1] << 8 | msg[0];
  //save value
  saveDust[num - 1] = pm2_5;
  //Serial.print(pm2_5);
  //Serial.println();
}

boolean runEvery(unsigned long interval){
  static unsigned long previousMillis = 0;
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval)
  {
    previousMillis = currentMillis;
    return true;
  }
  return false;
}
