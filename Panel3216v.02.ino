#include <DS3232RTC.h>
#include <TimeLib.h>
#include <Time.h>
#include <Wire.h>
#include <SPI.h>
#include <DMD2.h>
#include "OneButton.h"
#include <fonts/MyBigFont.h>
#include <fonts/Arial_Black_16.h>
#include <fonts/Droid_Sans_16.h>
#include <fonts/Droid_Sans_12.h>
#include <fonts/Arial14.h>
#include <fonts/Arial14my.h>
#include <fonts/Courier14.h>

#define DST_PIN 5  // Define DST adjust button pin
#define MIN_PIN 4  // Define Minutes adjust button pin
#define HOUR_PIN 3  // Define Hours adjust button pin
#define DISPLAYS_WIDE 1
#define DISPLAYS_HIGH 1
#define PIN_OE 9
#define PIN_A 6
#define PIN_B 7
#define PIN_SCK 8
#define PIN_CLK 13
#define PIN_MOSI 11

bool DST = false; //DST state
bool TempShow = true;
unsigned long lastTimeDOT = 0; 
unsigned long lastTimeShowTemp = 0; 
bool flagButtonHClick = false; bool flagButtonMClick = false;
bool flagButtonHPress = false; bool flagButtonMPress = false;
bool flagAnyButtonClick = false;
uint8_t oldHour = 25;
uint8_t oldMinute = 61;
byte i=0; 

//ClickButton buttonDST(DST_PIN, LOW, CLICKBTN_PULLUP);
OneButton buttonH(HOUR_PIN, true, true); //N-pin, _buttonPressed = LOW;, pinMode(pin, INPUT_PULLUP)
OneButton buttonM(MIN_PIN, true, true); //N-pin, _buttonPressed = LOW;, pinMode(pin, INPUT_PULLUP)
SoftDMD dmd(DISPLAYS_WIDE, DISPLAYS_HIGH, PIN_OE, PIN_A, PIN_B, PIN_SCK, PIN_CLK, PIN_MOSI);
DMD_TextBox boxH(dmd, 0, 3, 15, 0); //(DMDFrame &dmd, int left = 0, int top = 0, int width = 0, int height = 0);
DMD_TextBox boxH1(dmd, 3, 3, 10, 0); //Position for one digit 
DMD_TextBox boxM(dmd, 18, 3, 16, 0); 
DMD_TextBox boxDOT(dmd, 12, 2, 5, 0);
DMD_TextBox boxFull(dmd, 7, 0, 0, 0); 

void setup(){ 
  Serial.begin(9600); 
  randomSeed(analogRead(0));
  Wire.begin();
  buttonH.attachClick(clickH);
  buttonM.attachClick(clickM);
  buttonH.attachDuringLongPress(longPressH);
  buttonM.attachDuringLongPress(longPressM);
  buttonH.attachLongPressStop(LongPressStopH);
  buttonM.attachLongPressStop(LongPressStopM);
  dmd.setBrightness(50);
  dmd.begin();
  dmd.clearScreen();
  dmd.selectFont(Arial_Black_16);   
  Serial.println(F("Ready..."));  
} 

void TimeAdjust() {
  if (flagAnyButtonClick) { 
    tmElements_t Now;
    RTC.read(Now);
    int hour=Now.Hour;
    int minutes=Now.Minute;
    int second =Now.Second;
    if (flagButtonHClick) {
        if (Now.Hour== 23) {Now.Hour=0;} else {Now.Hour += 1;};
        flagButtonHClick = false;
    }
    if (flagButtonMClick) {
        if (Now.Minute== 59){Now.Minute=0;} else {Now.Minute += 1;};
        flagButtonMClick = false;
    }
    if (flagButtonHPress) {
        if (Now.Hour== 23) {Now.Hour=0;} else {Now.Hour += 1;};
    }    
    if (flagButtonMPress) {
        if (Now.Minute== 59){Now.Minute=0;} else {Now.Minute += 1;};
    }    
    RTC.write(Now);
    Serial.print("Change "); Serial.print(Now.Hour); Serial.print(":"); Serial.println(Now.Minute); 
    flagAnyButtonClick = false;
  }
}

void TimeToDisplay(){ 
  String strTime;
  static bool Dot = true;  //Dot state
  dmd.selectFont(Droid_Sans_12); 
  tmElements_t Now;
  RTC.read(Now);  //time_t Now = RTC.Now();// Getting the current Time and storing it into a DateTime object 
  uint8_t hour=Now.Hour;
  uint8_t minute=Now.Minute;
  uint8_t second =Now.Second;
  strTime = String(hour) + ":" + String(minute) + ":" + String(second);
  if (hour != oldHour) {
    boxH.clear();
    hour < 10 ? boxH1.println(hour): boxH.println(hour);
    oldHour = hour;
  }
  
  if (millis() - lastTimeDOT > 1000)  {
    lastTimeDOT = millis();
    if (Dot) {
      boxDOT.clear();
    }
    else {
      boxDOT.println(":");  
    }
    Dot = !Dot;
    Serial.println(strTime);
  }

  if (minute != oldMinute) {
    boxM.clear();
    if (minute < 10) {
      boxM.print("0"); boxM.println(minute);  
    }
    else {boxM.println(minute);}
    oldMinute = minute;
  }

};

void StringToDisplay(char *infoArr){
  while(*infoArr) {
    Serial.print(*infoArr); 
    boxFull.print(*infoArr);
    delay(150);
    infoArr++;
  } 
  Serial.println();
  oldHour = 25; 
  oldMinute = 61;  
}

void TempToDisplay(){ 
  int t = RTC.temperature();
  Serial.print("Temp is: ");Serial.println(t);  
}


/*void DSTcheck(){
   // Serial.print("DST is: ");Serial.println(DST);
   if (buttonDST.flagPress){
    DST = !DST;
   };
}*/

void loop()  { // Main loop
  //DSTcheck(); // Check DST
  buttonH.tick(); 
  buttonM.tick(); 
  TimeAdjust(); // Check to se if time is geting modified
  TimeToDisplay();

  if ((millis() - lastTimeShowTemp >7000) && TempShow) {
    //dmd.clearScreen();
    TempToDisplay(); 
    //delay(2000);
    //boxFull.clear();
    lastTimeShowTemp = millis(); 
  }
  
}

void clickH() {
  flagButtonHClick = true;
  Serial.println("Button H click.");
  flagAnyButtonClick = true;
} 

void clickM() {
  flagButtonMClick = true;
  Serial.println("Button M click.");
  flagAnyButtonClick = true;
} 

void longPressH() {
  flagButtonHPress = true;
  Serial.println("Button H longPress...");
  flagAnyButtonClick = true;
} // longPress1

void longPressM() {
  flagButtonMPress = true;
  Serial.println("Button M longPress...");
  flagAnyButtonClick = true;
} // longPress1

void LongPressStopH() {
  flagButtonHPress = false;
  flagAnyButtonClick = false;  
  Serial.println("Button H LongPressStop...");
}

void LongPressStopM() {
  flagButtonMPress = false;
  flagAnyButtonClick = false;  
  Serial.println("Button M LongPressStop...");
}
