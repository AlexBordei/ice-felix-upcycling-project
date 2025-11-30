#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClientSecure.h>
#include <ArduinoJson.h>
#include <SoftwareSerial.h>

// WiFi credentials - change these to your network
const char* ssid = "YOUR_WIFI_SSID";
const char* password = "YOUR_WIFI_PASSWORD";

// OpenAI API key - get yours from https://platform.openai.com/api-keys
const char* apiKey = "YOUR_OPENAI_API_KEY";

// Conversation history
#define MAX_HISTORY 6
String conversationHistory[MAX_HISTORY];
String conversationRoles[MAX_HISTORY];
int historyCount = 0;

// Input buffer
String inputBuffer = "";

SoftwareSerial SerialOne(D1, D2); // RX, TX

void setup() {
  Serial.begin(115200);
  SerialOne.begin(4800);

  delay(500);
  clearScreen();

  // Titlu festiv
  SerialOne.println("****************************************");
  SerialOne.println("*                                      *");
  SerialOne.println("*   1 DECEMBRIE - ZIUA ROMANIEI!       *");
  SerialOne.println("*   La multi ani, Romania!             *");
  SerialOne.println("*                                      *");
  SerialOne.println("****************************************");
  SerialOne.println();
  SerialOne.print("Conectare WiFi");

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    SerialOne.print(".");
  }
  SerialOne.println(" OK");
  SerialOne.println();

  // Test API
  SerialOne.print("Verificare API...");
  if (testOpenAI()) {
    SerialOne.println(" OK");
  } else {
    SerialOne.println(" EROARE");
  }

  // Meniu
  SerialOne.println();
  SerialOne.println("========================================");
  SerialOne.println("  MENIU - Ce vrei sa afli?");
  SerialOne.println("========================================");
  SerialOne.println();
  SerialOne.println("  1. Spune-mi un banc romanesc");
  SerialOne.println("  2. Poveste sau legenda romaneasca");
  SerialOne.println("  3. Istorie despre 1 Decembrie 1918");
  SerialOne.println("  4. Traditii romanesti");
  SerialOne.println("  5. Personalitati istorice");
  SerialOne.println();
  SerialOne.println("  Sau scrie orice intrebare!");
  SerialOne.println();
  SerialOne.println("========================================");
  SerialOne.println();
  SerialOne.print("Tu: ");
}

void loop() {
  // Read from SerialOne (main terminal)
  while (SerialOne.available()) {
    char c = SerialOne.read();

    // Enter key = 13 (CR)
    if (c == 13) {
      SerialOne.println();

      inputBuffer.trim();
      if (inputBuffer.length() > 0) {
        SerialOne.println();
        SerialOne.println();
        SerialOne.println("Se gandeste...");

        String response = chatGPT(inputBuffer);

        SerialOne.println();
        SerialOne.println();
        SerialOne.print("EAEA: ");
        printWrapped(response);
        SerialOne.println();

        inputBuffer = "";
      }
      SerialOne.print("Tu> ");
    }
    else if (c == 127 || c == 8) {
      // Backspace
      if (inputBuffer.length() > 0) {
        inputBuffer.remove(inputBuffer.length() - 1);
        SerialOne.print("\b \b");
      }
    }
    else if (c >= 32 && c <= 126) {
      // Printable character
      inputBuffer += c;
      SerialOne.print(c);  // Echo character to screen
    }
  }

  // Also check hardware Serial for debugging
  while (Serial.available()) {
    char c = Serial.read();
    if (c == '\n') {
      inputBuffer.trim();
      if (inputBuffer.length() > 0) {
        Serial.println("Sending: " + inputBuffer);
        String response = chatGPT(inputBuffer);
        Serial.println("Response: " + response);
        SerialOne.println();
        SerialOne.print("EAEA: ");
        printWrapped(response);
        SerialOne.println();
        SerialOne.print("Tu> ");
        inputBuffer = "";
      }
    } else if (c != '\r') {
      inputBuffer += c;
    }
  }
}

bool testOpenAI() {
  WiFiClientSecure client;
  client.setInsecure();

  HTTPClient http;
  http.begin(client, "https://api.openai.com/v1/chat/completions");
  http.addHeader("Content-Type", "application/json");
  http.addHeader("Authorization", "Bearer " + String(apiKey));
  http.setTimeout(15000);

  String payload = "{\"model\":\"gpt-4o\",\"max_tokens\":5,\"messages\":[{\"role\":\"user\",\"content\":\"hi\"}]}";
  int code = http.POST(payload);
  http.end();

  Serial.println("API test: " + String(code));
  return (code == 200);
}

