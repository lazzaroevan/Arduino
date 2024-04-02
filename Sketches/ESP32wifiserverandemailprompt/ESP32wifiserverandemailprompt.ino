#include <Arduino.h>
#if defined(ESP32)
#include <WiFi.h>
#endif
#include <ESP_Mail_Client.h>
#include <WebServer.h>
#include "time.h"
#include <tuple>
#include <vector>
#include <HardwareSerial.h>
#define SMTP_HOST "smtp.gmail.com"
#define SMTP_PORT 465

#define AUTHOR_EMAIL "arduino4720@gmail.com"
#define AUTHOR_PASSWORD "vipnpjgxyoygihrf"

// for ph sensor
#include <SoftwareSerial.h>
#define rx 33
#define tx 27
SoftwareSerial phSerial(rx, tx);
String inputstring = "";
String sensorstring = "";
boolean input_string_complete = false;
boolean sensor_string_complete = false;
float pH;

#define uS_TO_S_FACTOR 1000000

using namespace std;

RTC_DATA_ATTR float lastKnownPh = 999.9;

String lastKnownPhString = "";

SMTPSession smtp;
WebServer server(80);
void smtpCallback(SMTP_Status status);

uint8_t LED1pin = 4;
bool LED1status = LOW;
uint8_t LED2pin = 5;
bool LED2status = LOW;

tuple<String, String, bool, String> textRecipients1 = make_tuple("Evan", "6318071802@vtext.com", true, "");

tuple<String, String, bool, String> textRecipients2 = make_tuple("Peter", "6318071322@vtext.com", true, "");

tuple<String, String, bool, String> emailRecipients1 = make_tuple("Evan", "lazzaroevan@gmail.com", false, "");

tuple<String, String, bool, String> emailRecipients2 = make_tuple("Peter", "Achilles@optonline.net", false, "");

std::vector<tuple<String, String, bool, String>> recipients = { textRecipients1, textRecipients2, emailRecipients1, emailRecipients2 };

bool batteryCheckBool = false;

const char* WIFI_SSID = "f9e782";
const char* WIFI_PASSWORD = "Odysseus277";

//for runtime
const char* ntpServer = "pool.ntp.org";
const long gmtOffset_sec = -18000;
const int daylightOffset_sec = 3600;

//For Battery Level
const int MAX_ANALOG_VAL = 4095;

std::pair<double, double> senseBattery() {
  int rawValue = analogRead(A13);
  // Reference voltage on ESP32 is 1.1V
  double voltageLevel = (rawValue / 4095.0) * 2 * 1.1 * 3.3;  // calculate voltage level
  double batteryFraction = (voltageLevel - 3.2) / (4.2 - 3.2);
  double roundedVolt = round(voltageLevel * 100) / 100;
  double roundedPercent = round(batteryFraction * 10000) / 100;

  return std::make_pair(roundedVolt, roundedPercent);
}

void sendEmails(vector<tuple<String, String, bool, String>> recipients, int numOfRecipients, String senderName, String subject = "Pool PH Sensor") {
  /* Set the callback function to get the sending results */
  //smtp.callback(smtpCallback);

  /* Declare the session config data */
  ESP_Mail_Session session;

  /* Set the session config */
  session.server.host_name = SMTP_HOST;
  session.server.port = SMTP_PORT;
  session.login.email = AUTHOR_EMAIL;
  session.login.password = AUTHOR_PASSWORD;
  session.login.user_domain = "";

  /* Declare the message class */
  SMTP_Message message;

  /* Connect to server with the session config */
  if (!smtp.connect(&session))
    return;

  session.time.ntp_server = F("pool.ntp.org,time.nist.gov");
  session.time.gmt_offset = -18000;
  session.time.day_light_offset = 3600;

  for (int i = 0; i < numOfRecipients; i++) {
    /* Set the message headers */
    message.sender.name = senderName;
    message.sender.email = AUTHOR_EMAIL;
    message.subject = subject;
    tuple<String, String, bool, String> curTuple = recipients.at(i);
    String msg = std::get<3>(curTuple);
    message.addRecipient(std::get<0>(curTuple), std::get<1>(curTuple));
    if (std::get<2>(curTuple)) {
      /*if plain text*/
      message.text.content = msg.c_str();
      message.text.content = msg.c_str();
      message.text.transfer_encoding = Content_Transfer_Encoding::enc_7bit;
    } else {
      /*Send HTML message if html*/
      message.html.content = msg.c_str();
      message.html.content = msg.c_str();
      message.html.transfer_encoding = Content_Transfer_Encoding::enc_7bit;
    }
    message.text.charSet = "us-ascii";

    /* Start sending Email and close the session */
    if (!MailClient.sendMail(&smtp, &message, false)) Serial.println("Error sending Email, " + smtp.errorReason());

    message.clear();
  }
}


