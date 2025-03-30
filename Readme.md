# NodeMCU Live Score & Email Notification System
   - This project is an ESP8266-based notification system that displays emails and live cricket scores on two LCD screens. The system fetches data from a backend server via WiFi and updates the displays accordingly.

## Features
- **WiFi Connectivity**: Connects to a backend server to fetch emails and cricket scores.
- **Dual LCD Support**: Displays data on a 16x2 LCD (parallel) and a 20x4 LCD (I2C).
- **Live Cricket Updates**: Retrieves live match scores.
- **Email Alerts**: Notifies about new emails via LCD and LED blinks.

## Components Used
- **ESP8266 (NodeMCU)**
- **16x2 LCD (Parallel)**
- **20x4 LCD (I2C)**
- **LED Indicator**
- **Resistors & Jumper Wires**
- **5V Power Supply**

## Circuit Diagram

| Component      | ESP8266 Pin |
|----------------|-------------|
| 16x2 LCD (RS)  | D4          |
| 16x2 LCD (EN)  | D3          |
| 16x2 LCD (D4)  | D5          |
| 16x2 LCD (D5)  | D6          |
| 16x2 LCD (D6)  | D7          |
| 16x2 LCD (D7)  | D8          |
| 20x4 LCD (SDA) | D2          |
| 20x4 LCD (SCL) | D1          |
| LED Indicator  | D0          |

## Setup Instructions
1. Clone this repository:
   ```sh
   git clone https://github.com/parthgarg351/Node_mcu_email_reminder
   ```
2. Install required dependencies:
   ```sh
   npm install
   ```   
3. **Create a `.env` file** and store your email credentials and cricketAPI :
   ```env
   EMAIL_USER=<your_email>
   EMAIL_PASS=<your_app_password>
   CRICKET_API_URL=<api_url>
   ```
4. **Generate App Password** (for Gmail users):
   - Go to your **Google Account**.
   - Enable **2-Step Verification**.
   - Search for **App Passwords** and generate one.
5. **Create a server and host for cricket API**
   - Most cricket APIs are either paid or have limited requests, so it's recommended to host your own API.
   - Refer to this Github Repo https://github.com/sanwebinfo/cricket-api 
   - A big thanks to the owner of this repository 
6. Start the server:
   ```sh
   node server.js
   ```
7. Connect the components as per the circuit diagram.
8. Install the following Arduino libraries:
   - Wire  
   - LiquidCrystal_I2C  
   - LiquidCrystal  
   - ESP8266WiFi  
   - ESP8266HTTPClient  
   - NTPClient  
   - ArduinoJson  
   - WiFiClientSecure  
9. Upload the `ESP8266` sketch to the board.
10. Modify the `ssid` and `password` variables to connect to your WiFi.

## How to Use

1. **Find Match ID**
   - Go to [Cricbuzz Live Scores](https://www.cricbuzz.com/live-cricket-scores/)
   - Select your match and copy the **Match ID** from the URL.
   - Example: `https://www.cricbuzz.com/live-cricket-scores/115030`

2. **Set Match on Server**
   - Visit your **server URL**.
   - Enter the **Match ID** and press **Go**.


## Contributing
Feel free to fork this repository, create a new branch, and submit a pull request!

## License
This project is licensed under the MIT License.

**I am trying to add more features it like showing news and more.**
## Author
**Parth Garg**
**parthgarg351@gmail.com**
