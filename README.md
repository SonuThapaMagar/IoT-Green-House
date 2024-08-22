# Greenhouse IoT Project

This project is an IoT-based greenhouse monitoring system that tracks and controls key environmental parameters such as soil moisture, temperature, humidity, and gas levels. The data is displayed on a web dashboard and an LCD display, and the system can automatically control a fan and a relay based on the sensor readings.






## Features
- Soil Moisture Monitoring: Measures soil moisture levels using a soil moisture sensor. The system sends a signal to a relay to activate watering when the soil is dry.
- Temperature and Humidity Monitoring: Monitors temperature and humidity levels using a DHT22 sensor.
- Gas Level Monitoring: Measures CO2 concentration using an MQ135 sensor.
- Fan Control: Automatically controls a fan based on temperature thresholds with both manual and automatic modes.
- LCD Display: Displays temperature, humidity, soil moisture, and fan status on a 2004A LCD display.
- Web Dashboard: Displays real-time sensor data and allows for remote monitoring and control.
## Components Used
- ESP32: Microcontroller for handling sensor data and controlling the system.
- DHT22 Sensor: Measures temperature and humidity.
- Soil Moisture Sensor: Measures the moisture level in the soil.
- MQ135 Sensor: Measures the concentration of CO2 in the greenhouse.
- Relay Module: Controls the watering system based on soil moisture readings.
- Fan: Controls the airflow in the greenhouse based on temperature.
- 2004A LCD Display: Displays real-time data on the greenhouse environment.
- Web Dashboard: Allows for remote monitoring and control of the greenhouse system.
## Setup and Installation
Hardware Setup:
- Connect the DHT22 sensor to the ESP32.
- Connect the soil moisture sensor to the ESP32.
- Connect the MQ135 sensor to the ESP32.
- Connect the relay module to the ESP32 and the watering system.
- Connect the fan to the relay.
- Connect the 2004A LCD display to the ESP32.
- Ensure proper power supply to all components.

Software Setup:

- Install the Arduino IDE and set up the ESP32 board.
- Install the required libraries:
- DHT.h for the DHT22 sensor.
- Adafruit_Sensor.h for sensor handling.
- LiquidCrystal_I2C.h for the LCD display.
- WiFi.h for connecting to the web dashboard.
- Upload the code to the ESP32.

Web Dashboard:

- Set up a web server to host the dashboard.
- Configure the ESP32 to send data to the server.
- Access the dashboard via your web browser to monitor and control the greenhouse environment.




## Future Improvements
- Implement machine learning algorithms to predict environmental conditions and optimize control strategies.
- Add more sensors, such as light intensity or pH sensors, to provide a more comprehensive monitoring system.
- Develop a mobile app for easier remote monitoring and control.