double getPH() {
  delay(10000);
  pH = 0;
  int counter = 0;
  phSerial.begin(9600);
  phSerial.print("*OK,0\r");
  phSerial.print("L,0\r");
  phSerial.print("C,0\r");
  inputstring.reserve(10);
  sensorstring.reserve(30);
  inputstring = "R\r";
  for (int i = 0; i < 10; i++) {
    delay(5000);
    phSerial.print(inputstring);
    if (phSerial.available() > 0) {
      while (!sensor_string_complete) {       //if we see that the Atlas Scientific product has sent a character
        char inchar = (char)phSerial.read();  //get the char we just received
        sensorstring += inchar;               //add the char to the var called sensorstring
        if (inchar == '\r') {                 //if the incoming character is a <CR>
          sensor_string_complete = true;      //set the flag
        }
      }
    }
    Serial.println("PH reading: " + sensorstring);
    if (isdigit(sensorstring[0])) {  //if the first character in the string is a digit
      counter += 1;
      pH += sensorstring.toFloat();
    }
    sensorstring = "";
    sensor_string_complete = false;
  }
  if (counter != 0) {pH = pH / counter;}
  phSerial.print("Sleep\r");
  return pH;
}

pair<double, double> batteryCheckPair = senseBattery();

void print_wakeup_reason() {
  esp_sleep_wakeup_cause_t wakeup_reason;

  wakeup_reason = esp_sleep_get_wakeup_cause();

  switch (wakeup_reason) {
    case ESP_SLEEP_WAKEUP_EXT0: Serial.println("Wakeup caused by external signal using RTC_IO"); break;
    case ESP_SLEEP_WAKEUP_EXT1: Serial.println("Wakeup caused by external signal using RTC_CNTL"); break;
    case ESP_SLEEP_WAKEUP_TIMER: Serial.println("Wakeup caused by timer"); break;
    case ESP_SLEEP_WAKEUP_TOUCHPAD: Serial.println("Wakeup caused by touchpad"); break;
    case ESP_SLEEP_WAKEUP_ULP: Serial.println("Wakeup caused by ULP program"); break;
    default: Serial.printf("Wakeup was not caused by deep sleep: %d\n", wakeup_reason); break;
  }
}
void setup() {
  Serial.begin(115200);
  print_wakeup_reason();
  Serial.println();
  double ph = getPH();
  String ssid = WIFI_SSID;
  Serial.print("Connecting to: " + ssid);
  int timeOutCounter = 0;
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
    if (timeOutCounter >= 30) {
      ESP.restart();
    }
    timeOutCounter += 1;
  }

  Serial.println("");
  Serial.println("WiFi connected.");
  Serial.print("Your IP address: ");
  String ipAddress = WiFi.localIP().toString();
  Serial.println(ipAddress);
  Serial.println();
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  struct tm timeinfo;

  while (!getLocalTime(&timeinfo)) struct tm timeinfo;
  char timeHourChar[3];
  strftime(timeHourChar, 3, "%H", &timeinfo);
  char timeMinChar[3];
  strftime(timeMinChar, 3, "%M", &timeinfo);
  char timeSecChar[3];
  strftime(timeSecChar, 3, "%S", &timeinfo);

  int intTimeHour;
  int intTimeMin;
  int intTimeSec;

  int timeHour = std::stoi(timeHourChar);
  int timeMin = std::stoi(timeMinChar);
  int timeSec = std::stoi(timeSecChar);

  if (timeHour < 10) {
    intTimeHour = 9 - int(timeHour);
    intTimeMin = 59 - int(timeMin);
    intTimeSec = 60 - int(timeSec);
  } else if (timeHour < 22) {
    intTimeHour = 21 - int(timeHour);
    intTimeMin = 59 - int(timeMin);
    intTimeSec = 60 - int(timeSec);
  } else {
    intTimeHour = 10 + 23 - int(timeHour);
    intTimeMin = 59 - int(timeMin);
    intTimeSec = 60 - int(timeSec);
  }
  //adding 15 minutes to this
  long long timeToSleep = (intTimeHour * 3600) + ((intTimeMin+15) * 60) + intTimeSec;
  long long doubleTimeToSleep = timeToSleep * 1000000LL;
  esp_sleep_enable_timer_wakeup(doubleTimeToSleep);

  if (senseBattery().second <= 5) batteryLow();

  String phString = String(ph, 1);
  String batt = String(senseBattery().second, 0);
  String time = String(timeHourChar) + ":" + String(timeMinChar) + ":" + String(timeSecChar);
  if (lastKnownPh < 20) lastKnownPhString = String(lastKnownPh);
  else lastKnownPhString = "N/A";

  //setting rtc memory for last known ph
  lastKnownPh = ph;

  String emailMessageString = "<!DOCTYPE html><div style=\"color:#2f4468;\"><h1>Your pools PH is: "
                              + phString + "\nYour PH sensors battery level is: " + batt + "%</h1><p>- Sent from your ph sensor at"
                              + time + "\nGoing to sleep for: " + String(timeToSleep) + " seconds</p></div>";
  String einkMessage = phString + "," + lastKnownPhString + "," + batt + "," + time;
  String textMessageString = "Your pools PH is: " + phString + "\nYour PH sensors battery level is:" + batt + "%\nCycle run at: " + time;

  std::get<3>(recipients.at(0)) = textMessageString;
  std::get<3>(recipients.at(1)) = textMessageString;
  std::get<3>(recipients.at(2)) = emailMessageString;
  std::get<3>(recipients.at(3)) = emailMessageString;

  sendEmails(recipients, 4, "Pool PH sensor");
  Serial.println("Email Sent");

  Serial.print("Going to sleep for: " + String(timeToSleep) + " seconds or ");
  Serial.print(doubleTimeToSleep, HEX);
  Serial.println(" micro seconds (in hexadecimal)");

  esp_deep_sleep_start();
}
void batteryLow() {
  Serial.println("Battery Low");
  String htmlMsg = "<div style=\"color:#2f4468;\"><h1>Your pool sensor battery is low. The voltage has dropped below optimal levels, continued operation could result in the degredation of the battery. Please recharge the device. The device is shutting off now. Once it is recharged it will continue functioning as usual.</h1></div>";
  String txtMsg = "Your pool sensor battery is low. The voltage has dropped below optimal levels, continued operation could result in the degredation of the battery. Please recharge the device. The device is shutting off now. Once it is recharged it will continue functioning as usual.";

  std::get<3>(textRecipients1) = txtMsg;
  std::get<3>(textRecipients1) = txtMsg;
  std::get<3>(emailRecipients1) = htmlMsg;
  std::get<3>(emailRecipients2) = htmlMsg;

  sendEmails(recipients, 4, "Pool PH Sensor", "Low Battery");

  esp_deep_sleep_start();
}

