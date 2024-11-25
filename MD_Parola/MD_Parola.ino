#include <MD_Parola.h>
#include <MD_MAX72xx.h>
#include <DHT22.h>
#include <RTClib.h>
#include <Wire.h>
#include <SPI.h>
#include "Font7Seg.h"

#define HARDWARE_TYPE MD_MAX72XX::FC16_HW //tipe modul
#define MAX_DEVICES 4 //jumlah modul pada hardware
#define CS_PIN 10 //pin cs
#define SPEED_TIME 100 
#define PAUSE_TIME  0
#define DATA A0 //pin DHT22
#define MAX_MESG   20

MD_Parola md = MD_Parola(HARDWARE_TYPE, CS_PIN, MAX_DEVICES); //tipe modul, pin cs, max devices
DHT22 dht22(DATA);
RTC_DS3231 rtc;
int jam, menit, detik, sec;
int tanggal, bulan, tahun;
String hari;
char szMesg[MAX_MESG + 1] = "";
char szTime[9];

uint8_t degC[] = { 6, 3, 3, 56, 68, 68, 68 }; //simbol derajat celcius
uint8_t degF[] = { 6, 3, 3, 124, 20, 20, 4 }; //simbol derajat fahrenheit



void getTime(char *psz, bool f = true){
  DateTime now = rtc.now();
  jam     = now.hour();
  menit   = now.minute();
  detik   = now.second();
  sprintf(psz, "%02d%c%02d", jam, (f ? ':' : ' '), menit);
}

char* dow2str(uint8_t code, char *psz, uint8_t len) { 
  static const char* str[] = { 
    "Ahad", "Senin", "Selasa", 
    "Rabu", "Kamis", "Jum'at", 
    "Sabtu" 
  }; 
  strncpy(psz, str[code], len); 
  psz[len] = '\0'; 
  return psz; 
} 
void getDate(char *psz, uint8_t hari, uint8_t tanggal, uint8_t bulan, uint16_t tahun) { 
  char dayStr[10]; 
  dow2str(hari, dayStr, sizeof(dayStr) - 1); 
  sprintf(psz, "%s, %d - %d - %03d", dayStr, tanggal, bulan, tahun); 
}


void setup(){
  Wire.begin();
  rtc.begin();
  md.begin();
  md.setIntensity(15); //atur kecerahan led min 0, max 15
  md.displayClear();
  Serial.begin(115200);
  md.setZone(0,  MAX_DEVICES - 4, MAX_DEVICES - 1);
  md.setZone(1, MAX_DEVICES - 4, MAX_DEVICES - 1);

  md.displayZoneText(1, szTime, PA_CENTER, SPEED_TIME, PAUSE_TIME, PA_PRINT, PA_NO_EFFECT);
  md.displayZoneText(0, szMesg, PA_CENTER, SPEED_TIME, 0, PA_PRINT , PA_NO_EFFECT);

  md.addChar('$', degC);
  md.addChar('&', degF);
  if (rtc.lostPower())
  {
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }
  rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
}
void loop(){
  static uint8_t display = 0;
  static uint32_t lastTime = 0;
  static bool flasher = false;
  
  DateTime now = rtc.now();
  sec   = now.second();
  uint8_t hari = now.dayOfTheWeek();
  uint8_t tanggal = now.day();
  uint8_t bulan = now.month();
  uint16_t tahun = now.year();

  md.displayAnimate();
  
  float celcius = dht22.getTemperature();
  float fahrenheit = dht22.getTemperature(true);

  if(md.getZoneStatus(0)){
    switch (display){
      case 0:
        md.setPause(0, 2000);
        dtostrf(celcius,3,1,szMesg);
        md.setTextEffect(0, PA_SCROLL_LEFT, PA_SCROLL_UP);
        display++;
        strcat(szMesg, "$");
      break;
      case 1:
        md.setTextEffect(0, PA_SCROLL_UP, PA_SCROLL_DOWN);
        display++;
        dtostrf(fahrenheit, 3, 1, szMesg);
        strcat(szMesg, "&");
      break;
      case 2:
        md.setFont(0, sevenSegment);
        md.setTextEffect(0, PA_PRINT, PA_NO_EFFECT);
        md.setPause(0,0);
        if ((millis() - lastTime) >= 1000)
        {
          lastTime = millis();
          getTime(szMesg, flasher);
          flasher = !flasher;
        }
        if ((sec == 00) && (sec <= 30)) {
          display++;
          md.setTextEffect(0, PA_PRINT, PA_WIPE_CURSOR);
        }
      break;
      case 3:
        md.setFont(0, nullptr);
        md.setTextEffect(0, PA_SCROLL_LEFT, PA_SCROLL_LEFT);
        display++;
        getDate(szMesg, hari, tanggal, bulan, tahun);
      break;
      default: // Calendar
        md.setTextEffect(0, PA_SCROLL_LEFT, PA_SCROLL_LEFT);
        display = 0;
        getDate(szMesg, hari, tanggal, bulan, tahun);
      break;
    }
    md.displayReset(0);
  }
}