String chatGPT(String userInput) {
  WiFiClientSecure client;
  client.setInsecure();

  HTTPClient http;
  http.begin(client, "https://api.openai.com/v1/chat/completions");
  http.addHeader("Content-Type", "application/json");
  http.addHeader("Authorization", "Bearer " + String(apiKey));
  http.setTimeout(30000);

  // Build JSON
  DynamicJsonDocument doc(4096);
  doc["model"] = "gpt-4o";
  doc["max_tokens"] = 500;
  doc["temperature"] = 0.7;

  JsonArray messages = doc.createNestedArray("messages");

  // System message
  JsonObject sys = messages.createNestedObject();
  sys["role"] = "system";
  sys["content"] = "Esti EAEA Asistent pentru Ziua Romaniei (1 Decembrie). Furnizezi informatii despre Romania: bancuri romanesti, legende, istorie, traditii, personalitati. Fii prietenos si festiv! Daca userul scrie doar un numar (1-5), raspunde conform meniului: 1=banc, 2=legenda, 3=istorie 1918, 4=traditii, 5=personalitati. IMPORTANT: NU folosi NICIODATA diacritice sau caractere speciale. Scrie DOAR cu ASCII simplu (a-z, fara a cu accent, i cu accent, s cu sedila, t cu sedila). Raspunsuri scurte, max 3-4 propozitii. OBLIGATORIU: Dupa raspuns, lasa o linie goala apoi pune intrebarea de follow-up pe rand separat.";

  // History
  for (int i = 0; i < historyCount; i++) {
    JsonObject msg = messages.createNestedObject();
    msg["role"] = conversationRoles[i];
    msg["content"] = conversationHistory[i];
  }

  // Current message
  JsonObject usr = messages.createNestedObject();
  usr["role"] = "user";
  usr["content"] = userInput;

  String payload;
  serializeJson(doc, payload);
  Serial.println("Payload: " + payload);

  int code = http.POST(payload);
  String result = "";

  if (code == 200) {
    String response = http.getString();
    Serial.println("Response: " + response);

    DynamicJsonDocument resDoc(4096);
    if (!deserializeJson(resDoc, response)) {
      result = resDoc["choices"][0]["message"]["content"].as<String>();

      // Save to history
      addToHistory("user", userInput);
      addToHistory("assistant", result);
    } else {
      result = "Error: JSON parse failed";
    }
  } else {
    result = "Error: HTTP " + String(code);
  }

  http.end();
  return result;
}

void addToHistory(String role, String content) {
  if (historyCount >= MAX_HISTORY) {
    for (int i = 0; i < MAX_HISTORY - 1; i++) {
      conversationHistory[i] = conversationHistory[i + 1];
      conversationRoles[i] = conversationRoles[i + 1];
    }
    historyCount = MAX_HISTORY - 1;
  }
  conversationRoles[historyCount] = role;
  conversationHistory[historyCount] = content;
  historyCount++;
}

void clearScreen() {
  SerialOne.print("\x1B[2J\x1B[H");
  SerialOne.flush();
}

// Filtreaza caracterele non-ASCII - STRICT
String sanitizeText(String text) {
  String result = "";
  for (unsigned int i = 0; i < text.length(); i++) {
    char c = text.charAt(i);

    // DOAR litere, cifre, spatii, punctuatie standard
    if ((c >= 'a' && c <= 'z') ||
        (c >= 'A' && c <= 'Z') ||
        (c >= '0' && c <= '9') ||
        c == ' ' || c == '.' || c == ',' || c == '!' || c == '?' ||
        c == ':' || c == ';' || c == '-' || c == '_' || c == '\'' ||
        c == '"' || c == '(' || c == ')' || c == '\n' || c == '\r') {
      result += c;
    }
    // Orice altceva e ignorat complet
  }
  return result;
}

// Word wrap print - 80 caractere latime
#define LINE_WIDTH 80

void printWrapped(String text) {
  // Mai intai curata textul
  String clean = sanitizeText(text);

  // Proceseaza linie cu linie (split by newline)
  int start = 0;
  int len = clean.length();

  while (start < len) {
    // Gaseste urmatorul newline
    int nlPos = clean.indexOf('\n', start);
    if (nlPos == -1) nlPos = len;

    // Extrage linia curenta
    String line = clean.substring(start, nlPos);

    // Word wrap aceasta linie
    printLine(line);

    start = nlPos + 1;
  }
}

void printLine(String line) {
  int len = line.length();
  int pos = 0;

  if (len == 0) {
    SerialOne.println();
    return;
  }

  while (pos < len) {
    int lineEnd = pos + LINE_WIDTH;

    if (lineEnd >= len) {
      SerialOne.println(line.substring(pos));
      break;
    }

    // Cauta ultimul spatiu inainte de limita
    int lastSpace = -1;
    for (int i = pos; i < lineEnd && i < len; i++) {
      if (line.charAt(i) == ' ') {
        lastSpace = i;
      }
    }

    if (lastSpace > pos) {
      SerialOne.println(line.substring(pos, lastSpace));
      pos = lastSpace + 1;
    } else {
      SerialOne.println(line.substring(pos, lineEnd));
      pos = lineEnd;
    }
  }
}
