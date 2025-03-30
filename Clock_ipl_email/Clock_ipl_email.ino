#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <LiquidCrystal.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <time.h>
#include <ArduinoJson.h>
#include <WiFiClientSecure.h>

#define SDA_PIN D2  // GPIO13
#define SCL_PIN D1  // GPIO15
#define LED_PIN D0  // GPIO15

LiquidCrystal_I2C lcd1(0x27, 20, 4);  // Change 0x27 if needed
LiquidCrystal lcd2(D4, D3, D5, D6, D7, D8);  // RS, EN, D4, D5, D6, D7

const char* ssid = "PARTH8663";      // Replace with your WiFi SSID
const char* password = "12345678";   // Replace with your WiFi password
//do minor changes in fetchEmail() and fetchMatchData() (see minor changes in function commented out) when server is hosted somewhere 
const char* backendServer = "http://192.168.137.1:5000";  // Replace with your backend URL Donot mention port number when server hosted somewhere
const char* backendServerEmail = "http://192.168.137.1:5000/get-emails";  // Replace with your backend URL
const char* cricketAPI = "http://192.168.137.1:5000/get-match"; // Replace with actual API

unsigned long lastCheckTime = 0; // Timestamp for last check
unsigned long lastCheckTime2 = 0; // Timestamp for last check
const unsigned long checkInterval =1.5 * 60 * 1000; // 1.5 minutes (in milliseconds) //for email
const unsigned long checkInterval2 = 10 * 1000; // 10 sec (in milliseconds) // for match

WiFiServer server(80);

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", 19800, 60000); // Offset for IST (+5:30)

int i = 0;

  void updateTime(){
    unsigned long currentMillis = millis();
    timeClient.update(); 
  
    time_t epochTime = timeClient.getEpochTime();
    struct tm *ptm = gmtime ((time_t *)&epochTime); 
    int monthDay = ptm->tm_mday;
    Serial.print("Month day: ");
    Serial.println(monthDay);

    int currentMonth = ptm->tm_mon+1;
    Serial.print("Month: ");
    Serial.println(currentMonth);
    String currentDate =  String(monthDay) + "-" + String(currentMonth);
    Serial.print("Current date: ");
    Serial.println(currentDate);

    lcd2.setCursor(0, 0);

    lcd2.print(timeClient.getFormattedTime());
    lcd2.setCursor(8, 0);
    lcd2.print(" ");
    lcd2.setCursor(10, 0);
    lcd2.print(currentDate);
  }

void setup() {
    Serial.begin(115200);
    lcd2.begin(16, 2);
    Wire.begin(SDA_PIN, SCL_PIN);  // Initialize I2C on D7 (SDA) and D8 (SCL)
    
    lcd1.init();     // Initialize the I2C LCD
    lcd1.backlight(); // Turn on the I2C LCD backlight

    // Display a test message
    lcd1.setCursor(0, 0);
    lcd2.setCursor(0, 0);
    lcd1.print("Hello, Mr. Parth Garg");
    lcd2.print("Welcome Sir");
    lcd1.setCursor(0, 1);
    lcd2.setCursor(0, 1);
    // lcd1.print("20x4 I2C LCD");
    // lcd2.print("16x2 Parallel LCD");
    lcd1.print("Starting...");
    lcd2.print("Starting...");
    delay(2000);
    lcd1.print("Connecting...");
    lcd2.print("Connecting...");
    lcd1.clear();
    lcd2.clear();

    WiFi.begin(ssid, password);
    
    pinMode(LED_PIN, OUTPUT);
    digitalWrite(LED_PIN, LOW);

    Serial.print("Connecting to WiFi");
    while (WiFi.status() != WL_CONNECTED) 
    {
      delay(1000);
      lcd1.clear();
      lcd2.clear();
      lcd1.print("Connecting...");
      lcd2.print("Connecting...");
      lcd1.setCursor(0, 1);
      lcd2.setCursor(0, 1);
      lcd1.print(WiFi.SSID());
      lcd2.print(WiFi.SSID());
    }

    Serial.println("\n✅ Connected to WiFi: " + WiFi.localIP().toString());

    // Initialize NTP client
    timeClient.begin();
    timeClient.forceUpdate();
    lcd1.clear();
    lcd2.clear();
    delay(500);
    lcd1.setCursor(0, 0);
    lcd2.setCursor(0, 0);
    lcd1.print("Connected");
    lcd2.print("Connected");
    lcd1.setCursor(0, 1);
    lcd2.setCursor(0, 1);
    lcd1.print("Successfully.");
    lcd2.print("Successfully.");
    delay(1000);
    lcd1.clear();
    lcd2.clear();
}

