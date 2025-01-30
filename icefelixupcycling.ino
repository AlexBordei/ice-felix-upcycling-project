#include <WiFiManager.h>  // https://github.com/tzapu/WiFiManager
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>
#include <WiFiClient.h>

const char* apiKey = "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";

bool connected = false;
void setup() {
  Serial.begin(115200);

  WiFiManager wm;

  bool res;
  res = wm.autoConnect("AutoConnectAP", "password");  // password protected ap

  if (!res) {
    Serial.println("Failed to connect");
    ESP.restart();
  } else {
    Serial.println("connected...yeey :)");
    connected = true;
  }
}

void loop() {
  if (connected) {
    // Check if user entered something in the Serial Monitor
    if (Serial.available()) {
      String userInput = Serial.readStringUntil('\n');  // Read user input
      userInput.trim();                                 // Remove any trailing whitespace or newlines

      if (userInput.length() > 0) {
        sendMessageToOpenAI(userInput);
      }
    }
  }
}


void sendMessageToOpenAI(String userInput) {
  // OpenAI API endpoint
  String apiUrl = "https://api.openai.com/v1/chat/completions";

  // Create JSON payload
  String payload = "{\"model\": \"gpt-4o-mini\", \"messages\": [";
  payload += "{\"role\": \"developer\", \"content\": \"You are a helpful assistant.\"},";
  payload += "{\"role\": \"user\", \"content\": \"" + userInput + "\"}";
  payload += "]}";
  WiFiClient client;

  // Send HTTP POST request
  HTTPClient http;
  http.begin(client, apiUrl);
  http.addHeader("Content-Type", "application/json");
  http.addHeader("Authorization", "Bearer " + String(apiKey));

  int httpResponseCode = http.POST(payload);
  if (httpResponseCode == HTTP_CODE_OK) {
    String response = http.getString();

    // Parse JSON response
    DynamicJsonDocument jsonDoc(4096);
    DeserializationError error = deserializeJson(jsonDoc, response);
    if (error) {
      Serial.print("Failed to parse JSON: ");
      Serial.println(error.c_str());
    } else {
      String outputText = jsonDoc["choices"][0]["message"]["content"].as<String>();
      Serial.println("ChatGPT: " + outputText);
    }
  } else {
    Serial.print("Error: HTTP response code ");
    Serial.println(httpResponseCode);
  }
  http.end();  // Close connection
}
