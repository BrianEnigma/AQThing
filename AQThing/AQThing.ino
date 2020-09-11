/*
 *  Simple HTTP get webclient test
 */

#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <ESP8266WiFi.h>
#include <SPI.h>
#include <WiFiClient.h>
#include <Wire.h>

#include <Fonts/FreeSansBold18pt7b.h>

#include "settings.h"

const char *host = "api.airvisual.com";
const GFXfont *fontNumerals= &FreeSansBold18pt7b;

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 32 // OLED display height, in pixels
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire);

//#define FAKE_DATA
//#define DEBUG_JSON

void setup() 
{
    Serial.begin(115200);
    delay(100);
  
    // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
    if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Address 0x3C for 128x32
        Serial.println("SSD1306 allocation failed");
        for(;;); // Don't proceed, loop forever
    }
    display.clearDisplay();
    display.display();
    display.setTextSize(1);               // Normal 1:1 pixel scale
    display.setTextColor(SSD1306_WHITE);  // Draw white text
    display.setCursor(0, 0);              // Start at top-left corner
    display.cp437(true);                  // Use full 256 char 'Code Page 437' font

#ifndef FAKE_DATA
    // We start by connecting to a WiFi network
    Serial.println();
    Serial.println();
    Serial.print("Connecting to ");
    Serial.println(SSID);
    display.println("Connecting to wifi");
    display.println(SSID);
    display.display();
    
    WiFi.begin(SSID, PASSWORD);
  
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }

    Serial.println("");
    Serial.println("WiFi connected");  
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
    display.clearDisplay();
    display.setCursor(0, 0);
    display.println(WiFi.localIP());
    display.display();
    delay(1000);
#endif
}

void fetchData(String &result)
{
    result = "";
  
    // Secure connect
    Serial.print("connecting to server");
    Serial.println(host);
    display.clearDisplay();
    display.setCursor(0, 0);
    display.println("(...refreshing...)");
    display.println("Connecting to");
    display.println(host);
    display.display();
    
    // Use WiFiClient class to create TCP connections
    WiFiClient client;
    CertStore certStore;
    const int httpPort = 80;
    if (!client.connect(host, httpPort)) 
    {
        Serial.println("connection failed");
        delay(30000);
        return;
    }
    
    // We now create a URI for the request
    String url = "/v2/city?city=";
    url += CITY;
    url += "&state=";
    url += STATE;
    url += "&country=";
    url += COUNTRY;
    url += "&key=";
    url += API_KEY;
    Serial.print("Requesting URL");
    display.clearDisplay();
    display.setCursor(0, 0);
    display.println("(...refreshing...)");
    display.println("Loading AQI");
    display.display();
    
    // This will send the request to the server
    client.print(String("GET ") + url + " HTTP/1.1\r\n" +
                 "Host: " + host + "\r\n" + 
                 "Connection: close\r\n\r\n");
    delay(500);
    
    // Read all the lines of the reply from server and print them to Serial
    while(client.available())
    {
        String line = client.readStringUntil('\r');
        Serial.print(line);
        result += line;
    }
    
    Serial.println();
    Serial.println("closing connection");
}

// JSON parser that's not really a JSON parser. We want to find `.data.current.pollution.aqius`, so wil
// just do some dead-simple string searching to find it.
int parseAqi(const String &s)
{
    int result = -1;
#ifdef DEBUG_JSON    
    Serial.println(s);
#endif
    int pos = s.indexOf("current");
#ifdef DEBUG_JSON    
    Serial.println(pos);
#endif
    if (pos >= 0)
        pos = s.indexOf("pollution", pos);
#ifdef DEBUG_JSON    
    Serial.println(pos);
#endif
    if (pos >= 0)
        pos = s.indexOf("aqius", pos);
#ifdef DEBUG_JSON    
    Serial.println(pos);
#endif
    if (pos >= 0)
        pos = s.indexOf(":", pos);
#ifdef DEBUG_JSON    
    Serial.println(pos);
#endif
    if (pos >= 0)
    {
        pos += 1;
        char c = s.charAt(pos);
        result = 0;
        while (pos < s.length() && (c == ' ' || (c >= '0' && c <= '9')))
        {
            if (c >= '0' && c <= '9')
                result = result * 10 + c - '0';
            pos++;
            if (pos < s.length())
                c = s.charAt(pos);
        }
    }
    Serial.print("AQI: ");
    Serial.println(result);
    return result;
}

void displayAqi(int value)
{
    int x = 0;
    int y = 0;
    display.clearDisplay();
    display.setFont(NULL);
    display.setTextSize(1);
    
    display.setCursor(0, 5);
    display.print("Air\nQuality\nIndex");
    
    display.setFont(fontNumerals);
    display.setTextSize(1);
    display.setCursor(60, 28);
    if (value > 0)
        display.print(value);
    else
        display.print("???");
    display.display();
}

void loop() 
{
    String json;
    int aqi;
#ifndef FAKE_DATA
    fetchData(json);
    aqi = parseAqi(json);
#else
    aqi = 999;
#endif    
    displayAqi(aqi);
    // Delay an hour before next update.
    for (int minute = 0; minute < 60; minute++)
        delay(60000); // one minute
}
