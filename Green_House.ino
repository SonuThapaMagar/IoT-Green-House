#include <WiFi.h>
#include <DHT.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// Replace with your network credentials
const char* ssid = "Virinchi College";
const char* password = "virinchi@2024";

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
  digitalWrite(RELAYPIN, LOW); // Turn relay off initially
  digitalWrite(FAN1PIN, LOW);  // Turn fan 1 off initially
  digitalWrite(FAN2PIN, LOW);  // Turn fan 2 off initially

  // Initialize LCD with the specified I2C pins
  Wire.begin(21, 22);   // SDA = GPIO 21, SCL = GPIO 22
  lcd.begin(20, 4);     // Specify the number of columns and rows
  lcd.backlight();      // Turn on the backlight

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
  WiFiClient client = server.available(); // Check for incoming clients

  if (client) {
    String currentLine = "";

    // Read the client's request
    while (client.connected()) {
      if (client.available()) {
        char c = client.read();
        if (c == '\n') {
          // If the current line is blank, the HTTP request has ended
          if (currentLine.length() == 0) {
            // Send the response
            String response = "<!DOCTYPE html><html>";
            response += "<head><title>Greenhouse Monitor</title></head>";
            response += "<body><h1>Greenhouse Monitor</h1>";

            // Read DHT sensor values
            float h = dht.readHumidity();
            float t = dht.readTemperature();
            int airQuality = analogRead(MQ135PIN);
            int soilMoisture = analogRead(SOILMOISTUREPIN);

            // Calculate CO2 concentration
            float CO2_ppm = getCO2Concentration(airQuality);
            float CO2_percentage = convertPPMtoPercentage(CO2_ppm);

            // Check if any reads failed and exit early (to try again).
            if (isnan(h) || isnan(t)) {
              response += "<p>Failed to read from DHT sensor!</p>";
            } else {
              float t = dht.readTemperature();
              response += "<p>Humidity: " + String(h) + " %</p>";
              response += "<p>Temperature: " + String(t) + " °C</p>";
              response += "<p>Air Quality (MQ135): " + String(airQuality) + " (raw value)</p>";
              if (isnan(CO2_ppm)) {
                response += "<p>CO2: Failed to read CO2 concentration!</p>";
              } else {
                response += "<p>CO2 Concentration: " + String(CO2_ppm) + " ppm</p>";
                response += "<p>CO2 Percentage: " + String((CO2_percentage * 100.0) / 2) + " %</p>"; // Display CO2 percentage
              }

              response += "<p>Soil Moisture: " + String(soilMoisture) + "</p>";
            
              // Control the fans based on temperature
              if (t > 30) {
                digitalWrite(FAN1PIN, HIGH); // Turn on fan 1
                digitalWrite(FAN2PIN, HIGH);// Turn on fan 2
                
                response += "<p>Fans are ON.</p>";
              } else if (t < 28) {
                digitalWrite(FAN1PIN, LOW);  // Turn off fan 1
                digitalWrite(FAN2PIN, LOW);
                 // Turn off fan 2
                response += "<p>Fans are OFF.</p>";
              } else {
                response += "<p>Fans are in standby mode.</p>";
              }

              // If soil is dry, activate relay
              if (soilMoisture > 3000) {  // Adjust the threshold as needed
                digitalWrite(RELAYPIN, HIGH); // Turn relay on
                response += "<p>Soil is dry! Relay activated.</p>";
              } else if(soilMoisture < 2600) {
                digitalWrite(RELAYPIN, LOW);
                 // Turn relay off
                response += "<p>Soil is moist. Relay deactivated.</p>";
              }
              else{
                response += "<p>relay is in standby mode.</p>";
              }
            }

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

  // Update LCD with temperature, humidity, CO2 concentration, and soil moisture
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Temp: ");
  lcd.print(dht.readTemperature());
  lcd.print(" C");

  lcd.setCursor(0, 1);
  lcd.print("Humidity: ");
  lcd.print(dht.readHumidity());
  lcd.print(" %");

  lcd.setCursor(0, 2);
  lcd.print("CO2: ");
  int airQuality = analogRead(MQ135PIN);
  float CO2_ppm = getCO2Concentration(airQuality);
  lcd.print(CO2_ppm/1000);
  lcd.print(" %");

  lcd.setCursor(0, 3);
  lcd.print("Soil Moist: ");
  int soilMoisture = analogRead(SOILMOISTUREPIN);
  if(soilMoisture>3000)
  {
   lcd.print("Dry"); 
  }
  else if (soilMoisture<3000) 
  {

  lcd.print("Moist");
  }
  else{
    lcd.print("sensor connect gar");
  }
  

  delay(2000); // Update display every 2 seconds
}
