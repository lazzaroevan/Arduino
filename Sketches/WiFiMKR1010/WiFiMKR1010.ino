#include <WiFi.h>

char ssid[] = "Security";
char pass[] = "InuYasha";
int status = WL_IDLE_STATUS;
int calls = 1;   // Execution count, so this doesn't run forever
int maxCalls = 10;   // Maximum number of times the Choreo should be executed
WiFiServer server(80);
WiFiClient client = server.available();
String header;

void setup()
{
  Serial.begin(9600);
  enable_WiFi();
  connect_WiFi();
  server.begin();
  printWifiStatus();

}
void loop()
{
  client = server.available();
  if (client) 
  {
    printWEB(); 
  }
}
void printWifiStatus() 
{
  // print the SSID of the network you're attached to:
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  // print your board's IP address:
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);

  // print the received signal strength:
  long rssi = WiFi.RSSI();
  Serial.print("signal strength (RSSI):");
  Serial.print(rssi);
  Serial.println(" dBm");

  Serial.print("To see this page in action, open a browser to http://");
  Serial.println(ip);
}
void enable_WiFi() 
{
  // check for the WiFi module:
  if (WiFi.status() == WL_NO_MODULE) 
  {
    Serial.println("Communication with WiFi module failed!");
    // don't continue
    while (true);
  }

  String fv = WiFi.firmwareVersion();
  if (fv < "1.0.0") 
  {
    Serial.println("Please upgrade the firmware");
  }
}
void connect_WiFi() 
{
  // attempt to connect to Wifi network:
  while (status != WL_CONNECTED) 
  {
    Serial.print("Attempting to connect to SSID: ");
    Serial.println(ssid);
    // Connect to WPA/WPA2 network. Change this line if using open or WEP network:
    status = WiFi.begin(ssid, pass);

    // wait 10 seconds for connection:
    delay(10000);
  }
}
void printWEB() 
{

  if (client) 
  {                             // if you get a client,
    Serial.println("new client");           // print a message out the serial port
    String currentLine = "";                // make a String to hold incoming data from the client
    while (client.connected()) 
    {            // loop while the client's connected
      if (client.available()) 
      {             // if there's bytes to read from the client,
        char c = client.read(); // read a byte, then
        header += c;            
        //Serial.write(c);                    // print it out the serial monitor
        if (c == '\n') 
        {                    // if the byte is a newline character

          // if the current line is blank, you got two newline characters in a row.
          // that's the end of the client HTTP request, so send a response:
          if (currentLine.length() == 0) 
          {

            // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
            // and a content-type so the client knows what's coming, then a blank line:
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println();

            //no idea
            client.print("<form action=\"get\"target=\"_self\">");
            client.println("<br />");
            client.print("Green, Red, Blue: " );
            client.print("<input type=\"number\" name=\"g\" max=\"255\" min=\"0\" value=\"0\">");
            client.print("<input type=\"number\" name=\"r\" max=\"255\" min=\"0\" value=\"0\">");
            client.print("<input type=\"number\" name=\"b\" max=\"255\" min=\"0\" value=\"0\">");
            client.print("<input type=\"submit\" value=\"Go\">");
            client.println("<br />");

            //create the links
            //client.print("Click <a href=\"/G\">here</a> turn the LED Green<br>");
            //client.print("Click <a href=\"/R\">here</a> turn the LED Red<br>");
            //client.print("Click <a href=\"/B\">here</a> turn the LED Blue<br>");
            //client.print("Click <a href=\"/W\">here</a> turn the LED White<br>");
            //client.print("Click <a href=\"/Off\">here</a> turn the LED Off<br>");

            //int randomReading = analogRead(A1);
            //client.print("Random reading from analog pin: ");
            //client.print(randomReading);

            // The HTTP response ends with another blank line:
            client.println();
            // break out of the while loop:
            break;
          }
          else 
          {      // if you got a newline, then clear currentLine:
            currentLine = "";
          }
        }
        else if (c != '\r') {    // if you got anything else but a carriage return character,
          currentLine += c;      // add it to the end of the currentLine
        }
        /*if (currentLine.endsWith("GET /G")) 
        {
          WiFiDrv::analogWrite(25, 255);
          WiFiDrv::analogWrite(26, 0);
          WiFiDrv::analogWrite(27, 0);
        }
        if (currentLine.endsWith("GET /R")) {
        WiFiDrv::analogWrite(25, 0);
        WiFiDrv::analogWrite(26, 255);
        WiFiDrv::analogWrite(27, 0);
        }
        if (currentLine.endsWith("GET /B")) {
        WiFiDrv::analogWrite(25, 0);
        WiFiDrv::analogWrite(26, 0);
        WiFiDrv::analogWrite(27, 255);
        }
        if (currentLine.endsWith("GET /W")) {
        WiFiDrv::analogWrite(25, 255);
        WiFiDrv::analogWrite(26, 255);
        WiFiDrv::analogWrite(27, 0);
        }
        if (currentLine.endsWith("GET /Off")) {
        WiFiDrv::analogWrite(25, 0);
        WiFiDrv::analogWrite(26, 0);
        WiFiDrv::analogWrite(27, 0);
        }
        */
      }
    }
    // close the connection:
    client.stop();
    Serial.println("client disconnected");
    int headerInt = header.indexOf("GET /get?");
      if ((headerInt)>=(0))
      {
        headerInt = headerInt + 9;
        String cutHeader = header.substring(headerInt ,headerInt + 17);
        Serial.println(cutHeader);
        int greenIndex = cutHeader.indexOf("g=") + 2;
        int redIndex = cutHeader.indexOf("r=") + 2;
        int blueIndex = cutHeader.indexOf("b=") + 2;
        String greenString = cutHeader.substring(greenIndex,greenIndex + 3);
        String redString = cutHeader.substring(redIndex,redIndex + 3);
        String blueString = cutHeader.substring(blueIndex,blueIndex + 3);
        char greenArray[3] = {0,0,0};
        greenString.toCharArray(greenArray, 3);
        char redArray[3] = {0,0,0};
        redString.toCharArray(redArray, 3);
        char blueArray[3] = {0,0,0};
        blueString.toCharArray(blueArray, 3);
        String blueValStr = "";
        String redValStr = "";
        String greenValStr = "";
        for (int i = 0; i < sizeof(blueArray); i++)
        {
          if(isdigit(blueArray[i]))
          {
            blueValStr += blueArray[i];
          }
        }
        for (int i = 0; i < sizeof(redArray); i++)
        {
          if(isdigit(redArray[i]))
          {
            redValStr += redArray[i];
          }
        }
        for (int i = 0; i < sizeof(greenArray); i++)
        {
          if(isdigit(greenArray[i]));
          {
            greenValStr += greenArray[i];
          }
        }
        int greenVal = greenValStr.toInt();
        int blueVal = blueValStr.toInt();
        int redVal = redValStr.toInt();
        WiFiDrv::analogWrite(25, greenVal);
        WiFiDrv::analogWrite(26, redVal);
        WiFiDrv::analogWrite(27, blueVal);

        //int rVal = header.substring(,);
        //int gVal = ;
        //int bVal = ;
        header = "";
      }        
  }
}





void sendEmail(String ipAddress) 
{

}
