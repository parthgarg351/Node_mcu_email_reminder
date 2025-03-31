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

#define SDA_PIN D2 // GPIO13
#define SCL_PIN D1 // GPIO15
#define LED_PIN D0 // GPIO15

LiquidCrystal_I2C lcd1(0x27, 20, 4);        // Change 0x27 if needed
LiquidCrystal lcd2(D4, D3, D5, D6, D7, D8); // RS, EN, D4, D5, D6, D7

const char *ssid = "PARTH8663";    // Replace with your WiFi SSID
const char *password = "12345678"; // Replace with your WiFi password
// do minor changes in fetchEmail() and fetchMatchData() (see minor changes in function commented out) when server is hosted somewhere
const char *backendServer = "http://192.168.137.1:5000";                 // Replace with your backend URL Donot mention port number when server hosted somewhere
const char *backendServerEmail = "http://192.168.137.1:5000/get-emails"; // Replace with your backend URL
const char *cricketAPI = "http://192.168.137.1:5000/get-match";          // Replace with actual API
const char *newsAPI = "http://192.168.137.1:5000/get-news-title";        // Replace with actual API

unsigned long lastCheckTime = 0;                   // Timestamp for last check
unsigned long lastCheckTime2 = 0;                  // Timestamp for last check
const unsigned long checkInterval = 5 * 60 * 1000; // 5 minutes (in milliseconds) //for email
const unsigned long checkInterval2 = 10 * 1000;    // 10 sec (in milliseconds) // for match

// Variables for quotes display
int count = 0;
int quoteLength;
int currentQuoteIndex;
String quote;
// Quotes array
String quotes[] = {
    "Success is not final; failure is not fatal.",
    "Believe in yourself and all that you are.",
    "Don't watch the clock; do what it does.",
    "Success usually comes to those who are too busy to be looking for it.",
    "The harder you work for something, the greater you'll feel when you achieve it.",
    "Dream big and dare to fail.",
    "Don't stop when you're tired. Stop when you're done.",
    "It always seems impossible until it's done",
    "Success is the sum of small efforts, repeated day in and day out.",
    "You don't have to be great to start, but you have to start to be great.",
    "The only way to achieve the impossible is to believe it is possible.",
    "Your only limit is your mind.",
    "Hard work beats talent when talent doesn't work hard.",
    "Everything you can imagine is real.",
    "Do not wait to strike till the iron is hot, but make it hot by striking.",
    "The future belongs to those who believe in the beauty of their dreams.",
    "Act as if what you do makes a difference. It does.",
    "Don't wait for opportunity. Create it.",
    "Believe you can and you're halfway there.",
    "You are never too old to set another goal or to dream a new dream.",
    "Push yourself, because no one else is going to do it for you.",
    "Great things never come from comfort zones.",
    "Work hard in silence, let your success be your noise.",
    "The key to success is to focus on goals, not obstacles.",
    "The way to get started is to quit talking and begin doing",
    "Success doesn't just find you. You have to go out and get it.",
    "The only limit to our realization of tomorrow is our doubts of today",
    "What you get by achieving your goals is not as important as what you become by achieving your goals.",
    "Dream it. Wish it. Do it.",
    "The harder you work, the luckier you get",
    "Success is walking from failure to failure with no loss of enthusiasm.",
    "If you can dream it, you can do it.",
    "Opportunities don't happen, you create them",
    "Your time is limited, so don t waste it living someone else's life.",
    "Don't be pushed around by the fears in your mind. Be led by the dreams in your heart.",
    "The best way to predict the future is to create it.",
    "Don't stop when you're tired. Stop when you're done.",
    "The difference between who you are and who you want to be is what you do.",
    "Success is the result of preparation, hard work, and learning from failure.",
    "It does not matter how slowly you go as long as you  do not stop.",
    "Success doesn't come from what you do occasionally, it comes from what you do consistently.",
    "You are braver than you believe, stronger than you seem, and smarter than you think.",
    "In the middle of every difficulty lies opportunity.",
    "Success is the result of taking initiative and continuing to take action.",
    "Failure is simply the opportunity to begin again, this time more intelligently.",
    "Don't be afraid to give up the good to go for the great.",
    "The only place where success comes before work is in the dictionary.",
    "Success is not how high you have climbed, but how you make a positive difference to the world.",
    "The best way to predict the future is to invent it.",
    "If you want to live a happy life, tie it to a goal, not to people or things."};