void scrollText(String text, int row,int ledN) {
    int len = text.length();
    if (len <= 16) {
        if(ledN==2)
        {
          lcd2.setCursor(0, row);
          lcd2.print(text);
          delay(100);
          return;
        }
    }
    if (len <= 20) 
    {
      if(ledN==1)
      {
        lcd1.setCursor(0, row);
        lcd1.print(text);
        delay(100);
        return;
      }
    }
    if(ledN==1)
    {
      for (int i = 0; i <= len - 20; i++) {
        lcd1.setCursor(0, row);
        lcd1.print(text.substring(i, i + 20));
        delay(400);  // Adjust scrolling speed
      }
      return;
    }
    if(ledN==2)
    {
      for (int i = 0; i <= len - 16; i++) {
        lcd2.setCursor(0, row);
        lcd2.print(text.substring(i, i + 16));
        delay(400);  // Adjust scrolling speed
      }
    }
}

void fetchEmail() {
    HTTPClient http;
    // WiFiClient client;   //use for local server testing 
    WiFiClientSecure client; //use when server is hosted
    client.setInsecure();
    http.setTimeout(60000);  // Set timeout to 60 seconds (60000 ms)
    http.begin(client,backendServerEmail);
    int httpCode = http.GET();
    if (httpCode == 200) 
    { 
      String payload = http.getString();
            if (payload.length() == 0 || payload == "[]") {
                Serial.println("📭 No new emails. Skipping processing...");
                http.end();
                return;
            }
      Serial.println("📩 Raw Payload:\n" + payload);

      DynamicJsonDocument doc(1024); // Adjust buffer size based on expected data
      DeserializationError error = deserializeJson(doc, payload);
            
      if (error) 
      {
        Serial.println("❌ JSON Parsing Failed!");
      } 
      else 
      {
        Serial.println("\n📨 New Emails:");
        for (JsonObject email : doc.as<JsonArray>()) 
        {
          String from = email["from"].as<String>();
          String subject = email["subject"].as<String>();
                    
          Serial.println("-------------------------------");
          Serial.println("📧 From: " + from);
          Serial.println("📜 Subject: " + subject);
          for (int i = 0; i < 6; i++) 
          {
            digitalWrite(LED_PIN, HIGH);
            delay(500);
            lcd1.clear();
            scrollText(from,0,1);
            scrollText(subject,1,1);
            delay(2500);
            digitalWrite(LED_PIN, LOW);
            delay(100);
            lcd1.clear();
          }
        }
        Serial.println("-------------------------------\n");
      }
    } 
    else 
    {
      Serial.println("❌ Failed to fetch emails");
    }

    http.end();
}

