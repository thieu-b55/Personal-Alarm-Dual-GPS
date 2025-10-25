/*
* MIT License
*
* Copyright (c) 2025 thieu-b55
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in all
* copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*/


#include <Arduino.h>
#include <TinyGPS++.h>
#include <FlashStorage_STM32.h>
#include "STM32TimerInterrupt.h"

HardwareSerial M10Q_SERIAL(PB11, PB10);                                                               //RX, TX  Serial3
HardwareSerial A7670_SERIAL(PA3, PA2);                                                                //RX, TX  Serial2
HardwareSerial SERIAL_1(PA10, PA9);                                                                   //RX, TX  Serial1

TinyGPSPlus gps;

HardwareTimer *MyTim = new HardwareTimer(TIM1);

#define CODE                  66
#define BERICHT_LOCATIE       6

#define ALARM_INPUT           PA0
#define PRGM_EN               PA1
#define TEST                  PA4                                                                                  
#define LEDS_1                PA5
#define LEDS_2                PA6
#define BUZZER                PA7

void wis_eeprom();
void stuur_leds();
void instellen();
void wacht_op_antwoord();
void lees_antwoord();
void M10Q_opvragen();
void A7670_opvragen();
void sms_ontvangen();
void lees_tlf_nummer();
void lees_txt();
void stuur_sms();
void stuur_test_sms();

bool error_bool;
bool alarm_bool = false;
bool sms_gestuurd_bool = false;
bool timer_gestart_bool = false;
bool leds_gestuurd_bool = false;
bool prgm_en_vorig_bool = false;
bool nummer_gewist_bool = false;
bool list_bool = false;

byte controle_byte;

const char CMT[] = "+CMT:";  

char data_char[275];
char nummer_char[4];
char tlf_nummer_char[100];
char buffer_char[100];
char sms_nummer_char[100];
char text_char[100];

float lat_float;
float long_float;

int eerste_int;
int tweede_int;
int nummer_int;
int tlf_nummer_lengte_int;
int eeprom_start_int;

String ontvangen_string =       "                                                                                                   ";
String nummer_string =          "                                                                                                   ";
String CMT_string=              "        ";
String tlf_nummer_string =      "                                                                                                   ";
String text_string =            "                                                                                                   ";
String error_string =           "                             ";
String M10Q_lat_string =        "                             ";
String M10Q_long_string =       "                             ";
String A7670_lat_string =       "                             ";
String A7670_long_string =      "                             ";



void setup() {
  delay(1000);
  SERIAL_1.begin(115200);                                                                                   
  M10Q_SERIAL.begin(9600);
  A7670_SERIAL.begin(9600);
  pinMode(ALARM_INPUT, INPUT_PULLUP);
  pinMode(PRGM_EN, INPUT_PULLUP);
  pinMode(TEST, INPUT_PULLUP);
  pinMode(LEDS_1, OUTPUT);
  pinMode(LEDS_2, OUTPUT);
  pinMode(BUZZER, OUTPUT);
  digitalWrite(LEDS_1, false);
  digitalWrite(LEDS_2, false);
  digitalWrite(BUZZER, false);
  controle_byte = EEPROM.read(0);
  if(controle_byte != CODE){
    wis_eeprom();
  }
  MyTim->setOverflow(25, HERTZ_FORMAT); 
  MyTim->attachInterrupt(stuur_leds);
  delay(20000);                                                
  instellen();
}

void loop(){
  // ALARM
  if((!digitalRead(ALARM_INPUT)) && digitalRead(TEST)){
    alarm_bool = true;
  }

  // Stuur alarmen plus sms
  if(alarm_bool && (!sms_gestuurd_bool)){
    digitalWrite(LEDS_1, true);
    digitalWrite(LEDS_2, true);

/*
*
* Remove the // in line 145 to activate the buzzer on alarm
* 
*/

    //digitalWrite(BUZZER, true);
    stuur_sms();
  }
  
  if(sms_gestuurd_bool && (!timer_gestart_bool)){
    MyTim->resume();
    timer_gestart_bool = true;
  }

  // TEST ALARM
  if((!digitalRead(ALARM_INPUT)) && (!digitalRead(TEST)) && (!alarm_bool)){
    digitalWrite(LEDS_1, true);
    digitalWrite(LEDS_2, true);
    digitalWrite(BUZZER, true);
    delay(1000);
    digitalWrite(LEDS_1, false);
    digitalWrite(LEDS_2, false);
    digitalWrite(BUZZER, false);
    stuur_test_sms();
    while(!digitalRead(TEST)){
      delay(1);
    }
  }

  // Sms ontvangen ?
  if((!digitalRead(PRGM_EN)) && (!alarm_bool)){
    if(A7670_SERIAL.available() > 0){
      error_bool = false;
      error_string = "";
      nummer_gewist_bool = false;
      list_bool = false;
      memset(data_char, 0, sizeof data_char);
      A7670_SERIAL.readBytesUntil(char(0), data_char, 250);
      int x = 0;
      ontvangen_string = "";
      while(data_char[x] != char(0) && (x < 250)){
        ontvangen_string += data_char[x];
        x++;
      }
      if((strstr(data_char, CMT) != NULL)){
        sms_ontvangen();
      }
    }
  }
}


