#include <EEPROM.h>
#include "Wire.h"
#include "MCP4725.h"
#include "TM1637.h"

#define POT_PIN A0

#define CLK_PIN 2  //pins definitions for TM1637 and can be changed to other ports
#define DIO_PIN 3

#define D0_PIN 4
#define D1_PIN 5
#define D2_PIN 6
#define D3_PIN 7
#define Vt_PIN 8

#define BREAK_PIN 9
#define VITES_PIN 10
#define BUZZER_PIN 11
#define KONTAK_PIN 12

TM1637 tm1637(CLK_PIN, DIO_PIN);
MCP4725 MCP(0x62);

bool D0, D1, D2, D3;

void setup() {
  Wire.begin();
  MCP.begin();
  MCP.setMaxVoltage(5.1);  

  tm1637.init();
  tm1637.set(BRIGHT_TYPICAL);  //BRIGHT_TYPICAL = 2,BRIGHT_DARKEST = 0,BRIGHTEST = 7;
  pinMode(D0_PIN, INPUT);
  pinMode(D1_PIN, INPUT);
  pinMode(D2_PIN, INPUT);
  pinMode(D3_PIN, INPUT);
  pinMode(VT_PIN, INPUT);
  EEPROM.put(0, 1500);
  int total_used;
  EEPROM.get(0, total_used);
}

void loop() {

  if (digitalRead(VT_PIN == HIGH)) {
    D0 = digitalRead(D0_PIN);
    D1 = digitalRead(D1_PIN);
    D2 = digitalRead(D2_PIN);
    D3 = digitalRead(D3_PIN);
  }
  
  tm1637.display(0, 4);  //0. haneye 4 yazar
  tm1637.display(1, 6);
  tm1637.display(2, 6);
  tm1637.display(3, 7);

  MCP.setVoltage(2.5);
  delay(500);
}