// Function to extract short names
String extractShortNames(String matchStr) {
    String team1 = "", team2 = "";
    
    // Define short names for IPL teams
    String teams[][2] = {
        {"Mumbai Indians", "MI"},
        {"Kolkata Knight Riders", "KKR"},
        {"Chennai Super Kings", "CSK"},
        {"Delhi Capitals", "DC"},
        {"Royal Challengers Bangalore", "RCB"},
        {"Rajasthan Royals", "RR"},
        {"Sunrisers Hyderabad", "SRH"},
        {"Punjab Kings", "PBKS"},
        {"Lucknow Super Giants", "LSG"},
        {"Gujarat Titans", "GT"}
    };

    // Find " vs " in the input string
    int vsIndex = matchStr.indexOf(" vs ");
    if (vsIndex == -1) return "Invalid Format";

    // Extract full team names
    String fullTeam1 = matchStr.substring(0, vsIndex);
    String fullTeam2 = matchStr.substring(vsIndex + 4, matchStr.indexOf(","));

    // Match full names with abbreviations
    for (int i = 0; i < 10; i++) {
        if (fullTeam1 == teams[i][0]) team1 = teams[i][1];
        if (fullTeam2 == teams[i][0]) team2 = teams[i][1];
    }

    if (team1 != "" && team2 != "") {
        return team1 + " vs " + team2;
    }
    
    return "Teams Not Found";
}

void fetchMatchData() 
{
  HTTPClient http;
  // WiFiClient client;   //use for local server testing 
  WiFiClientSecure client; //use when server is hosted
  client.setInsecure();
  http.setTimeout(60000);  // Set timeout to 60 seconds (60000 ms)
  Serial.println("Fetching IPL Data...");
  http.begin(client, cricketAPI); 
  int httpCode = http.GET();
  if (httpCode == 200) 
  {
    String payload = http.getString();

    if (payload.indexOf("Match is not live") >= 0) 
    {
      Serial.println("📢 Match is NOT LIVE");
      http.end();
      return;
    }

    Serial.println("📩 Match Data Received:\n" + payload);

    DynamicJsonDocument doc(1024);
    DeserializationError error = deserializeJson(doc, payload);

    if (error) 
    {
      Serial.println("❌ JSON Parsing Failed!");
    } 
    else 
    {
      Serial.println("\n🏏 Match Details:");
      Serial.println("-------------------------------");
      Serial.println("📢 Title: " + doc["title"].as<String>());
      Serial.println("📜 Update: " + doc["update"].as<String>());
      Serial.println("🎯 Live Score: " + doc["livescore"].as<String>());
      Serial.println("🔥 Run Rate: " + doc["runrate"].as<String>());
      Serial.println("🏏 Batter 1: " + doc["batterone"].as<String>());
      Serial.println("🏏 Batter 2: " + doc["battertwo"].as<String>());
      Serial.println("-------------------------------\n");
      lcd1.clear();
      scrollText(extractShortNames(doc["title"].as<String>())+" "+doc["runrate"].as<String>(),0,1);
      scrollText(doc["livescore"].as<String>(),1,1);
      scrollText(doc["batterone"].as<String>()+"/"+doc["battertwo"].as<String>(),2,1);
      scrollText(doc["update"].as<String>(),3,1);
    }
  } 
  else 
  {
    Serial.println("❌ Failed to fetch match data");
  }

  http.end();

}


// void updateLCD() {
//     String iplData = fetchIPLData();
//     DynamicJsonDocument doc(2048);
//     deserializeJson(doc, iplData);
//     JsonArray data = doc["data"];
//     bool matchStarted = data[0]["matchStarted"];
//     bool matchEnded = data[0]["matchEnded"];
    
//     String news = getNews();
    
//     lcd1.clear();
//     if(matchStarted==true&&matchEnded==false)
//     {
//       displayIPLScore(iplData);
//     }
//     else if(news.length() > 0){
//       // Serial.println(news,"news willl be shown");
//       Serial.println("news willl be shown");
//     }
//     else
//     {
//       Serial.println("qoute willl be shown");
//     }
// }

void loop() 
{
  updateTime();
  unsigned long currentMillis = millis();
  if (currentMillis - lastCheckTime >= checkInterval) 
  {
    Serial.println("⏳ Checking for new emails...");
    fetchEmail();
    lastCheckTime = currentMillis; 
  }
  if (currentMillis - lastCheckTime2 >= checkInterval2) 
  {
    Serial.println("⏳ Checking for match update...");
    fetchMatchData();
    lastCheckTime2 = currentMillis; 
  }
  delay(1000); // Avoids CPU overuse
}