/*
*   FUNCTIONS
*/

void wis_eeprom(){
  for (unsigned int x = 0 ; x < EEPROM.length() ; x++) {
        EEPROM.write(x, 0);
  }
  EEPROM.write(0, CODE);
  EEPROM.commit();
}

void stuur_leds(){
  digitalWrite(LEDS_1, (!digitalRead(LEDS_2)));
  digitalWrite(LEDS_2, (!digitalRead(LEDS_2)));
}

void instellen(){
  //OK?
  A7670_SERIAL.println("AT");
  wacht_op_antwoord();
  lees_antwoord();

  //GSM character set
  A7670_SERIAL.println("AT+CSCS=\"GSM\""); 
  wacht_op_antwoord();
  lees_antwoord();
  A7670_SERIAL.println("AT+CSCS?"); 
  wacht_op_antwoord();
  lees_antwoord();
  
  //SMS Text mode
  A7670_SERIAL.println("AT+CMGF=1");
  wacht_op_antwoord();
  lees_antwoord();
  A7670_SERIAL.println("AT+CMGF?");
  wacht_op_antwoord();
  lees_antwoord();
  
  // Send SMS to Serial not to memory
  A7670_SERIAL.println("AT+CNMI=2,2,0,0,0"); 
  wacht_op_antwoord();
  lees_antwoord();
  A7670_SERIAL.println("AT+CNMI?");
  wacht_op_antwoord();
  lees_antwoord();

  //Turn on GNSS 
  A7670_SERIAL.println("AT+CGNSSPWR=1,0");
  delay(15000);
  lees_antwoord();
  A7670_SERIAL.println("AT+CGNSSPWR?");
  wacht_op_antwoord();
  lees_antwoord();
  
  //GPS + GLONASS + GALILEO + SBAS + QZSS
  A7670_SERIAL.println("AT+CGNSSMODE=3");
  wacht_op_antwoord();
  lees_antwoord();
}

void wacht_op_antwoord(){
  while(!(A7670_SERIAL.available())){
    delay(1);
  }
}

void lees_antwoord(){
  while(A7670_SERIAL.available()){
    //SERIAL_1.write(char(A7670_SERIAL.read()));
    A7670_SERIAL.read();
    delay(10);
  }
  delay(10);
}

void M10Q_opvragen(){
  unsigned long nu_long = millis();
  do 
    {
      while (M10Q_SERIAL.available()){
        gps.encode(M10Q_SERIAL.read());
      }
    }
  while ((millis() - nu_long) < 1000);
  M10Q_lat_string = "";
  M10Q_long_string = "";
  M10Q_lat_string = String(gps.location.lat(), 7);
  M10Q_long_string= String(gps.location.lng(), 7);
}

void A7670_opvragen(){
  String GNSS_string = "                                                                                                   ";
  int dummy_int;
  int lat_1_int;
  int lat_2_int;
  int long_1_int;
  int long_2_int;
  GNSS_string = "";
  A7670_SERIAL.println("AT+CGNSSINFO"); 
  wacht_op_antwoord();
  while(A7670_SERIAL.available()){
    GNSS_string += char(A7670_SERIAL.read());
    delay(10);
  }
  dummy_int = GNSS_string.indexOf(",");
  dummy_int = GNSS_string.indexOf(",", dummy_int + 1);
  dummy_int = GNSS_string.indexOf(",", dummy_int + 1);
  dummy_int = GNSS_string.indexOf(",", dummy_int + 1);
  lat_1_int = GNSS_string.indexOf(",", dummy_int + 1);
  lat_2_int = GNSS_string.indexOf(",", lat_1_int + 1);
  long_1_int = GNSS_string.indexOf(",", lat_2_int + 1);
  long_2_int = GNSS_string.indexOf(",", long_1_int + 1);
  A7670_lat_string = "";
  A7670_long_string = "";
  A7670_lat_string = GNSS_string.substring(lat_1_int + 1, lat_2_int);
  A7670_long_string = GNSS_string.substring(long_1_int + 1, long_2_int);
}

