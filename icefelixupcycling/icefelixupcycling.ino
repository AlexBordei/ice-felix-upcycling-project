#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

// Replace with your network credentials
const char* ssid = "eaea.ro";
const char* password = "EarlyAlphaEngineering2024";

// Replace with your OpenAI API key
const char* apiKey = "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";

void setup() {
  // Initialize Serial
  Serial.begin(9600);

  // Connect to Wi-Fi network
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi ..");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print('.');
    delay(1000);
  }
  Serial.println("\nConnected to WiFi. IP address: ");
  Serial.println(WiFi.localIP());

  // Send request to OpenAI API
  String inputText = "Hello, ChatGPT!";
  String apiUrl = "https://api.openai.com/v1/completions";
  String payload = "{\"model\":\"text-davinci-003\", \"prompt\":\"" + inputText + "\",\"max_tokens\":100}";

  HTTPClient http;
  http.begin(apiUrl);
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
      String outputText = jsonDoc["choices"][0]["text"].as<String>();
      Serial.println("Response from OpenAI: ");
      Serial.println(outputText);
    }
  } else {
    Serial.print("Error: HTTP response code ");
    Serial.println(httpResponseCode);
  }
  http.end(); // Close connection
}

void loop() {
  // do nothing
}
