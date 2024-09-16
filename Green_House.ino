#include <WiFi.h>
#include <DHT.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <ESP32Servo.h>  // Include the ESP32Servo library

// Replace with your network credentials
const char* ssid = "Virinchi College";
const char* password = "virinchi@2024";

Servo esc;

// DHT Sensor setup
#define DHTPIN 4          // DHT sensor pin
#define DHTTYPE DHT22     // DHT 22
DHT dht(DHTPIN, DHTTYPE);

// MQ135 Sensor setup
#define MQ135PIN 34       // MQ135 analog pin

// Soil Moisture Sensor setup
#define SOILMOISTUREPIN 35 // Soil moisture sensor pin
#define RELAYPIN 5         // Relay control pin

// Fan setup
#define FAN1PIN 12         // Fan 1 control pin
#define FAN2PIN 13         // Fan 2 control pin

// Bulb setup
#define BULBPIN 14         // Bulb control pin
bool bulbState = LOW;      // Initial state of the bulb (off)

// Constants for MQ135 CO2 Calculation
float R0 = 76.63;        // Replace with your calibrated R0 value
float RL = 10.0;         // Load resistance (in kOhms)
float a = 116.6020682;   // Derived from the MQ135 datasheet or calibration
float b = -2.769034857;  // Derived from the MQ135 datasheet or calibration

// Initialize WiFi
WiFiServer server(80);

// Initialize the LCD display (address 0x27, 20x4)
LiquidCrystal_I2C lcd(0x27, 20, 4);

float getCO2Concentration(int sensorValue) {
  float V_sensor = sensorValue * (3.3 / 4095.0);  // Convert analog reading to voltage for ESP32
  float Rs = RL * (3.3 - V_sensor) / V_sensor;    // Calculate Rs
  float ratio = Rs / R0;                          // Rs/R0 ratio
  float CO2_ppm = a * pow(ratio, b);              // Calculate CO2 concentration in ppm

  return CO2_ppm;
}

float convertPPMtoPercentage(float ppm) {
  return ppm / 10000.0;  // Convert ppm to percentage
}
float getAirQualityPercentage(int sensorValue) {
  int maxValue = 4095; // Maximum value for ESP32 analog input
  return (sensorValue / (float)maxValue) * 100.0; // Convert raw value to percentage
}

void setup() {
  
  Serial.begin(115200);

  // Set the attenuation for analog input (for ESP32)
  analogSetAttenuation(ADC_11db); // Set input range to 0-3.3V

  // Start DHT sensor
  dht.begin();

  // Set relay pin and fan pins as output
  pinMode(RELAYPIN, OUTPUT);
  pinMode(FAN1PIN, OUTPUT);
  pinMode(FAN2PIN, OUTPUT);
  pinMode(BULBPIN, OUTPUT);
  digitalWrite(RELAYPIN, LOW); // Turn relay off initially
  digitalWrite(FAN1PIN, LOW);  // Turn fan 1 off initially
  digitalWrite(FAN2PIN, LOW);  // Turn fan 2 off initially
  digitalWrite(BULBPIN, LOW);  // Turn bulb off initially

  // Initialize LCD with the specified I2C pins
  Wire.begin(21, 22);   // SDA = GPIO 21, SCL = GPIO 22
  lcd.begin(20, 4);     // Specify the number of columns and rows
  lcd.backlight();      // Turn on the backlight


   // Attach the ESC to the motor pin with min and max pulse widths
  // esc.attach(RELAYPIN, 1000, 2000);

 

  // Send the minimum throttle signal to initialize the ESC
  // esc.write(0);  // Corresponds to 1000 microseconds (minimum throttle)
  Serial.println("ESC initialization: sending minimum throttle");
  delay(1000);  // Wait for 1 second

  // Connect to Wi-Fi
  WiFi.begin(ssid, password);

  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }

  // Start the server
  server.begin();
  Serial.println("Server started");

  // Print the local IP address
  Serial.print("Local IP: ");
  Serial.println(WiFi.localIP());
}