void sms_ontvangen(){
  eerste_int = ontvangen_string.indexOf('"');                                                             // find sender between "     "                                       
  tweede_int = ontvangen_string.indexOf('"', eerste_int + 1);
  tlf_nummer_string = ontvangen_string.substring(eerste_int + 1, tweede_int);
  memset(sms_nummer_char, 0, sizeof sms_nummer_char);
  tlf_nummer_string.toCharArray(sms_nummer_char, tlf_nummer_string.length() + 1);                         // sms_number_char  =  SMS sender number
  eerste_int = ontvangen_string.indexOf("#");                                                             // message must start and end with #                                       
  if(eerste_int == -1){
    error_bool = true;
    error_string = "# MISSING";
  }
  if(!error_bool){
    tweede_int = ontvangen_string.indexOf("#", eerste_int + 1);
    if(tweede_int == -1){
      error_bool = true;
      error_string = "# MISSING";
    }
  }
  if(!error_bool){
    if((strstr(data_char, "LIST") != NULL)){
      list_bool = true;
    }
    if(!list_bool){
      nummer_string = "";
      nummer_string = ontvangen_string.substring(eerste_int + 1, eerste_int + 2);     
      nummer_int = nummer_string.toInt();                                             
      if((nummer_int < 1) || (nummer_int > 7)){
        error_bool = true;
        error_string = "POSITION < 1   OR   > 7";
      }
    }
  }
  if((!error_bool) && (!list_bool)){
    eeprom_start_int = nummer_int * 50; 
    if((strstr(data_char, "DEL") != NULL)){
      for(int x = eeprom_start_int; x < eeprom_start_int + 50; x++){
        EEPROM.write(x, 0);
      }
      EEPROM.commit();
      nummer_gewist_bool = true;
    }
    else{
      if(nummer_int != 7){
        tlf_nummer_string = "";
        tlf_nummer_string = ontvangen_string.substring(eerste_int + 3, tweede_int);
        if(tlf_nummer_string.length() > 48){
          error_bool = true;
          error_string = " PHONE NUMBER MORE THAN 50 CHARACTERS";
        }
        if(!error_bool){
          if(tlf_nummer_string.substring(0, 1) == "+"){     
            memset(tlf_nummer_char, 0, sizeof tlf_nummer_char);
            tlf_nummer_string.toCharArray(tlf_nummer_char, tlf_nummer_string.length() + 1);               // number to safe  >>  tlf_nummer_char
            for(int x = eeprom_start_int; x < eeprom_start_int + 50; x++){                                // First delete the previously saved number
              EEPROM.write(x, 0);
            }
            int y = 0;
            for(int x = eeprom_start_int; x < eeprom_start_int + 50; x++){                                // Save the new number
              if(tlf_nummer_char[y] != 0){
                EEPROM.write(x, tlf_nummer_char[y]);
              }
              y++;
            }
            EEPROM.commit();
            lees_tlf_nummer();                                                                            // Read back the number you just saved
          }
          else{
            error_bool = true;
            error_string = "+ MISSING AT BEGIN TELEPHONE NUMBER";
          }
        }
      }
      else{
        text_string = "";
        text_string = ontvangen_string.substring(eerste_int + 3, tweede_int);
        if(text_string.length() > 48){
          error_bool = true;
          error_string = " TEXT MORE THAN 50 CHARACTERS";
        }
        if(!error_bool){
          memset(text_char, 0, sizeof text_char);
          text_string.toCharArray(text_char, text_string.length() + 1);                                   // text to safe  >>  text_char
          for(int x = eeprom_start_int; x < eeprom_start_int + 50; x++){                                  // First delete the previously saved text
            EEPROM.write(x, 0);
          }
          int y = 0;
          for(int x = eeprom_start_int; x < eeprom_start_int + 50; x++){                                  // Save new text                               
            if(text_char[y] != 0){
              EEPROM.write(x, text_char[y]);
            }
            y++;
          }
          EEPROM.commit();
          lees_txt();                                                                                     // Read back text you just saved  
        }
      }
    }
  }
  if(list_bool){
    for(nummer_int = 1; nummer_int < 8; nummer_int++){
      eeprom_start_int = nummer_int * 50;
      lees_tlf_nummer();
      memset(buffer_char, 0, sizeof buffer_char);
      sprintf(buffer_char, "AT+CMGS=\"%s\"", sms_nummer_char);
      A7670_SERIAL.println(buffer_char);
      wacht_op_antwoord();
      memset(buffer_char, 0, sizeof buffer_char);
      if(nummer_int != 7){
        sprintf(buffer_char, "pos : %d    tel : %s", nummer_int, tlf_nummer_char);
      }
      else{
        sprintf(buffer_char, "pos : %d    txt : %s", nummer_int, tlf_nummer_char);
      }
      A7670_SERIAL.print(buffer_char);
      wacht_op_antwoord();
      lees_antwoord();  
      A7670_SERIAL.write(26);
      wacht_op_antwoord();
      lees_antwoord();
    }
  }
  else{
    memset(buffer_char, 0, sizeof buffer_char);                                                           // forward the result of the position just programmed to the sender of the SMS
    sprintf(buffer_char, "AT+CMGS=\"%s\"", sms_nummer_char);                        
    A7670_SERIAL.println(buffer_char);
    wacht_op_antwoord();
    if(!error_bool){
      if(!nummer_gewist_bool){
        if(nummer_int != 7){
          memset(buffer_char, 0, sizeof buffer_char);
          sprintf(buffer_char, "pos : %d    tel : %s", nummer_int, tlf_nummer_char);
          A7670_SERIAL.print(buffer_char); 
        }
        else{
          memset(buffer_char, 0, sizeof buffer_char);
          sprintf(buffer_char, "pos : %d    txt : %s", nummer_int, text_char);
          A7670_SERIAL.print(buffer_char); 
        }
      }
      if(nummer_gewist_bool){
        memset(buffer_char, 0, sizeof buffer_char);
        sprintf(buffer_char, "pos : %d  %s", nummer_int, "deleted");
        A7670_SERIAL.print(buffer_char); 
      }
    }
    if(error_bool){
      A7670_SERIAL.print(error_string);   
    }
    wacht_op_antwoord();
    A7670_SERIAL.write(26);
    delay(1000);
    wacht_op_antwoord();
    lees_antwoord();  
  }
}