const int totalQuotes = sizeof(quotes) / sizeof(quotes[0]);

WiFiServer server(80);

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", 19800, 60000); // Offset for IST (+5:30)

int i = 0;

void updateTime()
{
  unsigned long currentMillis = millis();
  timeClient.update();

  time_t epochTime = timeClient.getEpochTime();
  struct tm *ptm = gmtime((time_t *)&epochTime);
  int monthDay = ptm->tm_mday;
  Serial.print("Month day: ");
  Serial.println(monthDay);

  int currentMonth = ptm->tm_mon + 1;
  Serial.print("Month: ");
  Serial.println(currentMonth);
  String currentDate = String(monthDay) + "-" + String(currentMonth);
  Serial.print("Current date: ");
  Serial.println(currentDate);

  lcd2.setCursor(0, 0);

  lcd2.print(timeClient.getFormattedTime());
  lcd2.setCursor(8, 0);
  lcd2.print(" ");
  lcd2.setCursor(10, 0);
  lcd2.print(currentDate);
}

void setup()
{
  Serial.begin(115200);
  lcd2.begin(16, 2);
  Wire.begin(SDA_PIN, SCL_PIN); // Initialize I2C on D7 (SDA) and D8 (SCL)

  lcd1.init();      // Initialize the I2C LCD
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
  currentQuoteIndex = random(0, totalQuotes); // Choose a random quote
  quote = quotes[currentQuoteIndex];
  count = 0;
  quoteLength = quote.length();
}

void qoute()
{
  if (count >= 3)
  {
    currentQuoteIndex = random(0, totalQuotes); // Choose a random quote
    quote = quotes[currentQuoteIndex];
    count = 0;
    quoteLength = quote.length();
  }
  // display
  displayFormattedText(quote);
  delay(1000);
  count++;
}

void displayFormattedText(String text)
{
  lcd1.clear();
  String lines[4] = {"", "", "", ""};
  int lineIndex = 0;
  String word = "";

  for (int i = 0; i < text.length(); i++)
  {
    if (text[i] == ' ' || i == text.length() - 1)
    {
      if (i == text.length() - 1)
        word += text[i]; // Add last character if it's the last word
      if (lines[lineIndex].length() + word.length() <= 20)
      {
        if (lines[lineIndex].length() > 0)
          lines[lineIndex] += " ";
        lines[lineIndex] += word;
      }
      else
      {
        lineIndex++;
        if (lineIndex < 4)
        {
          lines[lineIndex] = word;
        }
        else
        {
          break; // Stop if more than 4 lines
        }
      }
      word = "";
    }
    else
    {
      word += text[i];
    }
  }

  for (int i = 0; i < 4; i++)
  {
    lcd1.setCursor(0, i);
    lcd1.print(lines[i]);
  }
}

void scrollText(String text, int row, int ledN)
{
  int len = text.length();
  if (len <= 16)
  {
    if (ledN == 2)
    {
      lcd2.setCursor(0, row);
      lcd2.print(text);
      delay(100);
      return;
    }
  }
  if (len <= 20)
  {
    if (ledN == 1)
    {
      lcd1.setCursor(0, row);
      lcd1.print(text);
      delay(100);
      return;
    }
  }
  if (ledN == 1)
  {
    for (int i = 0; i <= len - 20; i++)
    {
      lcd1.setCursor(0, row);
      lcd1.print(text.substring(i, i + 20));
      delay(400); // Adjust scrolling speed
    }
    return;
  }
  if (ledN == 2)
  {
    for (int i = 0; i <= len - 16; i++)
    {
      lcd2.setCursor(0, row);
      lcd2.print(text.substring(i, i + 16));
      delay(400); // Adjust scrolling speed
    }
  }
}