void loop() {
   float t = dht.readTemperature();
   int soilMoisture = analogRead(SOILMOISTUREPIN);

   // Control the fans based on temperature
              if (t > 32) {
                digitalWrite(FAN1PIN, HIGH); // Turn on fan 1
                digitalWrite(FAN2PIN, HIGH); // Turn on fan 2
               
              } else {
                digitalWrite(FAN1PIN, LOW);  // Turn off fan 1
                digitalWrite(FAN2PIN, LOW);  // Turn off fan 2
              
              }

               if (t < 30 ) {
                digitalWrite(BULBPIN, HIGH); // Turn on fan 1
                 // Turn on fan 2
               
              } else {
                digitalWrite(BULBPIN, LOW);  // Turn off fan 1
                 // Turn off fan 2
              
              }
              if (soilMoisture > 2900) {
       digitalWrite(RELAYPIN, HIGH); // Corresponds to 2000 microseconds (maximum throttle)
    
  } else {
      digitalWrite(RELAYPIN, LOW);  // Corresponds to 2000 microseconds (maximum throttle)
   
  } 
  WiFiClient client = server.available(); // Check for incoming clients


  if (client) {
    String currentLine = "";

    // Read the client's request
    while (client.connected()) {
      if (client.available()) {
        char c = client.read();
        if (c == '\n') {
          // If the current line is blank, the HTTP request has ended
           // Read DHT sensor values
            float h = dht.readHumidity();
            float t = dht.readTemperature();
            int airQuality = analogRead(MQ135PIN);
            int soilMoisture = analogRead(SOILMOISTUREPIN);
            // Calculate CO2 concentration
            float CO2_ppm = getCO2Concentration(airQuality);
            float CO2_percentage = convertPPMtoPercentage(CO2_ppm);
          if (currentLine.length() == 0) {
            // Send the response
            String response = "<!DOCTYPE html><html lang=\"en\">";
            response += "<head>";
            response += "<meta charset=\"UTF-8\">";
            response += "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">";
            response += "<meta http-equiv=\"refresh\" content=\"3\">"; // Auto-reload every 3 seconds
            response += "<title>Greenhouse Monitor</title>";
            response += "<style>";
            response += "body { font-family: Arial, sans-serif; margin: 0; padding: 0; background-color: #f4f4f4; }";
            response += "h1 { text-align: center; color: #4CAF50; }";
            response += ".container { max-width: 600px; margin: 20px auto; padding: 20px; background-color: #fff; box-shadow: 0 0 10px rgba(0, 0, 0, 0.1); }";
            response += ".sensor-data { display: flex; justify-content: space-between; padding: 10px; border-bottom: 1px solid #ddd; }";
            response += ".sensor-data:last-child { border-bottom: none; }";
            response += ".sensor-data h2 { margin: 0; color: #333; font-size: 18px; }";
            response += ".sensor-data p { margin: 0; color: #555; font-size: 16px; }";
            response += ".status { font-weight: bold; color: #FF5722; }";
            response += "</style>";
            response += "</head>";
            response += "<body>";
            response += "<h1>Greenhouse Monitor</h1>";
            response += "<div class=\"container\">";

           

            

            // Check if any reads failed and exit early (to try again).
            if (isnan(h) || isnan(t)) {
              response += "<div class=\"sensor-data\"><h2>Sensor Error:</h2><p>Failed to read from DHT sensor!</p></div>";
            } else {
              response += "<div class=\"sensor-data\"><h2>Humidity:</h2><p>" + String(h) + " %</p></div>";
              response += "<div class=\"sensor-data\"><h2>Temperature:</h2><p>" + String(t) + " °C</p></div>";
              response += "<div class=\"sensor-data\"><h2>Air Quality (MQ135):</h2><p>" + String(airQuality) + " (raw value)</p></div>";
              if (isnan(CO2_ppm)) {
                response += "<div class=\"sensor-data\"><h2>CO2:</h2><p>Failed to read CO2 concentration!</p></div>";
              } else {
                response += "<div class=\"sensor-data\"><h2>CO2 Concentration:</h2><p>" + String(CO2_ppm) + " ppm</p></div>";
                response += "<div class=\"sensor-data\"><h2>CO2 Percentage:</h2><p>" + String((CO2_percentage * 100.0) / 2) + " %</p></div>";
              }

              response += "<div class=\"sensor-data\"><h2>Soil Moisture:</h2><p>" + String(soilMoisture) + "</p></div>";

              

              // Control the fans based on temperature
              if (t > 31) {
                // digitalWrite(FAN1PIN, HIGH); // Turn on fan 1
                // digitalWrite(FAN2PIN, HIGH); // Turn on fan 2
                response += "<div class=\"sensor-data\"><h2>Fans:</h2><p class=\"status\">ON</p></div>";
              } else {
                // digitalWrite(FAN1PIN, LOW);  // Turn off fan 1
                // digitalWrite(FAN2PIN, LOW);  // Turn off fan 2
                response += "<div class=\"sensor-data\"><h2>Fans:</h2><p class=\"status\">OFF</p></div>";
              }

              
            }

           

            response += "</div>";
            response += "</body></html>";
            client.println("HTTP/1.1 200 OK");
            client.println("Content-Type: text/html");
            client.println("Connection: close");
            client.println();
            client.println(response);
            break;
          } else {
            currentLine = "";
          }
        } else if (c != '\r') {
          currentLine += c; // Add to the current line
        }
      }
    }

    // Close the connection
    client.stop();
    Serial.println("Client disconnected");
  }
// Control the fans based on temperature
              if (t > 32) {
                digitalWrite(FAN1PIN, HIGH); // Turn on fan 1
                digitalWrite(FAN2PIN, HIGH); // Turn on fan 2
               
              } else {
                digitalWrite(FAN1PIN, LOW);  // Turn off fan 1
                digitalWrite(FAN2PIN, LOW);  // Turn off fan 2
              
              }

               if (t < 30 ) {
                digitalWrite(BULBPIN, HIGH); // Turn on fan 1
                 // Turn on fan 2
               
              } else {
                digitalWrite(BULBPIN, LOW);  // Turn off fan 1
                 // Turn off fan 2
              
              }
  //             if (soilMoisture > 3100) {
  //    esc.write(100);  // Corresponds to 2000 microseconds (maximum throttle)
    
  // } else {
  //   esc.write(0);  // Corresponds to 2000 microseconds (maximum throttle)
   
  // } 
  // Update LCD with temperature, humidity, CO2 concentration, and soil moisture
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Temp: ");
  lcd.print(dht.readTemperature());
  lcd.print(" C");
 
  lcd.setCursor(0, 1);
  lcd.print("humidity: ");
  lcd.print(dht.readHumidity());
  lcd.print(" %");
  float R = dht.readTemperature();

   lcd.setCursor(0, 2);
  lcd.print("Air Qual: ");
  lcd.print(getAirQualityPercentage(analogRead(MQ135PIN)));
  lcd.print("%");
  

  lcd.setCursor(0, 3);
  lcd.print("Soil Moist: ");
  int soil = analogRead(SOILMOISTUREPIN);
   if (soilMoisture > 2900) {
      // Corresponds to 2000 microseconds (maximum throttle)
    lcd.print("Dry");
  } else {
     // Corresponds to 2000 microseconds (maximum throttle)
    lcd.print("Moist");
  }
    Serial.println(soil);

  delay(2000); // Update display every 2 seconds
}

