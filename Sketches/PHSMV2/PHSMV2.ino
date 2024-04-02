//This code was written to be easy to understand.
//Modify this code as you see fit.
//This code will output data to the Arduino serial monitor.
//Type commands into the Arduino serial monitor to control the pH circuit.
//This code was written in the Arduino 2.0 IDE
//An Arduino UNO was used to test this code.
//This code was last tested 10/2022


#include <SoftwareSerial.h>                           //we have to include the SoftwareSerial library, or else we can't use it
#include "TFT_eSPI.h"
#include "Free_Fonts.h"
#include "pin_config.h"
#define rx 12                                          //define what pin rx is going to be
#define tx 13                                          //define what pin tx is going to be

// Display.
#define     DISPLAY_HEIGHT          170                         // T-Display-S3 display height in pixels.
#define     DISPLAY_WIDTH           320                         // T-Display-S3 display width in pixels.
#define     DISPLAY_BRIGHTNESS_MAX  252                         // T-Display-S3 display brightness maximum.
#define     DISPLAY_BRIGHTNESS_MIN  0                           // T-Display-S3 display brightness minimum.

TFT_eSPI    lcd = TFT_eSPI();                                   // T-Display-S3 lcd.
int         lcdBacklightBrightness = DISPLAY_BRIGHTNESS_MAX;     // T-Display-S3 brightness.


SoftwareSerial myserial(rx, tx);                      //define how the soft serial port is going to work


String inputstring = "";                              //a string to hold incoming data from the PC
String sensorstring = "";                             //a string to hold the data from the Atlas Scientific product
boolean input_string_complete = false;                //have we received all the data from the PC
boolean sensor_string_complete = false;               //have we received all the data from the Atlas Scientific product
float pH;                                             //used to hold a floating point number that is the pH
float uVolt;

void print_wakeup_reason(){
  esp_sleep_wakeup_cause_t wakeup_reason;

  wakeup_reason = esp_sleep_get_wakeup_cause();

  switch(wakeup_reason)
  {
    case ESP_SLEEP_WAKEUP_EXT0 : Serial.println("Wakeup caused by external signal using RTC_IO"); break;
    case ESP_SLEEP_WAKEUP_EXT1 : Serial.println("Wakeup caused by external signal using RTC_CNTL"); break;
    case ESP_SLEEP_WAKEUP_TIMER : Serial.println("Wakeup caused by timer"); break;
    case ESP_SLEEP_WAKEUP_TOUCHPAD : Serial.println("Wakeup caused by touchpad"); break;
    case ESP_SLEEP_WAKEUP_ULP : Serial.println("Wakeup caused by ULP program"); break;
    default : Serial.printf("Wakeup was not caused by deep sleep: %d\n",wakeup_reason); break;
  }
}

void setup() 
{  
  pinMode(PIN_POWER_ON, OUTPUT);
  digitalWrite(PIN_POWER_ON, HIGH); 
  pinMode(15, OUTPUT);
  digitalWrite(15, 1);
  
  //Screen
  lcd.init();
  lcd.setRotation(1);
  analogReadResolution(12);
  ledcSetup(0, 10000, 8);
  ledcAttachPin(38, 0);
  ledcWrite(0, lcdBacklightBrightness);
  lcd.setTextFont(GLCD);
  lcd.setFreeFont(FSB9);
  lcd.fillScreen(TFT_BLACK);
  
  //set up the hardware
  Serial.begin(9600);                                 //set baud rate for the hardware serial port_0 to 9600
  myserial.begin(9600);                               //set baud rate for the software serial port to 9600
  inputstring.reserve(10);                            //set aside some bytes for receiving data from the PC
  sensorstring.reserve(30);                           //set aside some bytes for receiving data from Atlas Scientific product
  esp_sleep_enable_ext0_wakeup(GPIO_NUM_14,0);
  myserial.flush();
  Serial.flush();
  myserial.print("*OK,0\r");
  myserial.print("*OK,0\r");
  myserial.print("C,0\r");                      //send that string to the Atlas Scientific product
                                              //add a <CR> to the end of the string
  
  print_wakeup_reason();
}


void serialEvent() 
{                                  //if the hardware serial port_0 receives a char
  inputstring = Serial.readStringUntil(13);           //read the string until we see a <CR>
  input_string_complete = true;                       //set the flag used to tell if we have received a completed string from the PC
}


void loop() 
{                                         //here we go...
  uVolt = (analogRead(4) * 2 * 3.3 * 1000) / 4096;
  myserial.print('R');
  myserial.print('\r');
  delay(900);
  if (myserial.available() > 0) 
  {                     //if we see that the Atlas Scientific product has sent a character
    char inchar = (char)myserial.read();              //get the char we just received
    sensorstring += inchar;                           //add the char to the var called sensorstring
    if (inchar == '\r') 
    {                             //if the incoming character is a <CR>
      sensor_string_complete = true;                  //set the flag
    }
  }


  if (sensor_string_complete == true) 
  {               //if a string from the Atlas Scientific product has been received in its entirety
    String stringPrint = "PH: " + sensorstring;
    Serial.println(sensorstring);                     //send that string to the PC's serial monitor
    lcd.fillScreen(TFT_BLACK);
    lcd.drawString(sensorstring,50,60,4);
    lcd.drawString("PH:",5,60,4);
    float battVolt = uVolt/1000;
    float battPercent = 100 * (battVolt - 3.2);
    lcd.drawString(String(battPercent),0,0,2);
    sensorstring = "";                                //clear the string
    sensor_string_complete = false;                   //reset the flag used to tell if we have received a completed string from the Atlas Scientific product
  }
  if((digitalRead(14) == 0))
  {
    delay(2000);
    myserial.flush();
    Serial.flush();
    myserial.print("C,0");
    myserial.print("\r");
    delay(1000);
    myserial.print("Sleep");                      //send that string to the Atlas Scientific product
    myserial.print('\r');                             //add a <CR> to the end of the string
    myserial.flush();
    Serial.flush();
    esp_deep_sleep_start();  
  }
  if(digitalRead(0) == 0)
  { 
    delay(2000);
    lcd.fillScreen(TFT_BLACK);
    lcd.drawString("Run Calibration?",5,45,4);
    lcd.drawString("Yes -->",275,10,4);
    lcd.drawString("No  -->",275,150,4);
    if(digitalRead(0) == 0)
    {
      myserial.print('C,0');
      myserial.print('\r');
      myserial.flush();
      Serial.flush();
      //Calibration Code
      while(digitalRead(0) != 1)
      {
        delay(1000);
        myserial.print('R');
        myserial.print('\r');
        if (myserial.available() > 0) 
        {                     //if we see that the Atlas Scientific product has sent a character
          char inchar = (char)myserial.read();              //get the char we just received
          sensorstring += inchar;                           //add the char to the var called sensorstring
          if (inchar == '\r') 
          {                             //if the incoming character is a <CR>
            sensor_string_complete = true;                  //set the flag
          }
        }
        if(sensor_string_complete)
        {
          lcd.fillScreen(TFT_BLACK);
          lcd.drawString("Put probe end into PH 7 Solution",5,45,4);
          lcd.drawString("Wait for probe readings to stabilize",5,60,4);
          lcd.drawString("When Stable -->",275,10,4);
          lcd.drawString("PH: " + sensorstring,5,75,4);
          sensor_string_complete = false;
        }
      }
      myserial.print('Cal,mid,7.00');   
    }
    else if(digitalRead(14) == 0)
    {
      lcd.fillScreen(TFT_BLACK);
      lcd.drawString("Returning To Main Menu",5,60,4);
      delay(2000);
    }
  }
}
