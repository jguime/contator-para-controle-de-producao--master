#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <esp_adc_cal.h>
#include "SPIFFS.h"
#include <esp_now.h>
#include <esp_wifi.h>

#define ssid "ESP32-AP"     //   - Nome da rede Wi-Fi do ponto de acesso
#define password "senha123" //

const int serverPort = 80;         //   - Porta do servidor web
const int pinContador = 2;         //   - Pino 2 definido para a leitura do sinal de contagem

//_______________________________________________________________________________________________PARAMETROS 
unsigned long tempoCorrente = 0;
unsigned long contador = 0;
int contador_slv_1 = 0;
int contador_slv_2 = 0;
int contador_slv_3 = 0;
int contador_slv_4 = 0;

unsigned int of = 0;              // NUMERO DE ORDEM DE FABRICAÇÃO
bool teste = false;               // Valor Default para lógica de contagem      

AsyncWebServer server(serverPort);  

// Define a estrutura das informações transmitidas
typedef struct struct_message {
  String setor;
  unsigned long contador;
} struct_message;
 
// Create structured data object
struct_message myData;

void OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len) 
{
  // Get incoming data
  memcpy(&myData, incomingData, sizeof(myData));
  Serial.println("Receive Data: ");
  Serial.print(myData.setor);
  Serial.print(": ");
  Serial.println(myData.contador);
  
}
void setup() {
  // Montagem do SPiFFS (acesso a memoria flash do ESP)
  SPIFFS.begin();
  Serial.begin(115200);
  WiFi.mode(WIFI_AP_STA);
  WiFi.printDiag(Serial); // Uncomment to verify channel number before
  WiFi.printDiag(Serial); // Uncomment to verify channel change after
  // Configurar o ESP32 APmode (Acess Point)
  
  Serial.print("MacAddress: ");
  //Serial.println(WiFi.macAddress());
  
  Serial.print("Endereço IP do ponto de acesso: ");
  Serial.println(WiFi.softAPIP());
  
    // Initalize ESP-NOW
  if (esp_now_init() != 0) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

    // Register callback function
  esp_now_register_recv_cb(OnDataRecv);
  
  pinMode(pinContador, INPUT_PULLDOWN);

  //_________________________________________________________________________ CONFIGURAÇÕES DO SERVIDOR WEB.
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
  server.on("/contagem", HTTP_GET, [](AsyncWebServerRequest *request){
    String ContagemString = String(contador);
    request->send(200, "text/plain", ContagemString);  // Enviar a temperatura como resposta
    contador = 0;                                      // Zera o contador sempre que enviar a requisição ao site.
  });
  server.on("/of", HTTP_GET, [](AsyncWebServerRequest *request){
    String OfString = String(of);
    request->send(200, "text/plain", OfString);
  }); 
  server.begin(); // Iniciar o servidor web
}

void loop() {
  bool status = digitalRead(pinContador);
  
  if(millis() - tempoCorrente > 100){
    if(!status){
    teste = true;
    tempoCorrente = millis();
    }
    }
  if(millis() - tempoCorrente >100){
      if(status && teste){
        teste = false;
        contador++;
        tempoCorrente = millis();
        Serial.println(contador);
      }
  }
  
}
