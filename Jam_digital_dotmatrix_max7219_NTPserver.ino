/*
  ESP-01 pinout from top:
  
  GND    GP2 GP0 RX/GP3
  TX/GP1 CH  RST VCC
  MAX7219
  ESP-1 from rear
  Re Br Or Ye
  Gr -- -- --
  USB to Serial programming
  ESP-1 from rear, FF to GND, RR to GND before upload
  Gr FF -- Bl
  Wh -- RR Vi
  GPIO 2 - DataIn
  GPIO 1 - LOAD/CS
  GPIO 0 - CLK
  ------------------------
  NodeMCU 1.0 pinout:
  D8 - DataIn
  D7 - LOAD/CS
  D6 - CLK
  
*/


#include "Arduino.h"
#include <ESP8266WiFi.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <SPI.h>
#include <MD_Parola.h>
WiFiClient client;

String date;

#define NUM_MAX 4

// for ESP-01 module
//#define DIN_PIN 2 // D4
//#define CS_PIN  3 // D9/RX
//#define CLK_PIN 0 // D3

// for NodeMCU 1.0
#define HARDWARE_TYPE MD_MAX72XX :: FC16_HW
#define DIN_PIN 14 // D5
#define CS_PIN 12  // D6
#define CLK_PIN 13  // D7
#include "max7219.h"
#include "fonts.h"
MD_Parola MyDisplay = MD_Parola (HARDWARE_TYPE, DIN_PIN, CLK_PIN, CS_PIN, NUM_MAX);

// =======================================================================
// CHANGE YOUR CONFIG HERE:
// =======================================================================
const char* ssid     = "REDMI NOTE 8";     // SSID of local network
const char* password = "SATUDUATIGA";   // Password on network

// =======================================================================
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "id.pool.ntp.org");
String weekDays[7]={"MINGGU", " SENIN", "SELASA", "  RABU", " KAMIS", "JUM'AT", " SABTU"};
String months[12]={"JANWITHYOU", "FEBWITHYOU", "MARWITHYOU", "APRWITHYOU", "MEIWITHYOU", "JUNWITHYOU", "JULWITHYOU", "AGTWITHYOU", "SEPWITHYOU", "OKTOBER", "NOVWITHYOU", "DESWITHYOU"};
char arrtgl[22];
char arrhar[7];