void lees_tlf_nummer(){
  memset(tlf_nummer_char, 0, sizeof tlf_nummer_char);
  tlf_nummer_lengte_int = 0;
  for(int x = eeprom_start_int; x < eeprom_start_int + 50; x++){
    if(EEPROM.read(x) != 0){
      tlf_nummer_char[tlf_nummer_lengte_int] = EEPROM.read(x);
      tlf_nummer_lengte_int++;
    }
    else{
      break;
    }
  }
}

void lees_txt(){
  memset(text_char, 0, sizeof text_char);
  int y = 0;
  for(int x = 350; x < 400; x++){
    if(EEPROM.read(x) != 0){
      text_char[y] = EEPROM.read(x);
    }
    y++;
  }
}

void stuur_sms(){
  M10Q_opvragen();
  A7670_opvragen();
  lees_txt();
  for(nummer_int = 1; nummer_int < 6; nummer_int++){ 
    eeprom_start_int = nummer_int * 50;
    lees_tlf_nummer();
    if(tlf_nummer_lengte_int > 3){
      memset(buffer_char, 0, sizeof buffer_char);
      sprintf(buffer_char, "AT+CMGS=\"%s\"", tlf_nummer_char);
      A7670_SERIAL.println(buffer_char);
      wacht_op_antwoord();
      lees_antwoord();  
      A7670_SERIAL.println(text_char);
      wacht_op_antwoord();
      lees_antwoord();  
      A7670_SERIAL.print("www.google.com/maps/place/");
      A7670_SERIAL.print(M10Q_lat_string);
      A7670_SERIAL.print(",");
      A7670_SERIAL.print(M10Q_long_string); 
      A7670_SERIAL.print("\n\n"); 
      A7670_SERIAL.print("www.google.com/maps/place/");
      A7670_SERIAL.print(A7670_lat_string);
      A7670_SERIAL.print(",");
      A7670_SERIAL.println(A7670_long_string);      
      wacht_op_antwoord();
      lees_antwoord();  
      A7670_SERIAL.write(26);
      wacht_op_antwoord();
      lees_antwoord();
      delay(5000);
    }
  }
  sms_gestuurd_bool = true;
}

void stuur_test_sms(){
  M10Q_opvragen();
  A7670_opvragen();
  lees_txt();
  nummer_int = BERICHT_LOCATIE;
  eeprom_start_int = nummer_int * 50;
  lees_tlf_nummer();
  memset(buffer_char, 0, sizeof buffer_char);
  sprintf(buffer_char, "AT+CMGS=\"%s\"", tlf_nummer_char);
  A7670_SERIAL.println(buffer_char);
  wacht_op_antwoord();
  lees_antwoord();       
  A7670_SERIAL.println(text_char);
  wacht_op_antwoord();
  lees_antwoord();  
  A7670_SERIAL.print("www.google.com/maps/place/");
  A7670_SERIAL.print(M10Q_lat_string);
  A7670_SERIAL.print(",");
  A7670_SERIAL.print(M10Q_long_string); 
  A7670_SERIAL.print("\n\n"); 
  A7670_SERIAL.print("www.google.com/maps/place/");
  A7670_SERIAL.print(A7670_lat_string);
  A7670_SERIAL.print(",");
  A7670_SERIAL.println(A7670_long_string);     
  wacht_op_antwoord();
  lees_antwoord();        
  A7670_SERIAL.write(26);
  wacht_op_antwoord();
  lees_antwoord();       
}

