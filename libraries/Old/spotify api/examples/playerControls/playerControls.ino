/*******************************************************************
    Controls spotify player using an ESP32 or ESP8266

    Supports:
        - Next Track
        - Previous Track
        - Seek

    NOTE: You need to get a Refresh token to use this example
    Use the getRefreshToken example to get it.

    Compatible Boards:
	  - Any ESP8266 or ESP32 board

    Parts:
    ESP32 D1 Mini style Dev board* - http://s.click.aliexpress.com/e/C6ds4my

 *  * = Affiliate

    If you find what I do useful and would like to support me,
    please consider becoming a sponsor on Github
    https://github.com/sponsors/witnessmenow/
 *******************************************************************/

// ----------------------------
// Standard Libraries
// ----------------------------

#if defined(ESP8266)
#include <ESP8266WiFi.h>
#elif defined(ESP32)
#include <WiFi.h>
#endif

#include "TFT_eSPI.h"
#include <WiFi.h>
#include "time.h"
#include "SpotifyArduino.h"
#include <WiFiClientSecure.h>
#include <time.h>
#include "Free_Fonts.h"

// ----------------------------
// Additional Libraries - each one of these will need to be installed.
// ----------------------------

#include <SpotifyArduino.h>
// Library for connecting to the Spotify API

// Install from Github
// https://github.com/witnessmenow/spotify-api-arduino

// including a "spotify_server_cert" variable
// header is included as part of the SpotifyArduino libary
#include <SpotifyArduinoCert.h>

#include <ArduinoJson.h>
// Library used for parsing Json from the API responses

// Search for "Arduino Json" in the Arduino Library manager
// https://github.com/bblanchon/ArduinoJson

//------- Replace the following! ------

// Wifi.
String      stringIP;                                           // IP address.
char ssid[] = "f9e782";         // your network SSID (name)
char password[] = "Odysseus277"; // your network password

char songTitle[50];
char songAlbum[50];
char songArtist[50];
long progress = 0;
long duration = 1;
float percentProgress = 0;
float counter = 10000;

char clientId[] = "b017e997ac0f4047ab1e3f7cc274104b";     // Your client ID of your spotify APP
char clientSecret[] = "e6d886f8ef0941ed921f4ab0bef8b9d9"; // Your client Secret of your spotify APP (Do Not share this!)



// Country code, including this is advisable
#define SPOTIFY_MARKET "US"

#define SPOTIFY_REFRESH_TOKEN "AQBt5iaKOGPgMlji1OSWKvxegTFEvdRE3rN__a_V2YMXX50jlvd3kiN7C9vojfH5dl-qFSljQyB-21uxie-_zhi2GwWTgw2tx5zkZsG0KyvnceLcsKektJX0ZMaLzhd7bec"

//------- ---------------------- ------

WiFiClientSecure client;
SpotifyArduino spotify(client, clientId, clientSecret, SPOTIFY_REFRESH_TOKEN);

// Display.
#define     DISPLAY_HEIGHT          170                         // T-Display-S3 display height in pixels.
#define     DISPLAY_WIDTH           320                         // T-Display-S3 display width in pixels.
#define     DISPLAY_BRIGHTNESS_MAX  252                         // T-Display-S3 display brightness maximum.
#define     DISPLAY_BRIGHTNESS_MIN  0                           // T-Display-S3 display brightness minimum.

// Sprites.
#define     SPRITE_BATTERY_FONT     2                           // Battery sprite font size.
#define     SPRITE_BATTERY_HEIGHT   30                          // Battery sprite height in pixels.
#define     SPRITE_BATTERY_WIDTH    100                         // Battery sprite width in pixels.
#define     SPRITE_DATE_FONT        4                           // Date sprite font size.
#define     SPRITE_DATE_HEIGHT      40                          // Date sprite height in pixels.
#define     SPRITE_DATE_WIDTH       200                         // Date sprite width in pixels.
#define     SPRITE_TIME_FONT        2                           // Time sprite font size.
#define     SPRITE_TIME_HEIGHT      30                          // Time sprite height in pixels.
#define     SPRITE_TIME_WIDTH       100                         // Time sprite width in pixels.

uint32_t    uVolt;                                              // Battery voltage.

// Display (T-Display-S3 lcd display).

TFT_eSPI    lcd = TFT_eSPI();                                   // T-Display-S3 lcd.
int         lcdBacklighBrightness = DISPLAY_BRIGHTNESS_MAX;     // T-Display-S3 brightness.

// Sprites.

TFT_eSprite spriteBackground = TFT_eSprite(& lcd);              // Background sprite.
TFT_eSprite spriteBattery = TFT_eSprite(& lcd);                 // Battery sprite.
TFT_eSprite spriteDate = TFT_eSprite(& lcd);                    // Date sprite.
TFT_eSprite spriteTime = TFT_eSprite(& lcd);                    // Time sprite.

// Time.