String dot = " ";
void setup() 
{
  Serial.begin(115200);
  initMAX7219();
  sendCmdAll(CMD_SHUTDOWN,1);
  sendCmdAll(CMD_INTENSITY,0);
  MyDisplay.begin();
  MyDisplay.setIntensity(0); //seting kecerahan 0 - 15
  MyDisplay.displayClear();
  Serial.print("Connecting WiFi ");
  WiFi.begin(ssid, password);
  printStringWithShift("MENGHUBUNGKAN KE WIFI                                 ",30);
  while (WiFi.status() != WL_CONNECTED) {
    delay(00);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected: "); Serial.println(WiFi.localIP());
  timeClient.begin();
  timeClient.setTimeOffset(25200);
}
// =======================================================================
#define MAX_DIGITS 16
byte dig[MAX_DIGITS]={0};
byte digold[MAX_DIGITS]={0};
byte digtrans[MAX_DIGITS]={0};
int updCnt = 0;
int dots = 0;
long dotTime = 0;
long clkTime = 0;
int dx=0;
int dy=0;
byte del=0;
int h,m,s;
// =======================================================================
void loop()
{
  timeClient.update();
  time_t epochTime = timeClient.getEpochTime();
  String weekDay = weekDays[timeClient.getDay()];
  weekDay.toCharArray(arrhar, 7);
  //Get a time structure
  struct tm *ptm = gmtime ((time_t *)&epochTime); 
  int hari = ptm->tm_mday;
  String sTgl="";
  if (hari<10)
  sTgl = "0" + String(hari); 
  else 
  sTgl = String(hari);
  int bulan = ptm->tm_mon+1;
  String bulanName = months[bulan-1];
  int tahun = ptm->tm_year+1900;
  String tanggal_lengkap = ", " + sTgl + " " + bulanName + " " + String(tahun);
  tanggal_lengkap.toCharArray(arrtgl, 22);
  
  /*Serial.print(h);
  Serial.print(":");
  Serial.print(m);
  Serial.print(":");
  Serial.println(s);
  Serial.println("");*/
  if(updCnt<=0) { // every 10 scrolls, ~450s=7.5m
    updCnt = 10;
    Serial.println("AMBIL DATA.."); 
    printStringWithShift("  AMBIL DATA                           ",35);
   
    getTime();
    Serial.println("Data loaded");
    clkTime = millis();
    Serial.println(updCnt);
  }
 
  if(millis()-clkTime > 45000 && !del && dots) { // clock for 15s, then scrolls for about 30s
   printStringWithShift("  ",30);
   printStringWithShift(arrhar,30);
   //printStringWithShift(", ",30);
   printStringWithShift(arrtgl ,30);
   printStringWithShift("              Notes :  Awali semua dengan Bismillah                     ",30);
   //date.c_str()                   ,40);
   delay(300);
    //updCnt--;
    clkTime = millis();
    
  }
  if(millis()-dotTime > 500) {
    dotTime = millis();
    dots = !dots;
  }
  updateTime();
  showAnimClock();
  Serial.println(weekDay + tanggal_lengkap);
  Serial.print(h);
  Serial.print(":");
  Serial.print(m);
  Serial.print(":");
  Serial.println(s);
  Serial.println("");
  
}

// =======================================================================

void showSimpleClock()
{
  dx=dy=0;
  clr();
  showDigit(h/10,  1, dig6x8);
  showDigit(h%10,  9, dig6x8);
  showDigit(m/10, 18, dig6x8);
  showDigit(m%10, 26, dig6x8);
 // showDigit(s/10, 38, dig6x8);
  //showDigit(s%10, 46, dig6x8);
  setCol(16,dots ? B00100100 : 0);
  setCol(33,dots ? B00100100 : 0);
  refreshAll();
}

// =======================================================================

void showAnimClock()
{
  byte digPos[6]={1,9,18,26,36,44};
  int digHt = 12;
  int num = 6; 
  int i;
  int hr;
  if(del==0) {
    del = digHt;
    for(i=0; i<num; i++) digold[i] = dig[i];

    if(h>12) {
      hr=h;
    }
    else {
      hr=h;
    }

    if(h>24) {
      hr=0;
    }

    //dig[0] = hr/10 ? hr/10 : 10;
    dig[0] = hr/10;
    dig[1] = hr%10;
    
    dig[2] = m/10;
    dig[3] = m%10;
    dig[4] = s/10;
    dig[5] = s%10;
    for(i=0; i<num; i++)  digtrans[i] = (dig[i]==digold[i]) ? 0 : digHt;
    } else
    del--;
  
  clr();
  for(i=0; i<num; i++) {
    if(digtrans[i]==0) {
      dy=0;
      showDigit(dig[i], digPos[i], dig6x8);
    } else {
      dy = digHt-digtrans[i];
      showDigit(digold[i], digPos[i], dig6x8);
      dy = -digtrans[i];
      showDigit(dig[i], digPos[i], dig6x8);
      digtrans[i]--;
    }
  }
  dy=0;
  setCol(16,dots ? B00100100 : 0);
  setCol(33,dots ? B00100100 : 0);
  refreshAll();
  delay(30);
}

// =======================================================================

void showDigit(char ch, int col, const uint8_t *data)
{
  if(dy<-8 | dy>8) return;
  int len = pgm_read_byte(data);
  int w = pgm_read_byte(data + 1 + ch * len);
  col += dx;
  for (int i = 0; i < w; i++)
    if(col+i>=0 && col+i<8*NUM_MAX) {
      byte v = pgm_read_byte(data + 1 + ch * len + 1 + i);
      if(!dy) scr[col + i] = v; else scr[col + i] |= dy>0 ? v>>dy : v<<-dy;
    }
}

// =======================================================================

void setCol(int col, byte v)
{
  if(dy<-8 | dy>8) return;
  col += dx;
  if(col>=0 && col<8*NUM_MAX)
    if(!dy) scr[col] = v; else scr[col] |= dy>0 ? v>>dy : v<<-dy;
}

// =======================================================================

int showChar(char ch, const uint8_t *data)
{
  int len = pgm_read_byte(data);
  int i,w = pgm_read_byte(data + 1 + ch * len);
  for (i = 0; i < w; i++)
    scr[NUM_MAX*8 + i] = pgm_read_byte(data + 1 + ch * len + 1 + i);
  scr[NUM_MAX*8 + i] = 0;
  return w;
}

// =======================================================================

void printCharWithShift(unsigned char c, int shiftDelay) {
  
  if (c < ' ' || c > '~'+25) return;
  c -= 32;
  int w = showChar(c, font);
  for (int i=0; i<w+1; i++) {
    delay(shiftDelay);
    scrollLeft();
    refreshAll();
  }
}

// =======================================================================

void printStringWithShift(const char* s, int shiftDelay){
  while (*s) {
    printCharWithShift(*s, shiftDelay);
    s++;
  }
}

// =======================================================================

float utcOffset = +7;
long localEpoc = 25200;
//long localEpoc = 0;
long localMillisAtUpdate = 0;

void getTime()
{
  WiFiClient client;
  if (!client.connect("www.google.com", 80)) {
    Serial.println("connection to google failed");
    return;
  }

  client.print(String("GET / HTTP/1.1\r\n") +
               String("Host: www.google.com\r\n") +
               String("Connection: close\r\n\r\n"));
  int repeatCounter = 0;
  while (!client.available() && repeatCounter < 10) {
    delay(500);
    //Serial.println(".");
    repeatCounter++;
    
  }

  String line;
  client.setNoDelay(false);
  while(client.connected() && client.available()) {
    line = client.readStringUntil('\n');
    line.toUpperCase();
    if (line.startsWith("DATE: ")) {
      date = "     "+line.substring(6, 22);
      h = line.substring(23, 25).toInt();
      m = line.substring(26, 28).toInt();
      s = line.substring(29, 31).toInt();
      localMillisAtUpdate = millis();
      localEpoc = (h * 60 * 60 + m * 60 + s);
      
    }
  }
  client.stop();
}

// =======================================================================

void updateTime()
{
  long curEpoch = localEpoc + ((millis() - localMillisAtUpdate) / 1000);
  long epoch = round(curEpoch + 3600 * utcOffset + 86400L);
  h = ((epoch  % 86400L) / 3600) % 24;
  m = (epoch % 3600) / 60;
  s = epoch % 60;
}

// =======================================================================
