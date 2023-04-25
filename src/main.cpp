/*
  João Guilherme

  Projeto feito afim de facilitar o acesso a informações de produção entre outras.
  
  Projeto feito com adaptações do exemplo de @Rui Santos
  Link completo do projeto em https://RandomNerdTutorials.com/esp32-esp-now-wi-fi-web-server/
  
*/

#include <esp_now.h>
#include <WiFi.h>
#include "ESPAsyncWebServer.h"
#include <Arduino_JSON.h>
#include "SPIFFS.h"

// Replace with your network credentials (STATION)
const char* ssid = "ESP32-AP";
const char* password = "123456789";

unsigned long contador = 0;
const char* PARAM_INPUT = "ordem";
int of = 0;            // NUMERO DA ORDEM DE FABRICAÇÃO
bool teste = false;               // Valor Default para lógica de contagem      

// Structure example to receive data
// Must match the sender structure
typedef struct struct_message {
  int id;
  unsigned long contagem;
} struct_message;

struct_message incomingReadings;





JSONVar board;



AsyncWebServer server(80);
AsyncEventSource events("/events");

// callback function that will be executed when data is received
void OnDataRecv(const uint8_t * mac_addr, const uint8_t *incomingData, int len) { 
  // Copies the sender mac address to a string
  char macStr[18];
  Serial.print("Packet received from: ");
  snprintf(macStr, sizeof(macStr), "%02x:%02x:%02x:%02x:%02x:%02x",
           mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);
  Serial.println(macStr);
  memcpy(&incomingReadings, incomingData, sizeof(incomingReadings));
  
  board["of"] = of;
  board["contagem"] = incomingReadings.contagem;
  String jsonString = JSON.stringify(board);
  events.send(jsonString.c_str(), "new_readings", millis());
  
  Serial.printf("Contagem %u: %u bytes\n", incomingReadings.contagem, len);

  Serial.println();
}

void setup() {
  // Initialize Serial Monitor
  Serial.begin(115200);
  SPIFFS.begin();
  // Set the device as a Station and Soft Access Point simultaneously
  WiFi.mode(WIFI_AP_STA);

  WiFi.softAP(ssid, password);
  // Set device as a Wi-Fi Station
  //WiFi.begin(ssid, password);
  Serial.print("Station IP Address: ");
  Serial.println(WiFi.softAPIP());
  Serial.print("Wi-Fi Channel: ");
  Serial.println(WiFi.channel());

  // Init ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }
  
  // Once ESPNow is successfully Init, we will register for recv CB to
  // get recv packer info
  esp_now_register_recv_cb(OnDataRecv);

  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/index.html");
  });
  server.on("/style.css", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(SPIFFS, "/style.css");
  });
  server.on("/logo.png", HTTP_GET, [](AsyncWebServerRequest *request){
  request->send(SPIFFS, "/logo.png", "image/png");
  });
  server.on("/cardapio.png", HTTP_GET, [](AsyncWebServerRequest *request){
  request->send(SPIFFS, "/cardapio.png", "image/png");
  });
  server.on("/update", HTTP_GET, [] (AsyncWebServerRequest *request) {
    String inputMessage;
    // GET input1 value on <ESP_IP>/update?output=<inputMessage1>&state=<inputMessage2>
    if (request->hasParam(PARAM_INPUT)) {
      inputMessage = request->getParam(PARAM_INPUT)->value();
      request->send(200,"text/plain", "OK");
      of = inputMessage.toInt();
    }
    else {
      inputMessage = "No message sent";

    }
    Serial.print("of: ");
    Serial.print(inputMessage);
  });
  events.onConnect([](AsyncEventSourceClient *client){
    if(client->lastId()){
      Serial.printf("Client reconnected! Last message ID that it got is: %u\n", client->lastId());
    }
    // send event with message "hello!", id current millis
    // and set reconnect delay to 1 second
    client->send("hello!", NULL, millis(), 10000);
  });
  server.addHandler(&events);
  server.begin(); // Iniciar o servidor web
}
 
void loop() {
  static unsigned long lastEventTime1 = millis();
  static unsigned long lastEventTime2 = millis();
  static const unsigned long EVENT_INTERVAL_MS = 5000;
  bool status = digitalRead(2);
  
  if(millis() - lastEventTime1 > 100){
    if(!status){
    teste = true;
    lastEventTime1 = millis();
    }
    }
  if(millis() - lastEventTime1 >100){
      if(status && teste){
        teste = false;
        contador++;
        lastEventTime1 = millis();
        Serial.println(contador);
      }
  }
  if ((millis() - lastEventTime2) > EVENT_INTERVAL_MS) {
    events.send("ping",NULL,millis());
    lastEventTime2 = millis();
  }
}