void fetchEmail()
{
  HTTPClient http;
  // WiFiClient client;
  WiFiClientSecure client;
  client.setInsecure();
  http.setTimeout(60000); // Set timeout to 60 seconds (60000 ms)

  http.begin(client, backendServerEmail);
  int httpCode = http.GET();
  if (httpCode == 200)
  {
    String payload = http.getString();
    if (payload.length() == 0 || payload == "[]")
    {
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
          scrollText(from, 0, 1);
          scrollText(subject, 1, 1);
          delay(2500);
          digitalWrite(LED_PIN, LOW);
          delay(100);
          // lcd1.clear();
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
String extractShortNames(String matchStr)
{
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
      {"Gujarat Titans", "GT"}};

  // Find " vs " in the input string
  int vsIndex = matchStr.indexOf(" vs ");
  if (vsIndex == -1)
    return "Invalid Format";

  // Extract full team names
  String fullTeam1 = matchStr.substring(0, vsIndex);
  String fullTeam2 = matchStr.substring(vsIndex + 4, matchStr.indexOf(","));

  // Match full names with abbreviations
  for (int i = 0; i < 10; i++)
  {
    if (fullTeam1 == teams[i][0])
      team1 = teams[i][1];
    if (fullTeam2 == teams[i][0])
      team2 = teams[i][1];
  }

  if (team1 != "" && team2 != "")
  {
    return team1 + " vs " + team2;
  }

  return "Teams Not Found";
}

void fetchMatchData()
{
  // WiFiClient client;
  WiFiClientSecure client;
  client.setInsecure();
  HTTPClient http;
  http.setTimeout(60000); // Set timeout to 60 seconds (60000 ms)
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
      fetchNews();
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
      scrollText(extractShortNames(doc["title"].as<String>()) + " " + doc["runrate"].as<String>(), 0, 1);
      scrollText(doc["livescore"].as<String>(), 1, 1);
      scrollText(doc["batterone"].as<String>() + "/" + doc["battertwo"].as<String>(), 2, 1);
      scrollText(doc["update"].as<String>(), 3, 1);
    }
  }
  else
  {
    Serial.println("❌ Failed to fetch match data");
  }

  http.end();
}

void displayNews(String news)
{
  lcd1.clear();
  lcd1.setCursor(0, 0);
  lcd1.print("News Title:");
  delay(400);
  lcd1.clear();

  String words[50];
  int wordCount = 0;

  String temp = "";
  for (int i = 0; i < news.length(); i++)
  {
    if (news[i] == ' ')
    {
      words[wordCount++] = temp;
      temp = "";
    }
    else
    {
      temp += news[i];
    }
  }
  words[wordCount++] = temp;

  int line = 0, pos = 0, right = 0;
  String currentLine = "";

  for (int i = 0; i < wordCount; i++)
  {
    if (pos + words[i].length() <= 20)
    {
      currentLine += words[i] + " ";
      pos += words[i].length() + 1;
    }
    else
    {
      lcd1.setCursor(0, line);
      lcd1.print(currentLine);
      line++;
      if (line > 3)
        break; // Stop filling if the fourth line is reached
      currentLine = words[i] + " ";
      pos = words[i].length() + 1;
    }
    right = i;
  }

  if (line <= 3)
  {
    lcd1.setCursor(0, line);
    lcd1.print(currentLine);
  }
  else
  {
    String remainingText = currentLine;
    for (int i = right; i < wordCount; i++)
    {
      remainingText += words[i] + " ";
    }
    scrollText(remainingText, 3, 1);
  }
}

void fetchNews()
{
  // WiFiClient client;
  WiFiClientSecure client;
  client.setInsecure();
  HTTPClient http;
  http.setTimeout(60000); // Set timeout to 60 seconds (60000 ms)
  http.begin(client, newsAPI);
  int httpCode = http.GET();
  if (httpCode == HTTP_CODE_OK)
  {
    String payload = http.getString();
    Serial.println(payload);

    StaticJsonDocument<200> doc;
    deserializeJson(doc, payload);
    String newsTitle = doc["title"].as<String>();

    displayNews(newsTitle); // Call function to display formatted text
  }
  else
  {
    Serial.println("Failed to fetch news");
    qoute();
  }
  http.end();
}

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
