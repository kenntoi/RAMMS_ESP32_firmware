#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <PZEM004Tv30.h>
#include <HardwareSerial.h>

// WiFi credentials
const char* ssid = "Converge_2.4GHz_B97D";
const char* password = "6GchmMP3";

// API endpoint
const char* apiURL = "http://192.168.1.34/PHP/testState.php";

// Polling interval (milliseconds)
const unsigned long pollInterval = 2000; // Poll every 2 seconds
unsigned long lastPollTime = 0;

// Device to GPIO pin mapping (customize based on your setup)
// Map device IDs to GPIO pins
struct DevicePin {
  int deviceId;
  int gpioPin;
  String deviceName;
  int currentState;
};

// Configure your 2-channel relay - Change device IDs to match your database
DevicePin devices[] = {
  {4, 26, "lights", -1},    // Device ID 4 -> GPIO 26 (Relay Channel 1)
  {5, 27, "tv", -1}         // Device ID 5 -> GPIO 27 (Relay Channel 2)
};

const int numDevices = sizeof(devices) / sizeof(devices[0]);
HardwareSerial pzemSerial(2);   // UART2
PZEM004Tv30 pzem(pzemSerial, 16, 17); // RX, TX

void setup() {
  Serial.begin(115200);
  Serial.println("\n\n=== ESP32 Relay Controller ===");
  
  // Initialize all GPIO pins as outputs
  for (int i = 0; i < numDevices; i++) {
    pinMode(devices[i].gpioPin, OUTPUT);
    digitalWrite(devices[i].gpioPin, LOW); // Start with relays OFF
    Serial.printf("Initialized GPIO %d for Device ID %d (%s)\n", 
                  devices[i].gpioPin, devices[i].deviceId, devices[i].deviceName.c_str());
  }
  
  // Connect to WiFi
  Serial.println("\nConnecting to WiFi...");
  WiFi.begin(ssid, password);
  
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 20) {
    delay(500);
    Serial.print(".");
    attempts++;
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nWiFi Connected!");
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("\nWiFi Connection Failed!");
  }
  
  Serial.println("\n=== Starting Device Monitoring ===\n");
}

void loop() {
  // Check if it's time to poll the API
  if (millis() - lastPollTime >= pollInterval) {
  lastPollTime = millis();

  if (WiFi.status() == WL_CONNECTED) {
    fetchAndUpdateDevices();
  } else {
    Serial.println("WiFi Disconnected! Reconnecting...");
    WiFi.reconnect();
  }

  // NEW → Read power consumption
  readPZEM();
}

}

void fetchAndUpdateDevices() {
  HTTPClient http;
  
  Serial.println("--- Fetching device states from API ---");
  http.begin(apiURL);
  
  int httpResponseCode = http.GET();
  
  if (httpResponseCode == 200) {
    String payload = http.getString();
    Serial.println("API Response received:");
    Serial.println(payload);
    Serial.println();
    
    // Parse JSON
    DynamicJsonDocument doc(4096);
    DeserializationError error = deserializeJson(doc, payload);
    
    if (error) {
      Serial.print("JSON parsing failed: ");
      Serial.println(error.c_str());
      http.end();
      return;
    }
    
    JsonArray devicesArray = doc["devices"];
    
    Serial.println("--- Updating Relay States ---");
    
    // Update each device
    for (JsonObject deviceObj : devicesArray) {
      int deviceId = deviceObj["id"].as<int>();
      String deviceName = deviceObj["name"].as<String>();
      int state = deviceObj["state"].as<String>().toInt();
      String roomName = deviceObj["room_name"].as<String>();
      
      // Find matching device in our array
      for (int i = 0; i < numDevices; i++) {
        if (devices[i].deviceId == deviceId) {
          // Only update if state has changed
          if (devices[i].currentState != state) {
            devices[i].currentState = state;
            digitalWrite(devices[i].gpioPin, state == 1 ? HIGH : LOW);
            
            Serial.printf("✓ Device ID %d (%s) - Room: %s\n", 
                         deviceId, deviceName.c_str(), roomName.c_str());
            Serial.printf("  GPIO %d set to %s (State: %d)\n", 
                         devices[i].gpioPin, state == 1 ? "HIGH" : "LOW", state);
          }
          break;
        }
      }
    }
    
    Serial.println("--- Update Complete ---\n");
    
  } else {
    Serial.printf("HTTP Error: %d\n", httpResponseCode);
    Serial.println(http.errorToString(httpResponseCode));
  }
  
  http.end();
}

// Optional: Function to print current status of all relays
void printDeviceStatus() {
  Serial.println("\n=== Current Device Status ===");
  for (int i = 0; i < numDevices; i++) {
    Serial.printf("Device ID %d (%s): GPIO %d = %s\n",
                 devices[i].deviceId,
                 devices[i].deviceName.c_str(),
                 devices[i].gpioPin,
                 devices[i].currentState == 1 ? "ON" : "OFF");
  }
  Serial.println("============================\n");
}

void readPZEM() {

  float voltage = pzem.voltage();
  float current = pzem.current();
  float power = pzem.power();
  float energy = pzem.energy();
  float frequency = pzem.frequency();
  float pf = pzem.pf();

  if (!isnan(voltage)) {
    Serial.println("----- PZEM Power Data -----");
    Serial.print("Voltage: ");
    Serial.print(voltage);
    Serial.println(" V");

    Serial.print("Current: ");
    Serial.print(current);
    Serial.println(" A");

    Serial.print("Power: ");
    Serial.print(power);
    Serial.println(" W");

    Serial.print("Energy: ");
    Serial.print(energy);
    Serial.println(" kWh");

    Serial.print("Frequency: ");
    Serial.print(frequency);
    Serial.println(" Hz");

    Serial.print("Power Factor: ");
    Serial.println(pf);
    Serial.println("---------------------------\n");
  } 
  else {
    Serial.println("PZEM Read Failed");
  }
}
