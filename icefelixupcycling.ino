#include <WiFiManager.h>  // https://github.com/tzapu/WiFiManager
#include <ESP8266HTTPClient.h>
#include <WiFiClientSecure.h>  // Secure client for HTTPS
#include <ArduinoJson.h>
#include <SoftwareSerial.h>

const char* apiKey = "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";


// Define additional serial port
SoftwareSerial SerialOne(D1, D2); // RX, TX

void setup() {
  Serial.begin(115200);  // Hardware Serial for debugging
  SerialOne.begin(9600); // Software Serial for main communication

  clearScreen();  // Clear screen at startup

  SerialOne.println("=== Early Alpha Engineering AI Chat ===");
  SerialOne.println("Connecting to WiFi...");
  SerialOne.println("===========================================");
  SerialOne.println();

  WiFiManager wm;

  // Connect to WiFi with a fallback AP mode
  if (!wm.autoConnect("AutoConnectAP", "password")) {
    Serial.println("Failed to connect, restarting...");
    SerialOne.println("Failed to connect. Device will restart...");
    delay(3000);
    ESP.restart();
  }

  Serial.println("Connected to WiFi!");
  
  // Show ready message only after successful connection
  clearScreen();
  SerialOne.println("=== Early Alpha Engineering AI Chat ===");
  SerialOne.println("Connected and ready!");
  SerialOne.println("Type your message and press Enter to send.");
  // SerialOne.println("===========================================");
}

void loop() {
  static unsigned long lastCheckTime = 0;
  static bool wasConnected = false;
  
  if (WiFi.status() == WL_CONNECTED) {
    if (!wasConnected) {
      // Just reconnected
      clearScreen();
      SerialOne.println("=== Early Alpha Engineering AI Chat ===");
      SerialOne.println("Connection restored!");
      SerialOne.println("===========================================");
      SerialOne.println();
      SerialOne.write('\r');
      wasConnected = true;
    }
    checkSerialAndSend(Serial);    // For debug input
    checkSerialAndSend(SerialOne); // For main communication
  } else {
    // Check every 5 seconds
    if (millis() - lastCheckTime > 5000) {
      if (wasConnected) {
        // Just disconnected
        clearScreen();
        SerialOne.println("=== Early Alpha Engineering AI Chat ===");
        SerialOne.println("WiFi disconnected!");
        SerialOne.println("Attempting to reconnect...");
        SerialOne.println("===========================================");
        SerialOne.println();
        SerialOne.write('\r');
      }
      Serial.println("WiFi disconnected! Waiting for reconnection...");
      lastCheckTime = millis();
      wasConnected = false;
    }
  }

  // Check for clear screen command
  if (SerialOne.available()) {
    String msg = SerialOne.readStringUntil('\n');
    msg.trim();
    if (msg == "[6~") {
      clearScreen();
      // Reprint the welcome message
      SerialOne.println("=== Early Alpha Engineering AI Chat ===");
      SerialOne.println("Type your message and press Enter to send.");
      SerialOne.println("===========================================");
      SerialOne.println();
    }
  }
}

void checkSerialAndSend(Stream &serial) {
  static String inputBuffer; // Buffer to store input data
  
  while (serial.available()) {
    char receivedChar = serial.read();
    
    // Echo character to debug Serial immediately if from SerialOne
    if (&serial == &SerialOne) {
      Serial.print(receivedChar);
    }
    
    if (receivedChar == '\n') { // User pressed Enter, send request
      inputBuffer.trim();  // Remove trailing spaces
      if (inputBuffer.length() > 0) {
        Serial.println(); // New line after the echoed input
        Serial.println("Debug: Received complete message: " + inputBuffer);
        
        Serial.println("Debug: Sending request to OpenAI...");
        SerialOne.println("Sending request to OpenAI...");
        sendMessageToOpenAI(inputBuffer, SerialOne);
        inputBuffer = ""; // Clear buffer after sending
      }
    } else {
      inputBuffer += receivedChar;  // Append character to buffer
    }
  }
}

void sendMessageToOpenAI(String userInput, Stream &responseSerial) {
  WiFiClientSecure client;
  client.setInsecure();

  HTTPClient http;
  http.begin(client, "https://api.openai.com/v1/chat/completions");
  http.addHeader("Content-Type", "application/json");
  http.addHeader("Authorization", "Bearer " + String(apiKey));

  // Create JSON payload for a single message
  DynamicJsonDocument jsonDoc(2048);
  jsonDoc["model"] = "gpt-4";
  jsonDoc["max_tokens"] = 1000;
  jsonDoc["temperature"] = 0.7;
  JsonArray messages = jsonDoc.createNestedArray("messages");

  // Add system message
  JsonObject systemMessage = messages.createNestedObject();
  systemMessage["role"] = "system";
  systemMessage["content"] = "You are a helpful assistant. Provide concise responses.";

  // Add current user message
  JsonObject userMessage = messages.createNestedObject();
  userMessage["role"] = "user";
  userMessage["content"] = userInput;

  String payload;
  serializeJson(jsonDoc, payload);
  
  Serial.println("Debug: Sending payload to OpenAI:");
  Serial.println(payload);

  int httpResponseCode = http.POST(payload);

  if (httpResponseCode > 0) {
    String response = http.getString();
    Serial.println("Debug: Received HTTP Response Code: " + String(httpResponseCode));
    Serial.println("Debug: Raw API Response:");
    Serial.println(response);

    DynamicJsonDocument responseJson(2048);
    DeserializationError error = deserializeJson(responseJson, response);

    if (error) {
      String errorMsg = "Error parsing response: " + String(error.c_str());
      responseSerial.println(errorMsg);
      Serial.println("Debug: " + errorMsg);
    } else {
      String outputText = responseJson["choices"][0]["message"]["content"].as<String>();
      
      Serial.println("Debug: ChatGPT Response:");
      Serial.println(outputText);
      
      responseSerial.println("ChatGPT: " + outputText);
    }
  } else {
    String errorMsg = "HTTP Request failed. Error code: " + String(httpResponseCode);
    responseSerial.println(errorMsg);
    Serial.println("Debug: " + errorMsg);
  }

  http.end();
}

// Function to clear the screen
void clearScreen() {
  SerialOne.write(27); // ESC
  SerialOne.print("[2J"); // Clear screen
  SerialOne.write(27);
  SerialOne.print("[H");  // Move cursor to home position
  SerialOne.write('\r');  // Return cursor to start of line
}