void loop() {
  //Serial.println(senseBattery());
  //server.handleClient();
}

/* Callback function to get the Email sending status */
void smtpCallback(SMTP_Status status) {
  /* Print the current status */
  Serial.println(status.info());

  /* Print the sending result */
  if (status.success()) {
    Serial.println("----------------");
    ESP_MAIL_PRINTF("Message sent success: %d\n", status.completedCount());
    ESP_MAIL_PRINTF("Message sent failled: %d\n", status.failedCount());
    Serial.println("----------------\n");
    struct tm dt;

    for (size_t i = 0; i < smtp.sendingResult.size(); i++) {
      /* Get the result item */
      SMTP_Result result = smtp.sendingResult.getItem(i);
      time_t ts = (time_t)result.timestamp;
      localtime_r(&ts, &dt);

      ESP_MAIL_PRINTF("Message No: %d\n", i + 1);
      ESP_MAIL_PRINTF("Status: %s\n", result.completed ? "success" : "failed");
      ESP_MAIL_PRINTF("Date/Time: %d/%d/%d %d:%d:%d\n", dt.tm_year + 1900, dt.tm_mon + 1, dt.tm_mday, dt.tm_hour, dt.tm_min, dt.tm_sec);
      ESP_MAIL_PRINTF("Recipient: %s\n", result.recipients.c_str());
      ESP_MAIL_PRINTF("Subject: %s\n", result.subject.c_str());
    }
    Serial.println("----------------\n");
  }
}