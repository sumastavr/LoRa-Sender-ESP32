#include <Arduino.h>

#include <Wire.h> 
#include "SSD1306Wire.h" 
#include <SPI.h>
#include <LoRa.h>

#define LORA_SENDER 0

#define OLED_CLASS_OBJ  SSD1306Wire
#define OLED_ADDRESS    0x3C
#define OLED_SDA    21
#define OLED_SCL    22
#define OLED_RST    -1

#define CONFIG_MOSI 27
#define CONFIG_MISO 19
#define CONFIG_CLK  5
#define CONFIG_NSS  18
#define CONFIG_RST  23
#define CONFIG_DIO0 26

#define SDCARD_MOSI 15
#define SDCARD_MISO 2
#define SDCARD_SCLK 14
#define SDCARD_CS   13

#define LORA_PERIOD 868 

#if LORA_PERIOD == 433
#define BAND 433E6
#elif LORA_PERIOD == 868
#define BAND 868E6
#elif LORA_PERIOD == 915
#define BAND 915E6
#else
#error "Please select the correct lora frequency"
#endif

OLED_CLASS_OBJ display(OLED_ADDRESS, OLED_SDA, OLED_SCL);

void setup() {

  Serial.begin(9600);
  //Serial1.begin(57600,3,1);
  // put your setup code here, to run once:
    if (OLED_RST > 0) {
        pinMode(OLED_RST, OUTPUT);
        digitalWrite(OLED_RST, HIGH);
        delay(100);
        digitalWrite(OLED_RST, LOW);
        delay(100);
        digitalWrite(OLED_RST, HIGH);
    }

    display.init();
    display.flipScreenVertically();
    display.clear();
    display.setFont(ArialMT_Plain_16);
    display.setTextAlignment(TEXT_ALIGN_CENTER);
    display.drawString(display.getWidth() / 2, display.getHeight() / 2, LORA_SENDER ? "LoRa Sender" : "LoRa Receiver");
    display.display();
    delay(2000);

    SPI.begin(CONFIG_CLK, CONFIG_MISO, CONFIG_MOSI, CONFIG_NSS);
    LoRa.setPins(CONFIG_NSS, CONFIG_RST, CONFIG_DIO0);
    if (!LoRa.begin(BAND)) {
        Serial.println("Starting LoRa failed!");
        while (1);
    }
    if (!LORA_SENDER) {
        display.clear();
        display.drawString(display.getWidth() / 2, display.getHeight() / 2, "LoraRecv Ready");
        display.display();
    }


}

int counter = 0;
long counterRequest=millis();
const long timeoutRequest=10000;
bool sendRequest=false;

void loop() {


#if LORA_SENDER

  if(millis()-counterRequest>timeoutRequest && sendRequest){
    Serial.write(97);
    display.clear();
    display.setTextAlignment(TEXT_ALIGN_LEFT);
    display.setFont(ArialMT_Plain_10);
    display.display();
    counterRequest=millis();
  }
  
  String in="";

  if(Serial.available()>0){
    delay(300);
    while(Serial.available()>0){
        byte i = Serial.read();
        //Serial.write(i);
        in+=(char)i;
    }
    display.clear();
    display.setTextAlignment(TEXT_ALIGN_LEFT);
    display.setFont(ArialMT_Plain_10);
    display.drawStringMaxWidth(0, 0,128, in);
    display.display();
    Serial.print("Received: ");
    Serial.println(millis());

    LoRa.beginPacket();
    LoRa.print("RCV: ");
    //LoRa.print(in.length());
    LoRa.print(in);
    LoRa.endPacket();

  }

#else
    if (LoRa.parsePacket()) {
        String recv = "";
        
        while (LoRa.available()) {
            recv += (char)LoRa.read();
        }

        counter++;
        display.clear();
        display.setTextAlignment(TEXT_ALIGN_LEFT);
        display.setFont(ArialMT_Plain_10);
        String info = "[" + String(counter) + "]" + "RSSI " + String(LoRa.packetRssi());
        display.drawString(0, 0, info);
        display.drawStringMaxWidth(0, 0, 128, recv);

        /*
        display.drawString(display.getWidth() / 2, display.getHeight() / 2, recv);
        String info = "[" + String(counter) + "]" + "RSSI " + String(LoRa.packetRssi());
        display.drawString(display.getWidth() / 2, display.getHeight() / 2 - 16, info);
        */

        display.display();
        Serial.print(info);
        Serial.print("  ");
        Serial.println(recv);
    }

#endif

}