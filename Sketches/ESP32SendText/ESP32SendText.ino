#include <Arduino.h>
#if defined(ESP32)
  #include <WiFi.h>
#endif
#include <ESP_Mail_Client.h>
#include <WebServer.h>

#define SMTP_HOST "smtp.gmail.com"
#define SMTP_PORT 465

/* The gmail sign in credentials */
#define AUTHOR_EMAIL "arduino4720@gmail.com"
#define AUTHOR_PASSWORD "vipnpjgxyoygihrf"

/* Recipient's email*/
#define RECIPIENT_EMAIL1 "boomandot@gmail.com"
#define RECIPIENT_EMAIL2 "lazzaroevan@gmail.com"
#define RECIPIENT_EMAIL3 "achilles@optonline.net"

/* The SMTP Session object used for Email sending */
SMTPSession smtp;
// webserver
WebServer server(80);

/* Callback function to get the Email sending status */
void smtpCallback(SMTP_Status status);

uint8_t LED1pin = 4;
bool LED1status = LOW;

uint8_t LED2pin = 5;
bool LED2status = LOW;

const char* recipients[][2] = {{"Evan","lazzaroevan@gmail.com"},{"Evan","boomandot@gmail.com"}};
const char* WIFI_SSID = "Security";
const char* WIFI_PASSWORD = "InuYasha";

void setup()
{

  Serial.begin(115200);
  Serial.println();
  pinMode(LED1pin, OUTPUT);
  pinMode(LED2pin, OUTPUT);
  String ssid = WIFI_SSID;
  Serial.print("Connecting to: " + ssid );
  int timeOutCounter = 0;
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) 
  {
    delay(1000);
    Serial.print(".");
    if(timeOutCounter >= 30)
    {
      ESP.restart();
    }
    timeOutCounter += 1;

  }

  Serial.println("");
  Serial.println("WiFi connected.");
  Serial.println("Your IP address: ");
  String ipAddress = WiFi.localIP().toString();
  Serial.println(ipAddress);
  Serial.println();

  //setting up server
  server.on("/", handle_OnConnect);
  server.on("/led1on", handle_led1on);
  server.on("/led1off", handle_led1off);
  server.on("/led2on", handle_led2on);
  server.on("/led2off", handle_led2off);
  server.onNotFound(handle_NotFound);
  server.begin();
  Serial.println("HTTP server started");


  /** Enable the debug via Serial port
   * none debug or 0
   * basic debug or 1
  */
  //smtp.debug(1);

  /* Set the callback function to get the sending results */
  smtp.callback(smtpCallback);

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

  /* Set the message headers */
  message.sender.name = "My Pool Sensor";
  message.sender.email = AUTHOR_EMAIL;
  message.subject = "Your PH Sensor IPAddress";
  
  message.addRecipient("Evan",RECIPIENT_EMAIL1);
  message.addRecipient("Evan",RECIPIENT_EMAIL2);
  //message.addRecipient("Peter",RECIPIENT_EMAIL3);

  /*Send HTML message*/
  String htmlMsg = "<div style=\"color:#2f4468;\"><h1>Visit this IPAddress for your pool sensor: <a href =http://"+ ipAddress +">here</h1><p>- Sent from your ph sensor</p></div>";
  message.html.content = htmlMsg.c_str();
  message.html.content = htmlMsg.c_str();
  message.text.charSet = "us-ascii";
  message.html.transfer_encoding = Content_Transfer_Encoding::enc_7bit;

  /* Connect to server with the session config */
  if (!smtp.connect(&session))
    return;

  /* Start sending Email and close the session */
  if (!MailClient.sendMail(&smtp, &message))
    Serial.println("Error sending Email, " + smtp.errorReason());
  Serial.println("Entering Main Loop");
}

void loop()
{
  server.handleClient();
  if(LED1status)
  {digitalWrite(LED1pin, HIGH);}
  else
  {digitalWrite(LED1pin, LOW);}
  
  if(LED2status)
  {digitalWrite(LED2pin, HIGH);}
  else
  {digitalWrite(LED2pin, LOW);}
}

void handle_OnConnect() 
{
  server.send(200, "text/html", SendHTML(LED1status,LED2status)); 
}
void handle_led1on() 
{
  server.send(200, "text/html", SendHTML(true,LED2status)); 
}
void handle_led1off()
{
  server.send(200, "text/html", SendHTML(false,LED2status)); 
}

void handle_led2on() 
{
  server.send(200, "text/html", SendHTML(LED1status,true)); 
}

void handle_led2off() 
{
  server.send(200, "text/html", SendHTML(LED1status,false)); 
}

void handle_NotFound()
{
  server.send(404, "text/plain", "Not found");
}
String SendHTML(uint8_t led1stat,uint8_t led2stat)
{
  String ptr = "<!DOCTYPE html> <html>\n";
  ptr +="<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0, user-scalable=no\">\n";
  ptr +="<title>LED Control</title>\n";
  ptr +="<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}\n";
  ptr +="body{margin-top: 50px;} h1 {color: #444444;margin: 50px auto 30px;} h3 {color: #444444;margin-bottom: 50px;}\n";
  ptr +=".button {display: block;width: 80px;background-color: #3498db;border: none;color: white;padding: 13px 30px;text-decoration: none;font-size: 25px;margin: 0px auto 35px;cursor: pointer;border-radius: 4px;}\n";
  ptr +=".button-on {background-color: #3498db;}\n";
  ptr +=".button-on:active {background-color: #2980b9;}\n";
  ptr +=".button-off {background-color: #34495e;}\n";
  ptr +=".button-off:active {background-color: #2c3e50;}\n";
  ptr +="p {font-size: 14px;color: #888;margin-bottom: 10px;}\n";
  ptr +="</style>\n";
  ptr +="</head>\n";
  ptr +="<body>\n";
  ptr +="<h1>ESP32 Web Server</h1>\n";
  ptr +="<h3>Using Station(STA) Mode</h3>\n";
  
   if(led1stat)
  {
    ptr +="<p>LED1 Status: ON</p><a class=\"button button-off\" href=\"/led1off\">OFF</a>\n";
  }
  else
  {
    ptr +="<p>LED1 Status: OFF</p><a class=\"button button-on\" href=\"/led1on\">ON</a>\n";
  }

  if(led2stat)
  {
    ptr +="<p>LED2 Status: ON</p><a class=\"button button-off\" href=\"/led2off\">OFF</a>\n";
  }
  else
  {
    ptr +="<p>LED2 Status: OFF</p><a class=\"button button-on\" href=\"/led2on\">ON</a>\n";
  }

  ptr +="</body>\n";
  ptr +="</html>\n";
  return ptr;
}
/* Callback function to get the Email sending status */
void smtpCallback(SMTP_Status status){
  /* Print the current status */
  Serial.println(status.info());

  /* Print the sending result */
  if (status.success()){
    Serial.println("----------------");
    ESP_MAIL_PRINTF("Message sent success: %d\n", status.completedCount());
    ESP_MAIL_PRINTF("Message sent failled: %d\n", status.failedCount());
    Serial.println("----------------\n");
    struct tm dt;

    for (size_t i = 0; i < smtp.sendingResult.size(); i++){
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