char        chDayOfMonth[3];                                    // Day of month (0 through 31).
const int   nDaylightOffsetSeconds = 3600;                      // Daylight savings time offset.
char        chDayofWeek[4];                                     // Day of week (Sunday through Saturday).
const long  lGmtOffsetSeconds = -5 * 3600;                      // Time zone offset.
char        chHour[3];                                          // Hour.
char        chMinute[3];                                        // Minute.
char        chMonth[4];                                         // Month.
const char* chNtpServer = "pool.ntp.org";                       // NTP time server.
char        chSecond[3];                                        // Second.
char        chYear[5];                                          // Year.
float msgWidth;


void setup()
{
  lcd.setTextFont(GLCD);
  lcd.setFreeFont(FSB9);
  String serialPos = "Starting Serial";
  msgWidth = lcd.textWidth(serialPos);
  lcd.drawString(songTitle,(DISPLAY_WIDTH-msgWidth)/2,DISPLAY_HEIGHT/2,4);

  
  Serial.begin(115200);
  analogReadResolution(12);
  
  lcd.init();
  lcd.setRotation(3);
  ledcSetup(0, 10000, 8);
  ledcAttachPin(38, 0);
  ledcWrite(0, lcdBacklighBrightness);

  //Battery
  spriteBattery.createSprite(SPRITE_BATTERY_WIDTH, SPRITE_BATTERY_HEIGHT);
  spriteBattery.setSwapBytes(true);
  spriteBattery.setTextColor(TFT_WHITE, TFT_WHITE);

  // Enable the modeul to operate from an external LiPo battery.
  
  pinMode(15, OUTPUT);
  digitalWrite(15, 1);

  // Background sprite.

  spriteBackground.createSprite(DISPLAY_WIDTH, DISPLAY_HEIGHT);
  spriteBackground.setSwapBytes(true);
  spriteBackground.setTextColor(TFT_WHITE, TFT_BLACK);

  // Date sprite.

  spriteDate.createSprite(SPRITE_DATE_WIDTH, SPRITE_DATE_HEIGHT);
  spriteDate.setSwapBytes(true);
  spriteDate.setTextColor(TFT_WHITE, TFT_BLACK);

  // Time sprite.

  spriteTime.createSprite(SPRITE_TIME_WIDTH, SPRITE_TIME_HEIGHT);
  spriteTime.setSwapBytes(true);
  spriteTime.setTextColor(TFT_WHITE, TFT_BLACK);
  
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.println("");

  // Wait for connection
  while (WiFi.status() != WL_CONNECTED)
  {
      delay(500);
      Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  stringIP = WiFi.localIP().toString();

      // Handle HTTPS Verification
  #if defined(ESP8266)
      client.setFingerprint(SPOTIFY_FINGERPRINT); // These expire every few months
  #elif defined(ESP32)
      client.setCACert(spotify_server_cert);
  #endif

    // If you want to enable some extra debugging
    // uncomment the "#define SPOTIFY_DEBUG" in SpotifyArduino.h

  Serial.println("Refreshing Access Tokens");
  if (!spotify.refreshAccessToken())
  {
      Serial.println("Failed to get access tokens");
  }

  currentPlay();
}

void getCurrentPlaying(CurrentlyPlaying currentlyPlaying)
{
    // Use the details in this method or if you want to store them
    // make sure you copy them (using something like strncpy)
    // const char* artist =

    strcpy(songTitle, currentlyPlaying.trackName);
    //Serial.println(currentlyPlaying.trackUri);

    for (int i = 0; i < currentlyPlaying.numArtists; i++)
    {
        strcpy(songArtist,currentlyPlaying.artists[i].artistName);
        //Serial.println(currentlyPlaying.artists[i].artistUri);
    }

    strcpy(songAlbum,currentlyPlaying.albumName);
    //Serial.println(currentlyPlaying.albumUri);

    progress = currentlyPlaying.progressMs; // duration passed in the song
    duration = currentlyPlaying.durationMs; // Length of Song

    percentProgress = ((float)progress / (float)duration) * 100;
}

void currentPlay()
{
  int status = spotify.getCurrentlyPlaying(getCurrentPlaying, SPOTIFY_MARKET);
    if (status == 200)
    {
        Serial.println("Successfully got currently playing");
    }
    else if (status == 204)
    {
        Serial.println("Doesn't seem to be anything playing");
    }
    else
    {
        Serial.print("Error: ");
        Serial.println(status);
    }
  lcd.fillScreen(TFT_BLACK);
  lcd.drawString("Spotify Player",0,0,2);
  lcd.drawString(songTitle,0,DISPLAY_HEIGHT/2 + (9/2));
  delay(1000);
}


void loop()
{
  float time = millis();
  while(time < counter)
  {
    time = millis();
    if(digitalRead(14) == 0)
    {
      Serial.println("Previous Track");
      spotify.previousTrack();
      currentPlay();
      delay(1000);
    }
    if(digitalRead(0) == 0)
    {
      Serial.println("Next Track");
      spotify.nextTrack();
      currentPlay();
      delay(1000);
    }
  }
  currentPlay();
  counter += 10000;
  Serial.println(counter);
  Serial.println(time);
}
