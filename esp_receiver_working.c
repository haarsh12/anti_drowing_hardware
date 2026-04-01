/*
 * ESP32 Simple Debug Test - WORKING VERSION
 * 
 * This is a clean, simple version of your ESP32 code for testing backend debugging
 * Upload this to your ESP32 to test the enhanced backend logging
 */

#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

// ==================== CONFIG ====================
const char* ssid = "Internet";                                    // Your WiFi name
const char* password = "Jislwifi*8";                             // Your WiFi password
const char* serverURL = "http://192.168.1.162:8000/api/alert";   // Backend server URL
const String deviceID = "ESP32_DEBUG_01";

// Fixed GPS coordinates for testing
double latitude = 20.9517;
double longitude = 75.1681;

// ==================== VARIABLES ====================
bool wifiConnected = false;
unsigned long lastSend = 0;
int testCounter = 0;

// ==================== HELPER FUNCTIONS ====================
void printSeparator() {
  for (int i = 0; i < 50; i++) Serial.print("=");
  Serial.println();
}

void connectToWiFi() {
  Serial.print("🌐 Connecting to WiFi: ");
  Serial.println(ssid);
  
  WiFi.begin(ssid, password);
  
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 20) {
    delay(500);
    Serial.print(".");
    attempts++;
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    wifiConnected = true;
    Serial.println("\n✅ WiFi Connected!");
    Serial.print("📶 RSSI: ");
    Serial.println(WiFi.RSSI());
    Serial.print("🌍 IP: ");
    Serial.println(WiFi.localIP());
  } else {
    wifiConnected = false;
    Serial.println("\n❌ WiFi FAILED");
  }
}

void sendData(bool danger) {
  if (!wifiConnected) {
    Serial.println("❌ Cannot send (WiFi not connected)");
    return;
  }
  
  HTTPClient http;
  http.begin(serverURL);
  http.addHeader("Content-Type", "application/json");
  http.addHeader("User-Agent", "ESP32HTTPClient/1.0");  // Identify as ESP32
  
  // Create comprehensive JSON payload
  DynamicJsonDocument doc(400);
  doc["device_id"] = deviceID;
  doc["danger"] = danger;
  doc["latitude"] = latitude;
  doc["longitude"] = longitude;
  doc["location_name"] = "Jalgaon Test Pool";
  
  // Add debug information
  doc["wifi_rssi"] = WiFi.RSSI();
  doc["uptime"] = millis();
  doc["timestamp"] = millis();
  doc["test"] = true;  // Mark as test data
  doc["free_heap"] = ESP.getFreeHeap();
  doc["chip_id"] = String(ESP.getEfuseMac(), HEX);
  
  String payload;
  serializeJson(doc, payload);
  
  printSeparator();
  Serial.println("📤 SENDING DATA TO BACKEND");
  Serial.print("🌐 URL: ");
  Serial.println(serverURL);
  Serial.print("📊 Payload Size: ");
  Serial.print(payload.length());
  Serial.println(" bytes");
  Serial.print("📄 JSON Data: ");
  Serial.println(payload);
  
  int responseCode = http.POST(payload);
  
  Serial.print("📥 RESPONSE CODE: ");
  Serial.println(responseCode);
  
  if (responseCode > 0) {
    String response = http.getString();
    Serial.print("📥 RESPONSE BODY: ");
    Serial.println(response);
    
    if (responseCode == 201) {
      Serial.println("✅ SUCCESS - Data sent to backend!");
      Serial.println("🔍 Check your backend terminal for detailed debug logs!");
    } else if (responseCode == 500) {
      Serial.println("❌ SERVER ERROR - Check backend logs for details");
    }
  } else {
    Serial.println("❌ FAILED TO CONNECT TO SERVER");
    Serial.println("🔧 Check:");
    Serial.println("   • Backend server is running");
    Serial.println("   • IP address is correct");
    Serial.println("   • Firewall allows port 8000");
  }
  
  http.end();
  printSeparator();
}

// ==================== SETUP ====================
void setup() {
  Serial.begin(115200);
  delay(1000);
  
  printSeparator();
  Serial.println("🚀 ESP32 BACKEND DEBUG TEST");
  Serial.println("📡 This will test backend debugging features");
  printSeparator();
  
  // Connect to WiFi
  connectToWiFi();
  
  if (wifiConnected) {
    Serial.println("🧪 Sending initial test data...");
    
    // Send emergency test
    Serial.println("🚨 Test 1: Emergency Alert");
    sendData(true);
    delay(3000);
    
    // Send safe test
    Serial.println("✅ Test 2: Safe Status");
    sendData(false);
    delay(3000);
    
    printSeparator();
    Serial.println("✅ INITIAL TESTS COMPLETE");
    Serial.println("🔄 Will send periodic tests every 15 seconds");
    Serial.println("🔍 Watch your backend terminal for debug logs!");
    printSeparator();
  }
}

// ==================== LOOP ====================
void loop() {
  // Check WiFi connection
  if (WiFi.status() != WL_CONNECTED) {
    if (wifiConnected) {
      Serial.println("⚠️ WiFi lost. Reconnecting...");
      wifiConnected = false;
    }
    connectToWiFi();
  }
  
  // Send test data every 15 seconds
  if (wifiConnected && (millis() - lastSend > 15000)) {
    lastSend = millis();
    testCounter++;
    
    Serial.print("🔁 Periodic Test #");
    Serial.println(testCounter);
    
    // Alternate between danger and safe
    bool isDanger = (testCounter % 2 == 1);
    
    if (isDanger) {
      Serial.println("🚨 Sending EMERGENCY alert...");
    } else {
      Serial.println("✅ Sending SAFE status...");
    }
    
    sendData(isDanger);
  }
  
  delay(100);
}

/*
 * SETUP INSTRUCTIONS:
 * 
 * 1. Update WiFi credentials above (ssid and password)
 * 2. Update serverURL with your computer's IP address
 * 3. Upload this code to your ESP32
 * 4. Open Serial Monitor (115200 baud)
 * 5. Start your backend: python backend_anti/main.py
 * 6. Watch both ESP32 serial output and backend terminal
 * 
 * EXPECTED BEHAVIOR:
 * 
 * ESP32 Serial Monitor will show:
 * - WiFi connection status
 * - JSON payload being sent
 * - HTTP response codes (should be 201 for success)
 * - Server response data
 * 
 * Backend Terminal will show:
 * - Detailed ESP32 communication logs
 * - Device information and status
 * - Emergency vs safe alerts
 * - All debug data from ESP32
 * 
 * TROUBLESHOOTING:
 * 
 * If you get 500 errors:
 * - Check backend terminal for error details
 * - Ensure backend is running properly
 * - Verify database is accessible
 * 
 * If you get connection errors:
 * - Check WiFi credentials
 * - Verify IP address in serverURL
 * - Ensure both devices on same network
 * - Check firewall settings
 */