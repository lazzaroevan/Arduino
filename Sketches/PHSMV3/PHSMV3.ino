#include <SoftwareSerial.h>                           //we have to include the SoftwareSerial library, or else we can't use it
#include "TFT_eSPI.h"
#include "Free_Fonts.h"
#include "pin_config.h"
#define rx 12                                          //define what pin rx is going to be
#define tx 13                                          //define what pin tx is going to be
#define     DISPLAY_HEIGHT          170                         // T-Display-S3 display height in pixels.
#define     DISPLAY_WIDTH           320                         // T-Display-S3 display width in pixels.
#define     DISPLAY_BRIGHTNESS_MAX  252                         // T-Display-S3 display brightness maximum.
#define     DISPLAY_BRIGHTNESS_MIN  0                           // T-Display-S3 display brightness minimum.

TFT_eSPI    lcd = TFT_eSPI();                                   // T-Display-S3 lcd.
int         lcdBacklightBrightness = DISPLAY_BRIGHTNESS_MAX;     // T-Display-S3 brightness.
float battery_level;
bool sensor_string_complete = false;
String inputstring;
String sensorstring;

SoftwareSerial myserial(rx, tx);                      //define how the soft serial port is going to work

void print_wakeup_reason()
{
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

double get_battery_level()
{
  return 100 * (((analogRead(4) * 2 * 3.3) / 4096)-3.2);
}

void flush_serials()
{
  Serial.flush();
  myserial.flush();
}

void populate_screen(String message = "", int messagex = 0, int messagey = 0, int msize = 4, String top_button = "", int topx = 0, int topy = 0,int tsize = 4, String bot_button = "",int botx = 0, int boty = 0,int bsize = 4)
{
  String battery = String(get_battery_level());
  lcd.fillScreen(TFT_BLACK);
  lcd.drawString(battery + "%",0,0,2);
  lcd.drawString(message,messagex,messagey,msize);
  lcd.drawString(top_button,topx,topy,tsize);
  lcd.drawString(bot_button,botx,boty,bsize);  
}

void take_sample(String message = "", int messagex = 5, int messagey = 60, int msize = 4, String top_button = "", int topx = 0, int topy = 0,int tsize = 4, String bot_button = "",int botx = 0, int boty = 0,int bsize = 4)
{
  if (myserial.available() > 0)                       //if we see that the Atlas Scientific product has sent a character (code copied from atlas scientific)
  {
    char inchar = (char)myserial.read();              //get the char we just received
    sensorstring += inchar;                           //add the char to the var called sensorstring
    if (inchar == '\r')                               //if the incoming character is a <CR>
    {                             
      sensor_string_complete = true;                  //set the flag
    }
  }
  if(sensor_string_complete)
  {
    populate_screen("PH: "+sensorstring + message,messagex,messagey,msize,top_button,topx,topy,tsize,bot_button,botx,boty,bsize);
    sensor_string_complete = false;
    sensorstring = "";
  }
}

bool calibration()
{
  lcd.fillScreen(TFT_BLACK);
  delay(2000);
  populate_screen("Run Calibration?",5,45,4,"Yes -->",275,10,4,"No  -->",275,150,4);
  if(digitalRead(14) == 0) {return false;}
  if(digitalRead(0) == 0)
  {
    //Calibrate probe at ph 7
    lcd.fillScreen(TFT_BLACK);
    populate_screen("Put the probe tip into\nthe PH 7 solution and\nwait for it to stabilize",5,40,4);
    flush_serials();
    delay(5000);
    while(digitalRead(0) != 0)
    {
      take_sample(" Stabilized?",5,60,4,"Yes -->",275,10,4,"No  -->",275,150,4);
    }
    myserial.print("Cal,mid,7.00\r");
    //Calibrate probe at ph 4
    lcd.fillScreen(TFT_BLACK);
    populate_screen("Put the probe tip into\nthe PH 4 solution and\nwait for it to stabilize",5,40,4);
    flush_serials();
    delay(5000);
    while(digitalRead(0) != 0)
    {
      take_sample(" Stabilized?",5,60,4,"Yes -->",275,10,4,"No  -->",275,150,4);
    }
    myserial.print("Cal,low,4.00\r");
    //Calibrate probe at ph 10
    lcd.fillScreen(TFT_BLACK);
    populate_screen("Put the probe tip into\nthe PH 4 solution and\nwait for it to stabilize",5,40,4);
    flush_serials();
    delay(5000);
    while(digitalRead(0) != 0)
    {
      take_sample(" Stabilized?",5,60,4,"Yes -->",275,10,4,"No  -->",275,150,4);
    }
    myserial.print("Cal,high,10.00\r");
    return true;
  }
}

void sleep()
{
  delay(2000);
  myserial.flush();
  Serial.flush();
  delay(1000);
  myserial.print("C,0\r");
  delay(1000);
  myserial.print("Sleep\r");
  esp_deep_sleep_start();
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
  esp_sleep_enable_ext0_wakeup(GPIO_NUM_14,0);        //enable wakeup from top button

  //clear serials
  myserial.flush();
  Serial.flush();

  //wait for ezo module to come online
  delay(5000);

  //load default values to ezo module
  myserial.print("*OK,0\r");  //turns off code responses
  myserial.print("*OK,0\r");  //second check to turn off code responses
  myserial.print("C,1\r");    //turns auto sample off (sets time to between samples to 0)

  //print wakeup reason
  print_wakeup_reason();    
}

void loop() 
{
  take_sample();
  if(digitalRead(0) == 0) {calibration();}
  if(digitalRead(14) == 0) {sleep();}
}
