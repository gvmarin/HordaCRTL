/*
 Controlador de Fermentacão - Geladeira /aquecimento conectado via MQtt 

 To install the ESP8266 board, (using Arduino 1.6.4+):
  - Add the following 3rd party board manager under "File -> Preferences -> Additional Boards Manager URLs":
       http://arduino.esp8266.com/stable/package_esp8266com_index.json
  - Open the "Tools -> Board -> Board Manager" and click install for the ESP8266"
  - Select your ESP8266 in "Tools -> Board"

ESP8266 ver 2.6.3
usando as Bibliotecas extras
ArduinoJson by Benoit Blanchon Version 5.13.4 - sem uso a partir da versao 3.0
DallasTemperature by Milles Burton, Tim Newsome, Guil Barros, Rob Tilaart version 3.9.0
Encoder by Paul Stoffregen Version 1.4.4 (versao do github, por causa do conflito de interrupcao) encoder.h
ESP8266 and ESP32 Oled Driver for SSD1306 Display by Daniel Eichhorn, Fabrice Weinberg Version 4.0.0   (4.1.0)
OneWire by Jim Studt, Tom Pollard, Robim James,Glenn Trewitt, Jason Dangel,Guillermo Lovato, Paul Stoffregen,
  Scott Robberts, Bertrik Sikken, Mark Tillotson, Ken Butcher, Roger Clark, Love Nystrom Version 2.3.7
PubSubClient by Nick O'Leary version 2.7.0
Wifimanager by tzapu V2.0.17 (V0.15.0)


*/
//#define Ver V2.0
/* Versao 2.0
* melhorias
* - assinatura de eeprom gravada para o portal de config 
* - DEfinida nova logica dos sensores para acomodar geladeira com sensor interno fixo

*/
#define Ver V3.0
/* Versao 3.0
* - Mudanca para servidor MQTT generico, incluindo usuario e password - ok
* - deixar de usar Json  - ok
* - Atualizar bibliotecas - ok exceto WifiManager que ~ao funcionou com OTA
* - incluir OTA para futuro - ok
* - definir onde atualizar a geladeira / pensar no stand alone
* - Incluir desenho da resistencia quando aquecendo
* - 
*
*/
/* Pendencias
* - ajuste temperatura negativa (-0.1 a -0.9) n~ao vai apareceer certo -- OK
* - incluir limites de temp no botao
* - incluir Bateria na mensagem do Mqtt 
* - Avaliar so mandar mensagem somente para parametros que mudaram
* - incluir mensagem de conectando ao servidor
* - Verificar se e possivel passar os parametors de wifi e MQTT por mensagem
*/


#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>   // veirificar para que
#include <DNSServer.h>    // verificar para que
#include <WiFiManager.h>          //https://github.com/tzapu/WiFiManager
#include <OneWire.h>
#include <DallasTemperature.h>
#include <Encoder.h>
#include "SH1106Wire.h" //alias for `#include "SH1106Wire.h"
//#include "SSD1306Wire.h" //display menor 'e SSD1306
#include <Ticker.h>
#include <EEPROM.h>
#include <PubSubClient.h>
#include <ArduinoOTA.h>

// Include custom images
/*#include "images.h"*/ 
//http://dotmatrixtool.com/#
// 16 x 16 row major litle endian 
const uint8_t cloud[] PROGMEM = {
//cloud com seta
0x80, 0x03, 0x60, 0x0c, 0x30, 0x18, 0x18, 0x10, 0x04, 0x30, 0x04, 0x40, 0x06, 0x81, 0x81, 0x83, 0x41, 0x85, 0x01, 0x81, 0x02, 0x41, 0x3c, 0x39, 0x00, 0x01, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00


// cloud 
//0x00, 0x0f, 0x80, 0x10, 0x70, 0x20, 0x08, 0x20, 0x08, 0x10, 0x0c, 0x70, 0x02, 0x80, 0x01, 0x80, 0x01, 0x80, 0x02, 0x40, 0xfc, 0x3f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

const uint8_t cool[] PROGMEM = {
// geladeira maior
// 0x00, 0x7e, 0x00, 0x81, 0x00, 0x85, 0x00, 0x81, 0x00, 0x81, 0x00, 0xff, 0x00, 0x81, 0x00, 0x85, 0x00, 0x85, 0x00, 0x81, 0x00, 0x81, 0x00, 0x81, 0x00, 0x7e, 0x00, 0x66, 0x00, 0x00, 0x00, 0x00
// gelade menor
0x00, 0x7e, 0x00, 0x81, 0x00, 0x85, 0x00, 0x81, 0x00, 0x81, 0x00, 0xff, 0x00, 0x81, 0x00, 0x85, 0x00, 0x85, 0x00, 0x81, 0x00, 0x81, 0x00, 0x7e, 0x00, 0x66, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00

// floco linhs pequena meio cheio
//0x00, 0x00, 0x00, 0x00, 0x40, 0x01, 0x90, 0x04, 0x98, 0x0c, 0xa0, 0x02, 0xc4, 0x11, 0xf8, 0x0f, 0xc4, 0x11, 0xa0, 0x02, 0x98, 0x0c, 0x90, 0x04, 0x40, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
// floco linha pequeno meio vazio
//0x00, 0x00, 0x00, 0x00, 0x40, 0x01, 0x90, 0x04, 0x98, 0x0c, 0xa0, 0x02, 0x04, 0x10, 0x38, 0x0e, 0x04, 0x10, 0xa0, 0x02, 0x98, 0x0c, 0x90, 0x04, 0x40, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
// floco linhas 
//0x00, 0x00, 0x20, 0x02, 0x40, 0x01, 0x90, 0x04, 0x98, 0x0c, 0xa2, 0x22, 0x04, 0x10, 0x38, 0x0e, 0x04, 0x10, 0xa2, 0x22, 0x98, 0x0c, 0x90, 0x04, 0x40, 0x01, 0x20, 0x02, 0x00, 0x00, 0x00, 0x00
// floco menor 
//0x00, 0x00, 0xa0, 0x02, 0xc0, 0x01, 0xa8, 0x0a, 0xb0, 0x06, 0xba, 0x2e, 0xc4, 0x11, 0xfe, 0x3f, 0xc4, 0x11, 0xba, 0x2e, 0xb0, 0x06, 0xa8, 0x0a, 0xc0, 0x01, 0xa0, 0x02, 0x00, 0x00, 0x00, 0x00
// floco de neve mais redondo
//0xa0, 0x02, 0xc0, 0x01, 0x94, 0x14, 0x98, 0x0c, 0x9c, 0x1c, 0xa1, 0x42, 0xc2, 0x21, 0xff, 0x7f, 0xc2, 0x21, 0xa1, 0x42, 0x9c, 0x1c, 0x98, 0x0c, 0x94, 0x14, 0xc0, 0x01, 0xa0, 0x02, 0x00, 0x00
// floco de neve mais quadrado 
//0x80, 0x00, 0xaa, 0x2a, 0xcc, 0x19, 0xce, 0x39, 0x90, 0x04, 0xa2, 0x22, 0xcc, 0x19, 0xff, 0x7f, 0xcc, 0x19, 0xa2, 0x22, 0x90, 0x04, 0xce, 0x39, 0xcc, 0x19, 0xaa, 0x2a, 0x80, 0x00, 0x00, 0x00
};

// Update these with values suitable for your network.

//const char* ssid = "name";
//const char* password = "password";

//const char* mqtt_server = "mqtt.beebotte.com";

#define ServerSize 40   // comprimento nome do servidor
#define UserSize 40   // Comprimento do usuario
#define PassSize 40   // comprimento da senha

char mqtt_server[ServerSize] = "mosquitto.local";
char user[UserSize]= "mosquitto";  // Usuario
char pass[PassSize]="123";// senha



//#define Token "token:1506993457205_dN58q0ZH5dFQ5DXc"  // Token do beebotte

//#define Channel "Geladeira_dev"                           // "equipamento teste"
#define Channel "Geladeira"                           // "equipamento normal"
#define Ambiente "stat/AmbTemp"                            //Temperatura Ambiente
#define Interna "stat/FridgeTemp"                          //Temperatura dentro da geladeira
#define Fermentador1 "stat/Ferm1Temp"                      //Temperatura no fermentador 1 (principal)
#define Ajuste "stat/TargetTemp"                           //Temperatura ajustada para reporte
#define AjusteIn "cmd/TargetTemp"                        //Temperatura ajustada informada 
#define Histerese "stat/Histeresys"                        //Histerese
#define HistereseIn "cmd/Histeresys"                        //Histerese
#define Geladeira "stat/CoolingON"                         //Stauts do controle Geladeira para reporte
#define Aquecedor "stat/HeatingON"                         //Status do controle aquecimento para reporte
#define Modo "stat/Mode"                                   // modo de operacao se collingon = cool, heatingon = heat, nenhum dos dois = off
#define Availability "stat/Availability"                // availability topic
#define Write true                                         // persistent
#define EPPsize 1+8*2+ServerSize+UserSize+PassSize            // tamanho da eeprom (1 assinatura, 2 x 8 sensor + 40 server + 40 user + 40 Password)
#define EPPsig 0                                      // Assinatura da epprom pra saber se foi inicializada
#define EPPsamb EPPsig+1                              // endereco do sensor de tem ambiente na eeprom
#define EPPsgel EPPsamb+8                             // endereco do sensor de tem dentro da geladeira na eeprom
#define EPPserver EPPsgel+8                           // endereco do servidor na eeprom
#define EPPuser EPPserver+ServerSize                 // endereco do username na eeprom
#define EPPpass EPPuser+UserSize                     // endereo da senha na eeprom
#define OutGeladeira 16                               // GPIO16 controle do compressor
#define OutAquecedor 15                               // GPIO15 Controle aquecedor
 
#define T_ATUALIZA 60000 // 60 segundos, atualiza 1 vez por minuto
#define T_RECONN 300000 // 5 minutos
#define T_AJUSTE 10000  // 10 segundos
#define TEMPMAX 99.9  // temperatura maxima que cabe no display
#define TEMPMIN -9.9  // temperatura minima que cabe no display

#define Button 14
#define OutBeep 2

#define BCurto 20
#define BLongo 50

#define DISPLAY_WIDTH 128
#define DISPLAY_HEIGHT 64

Encoder myEnc(13, 12);  // alguns encoders tem sequencia diferente, pode ser necessario trocar os pinos ou os fios no encoder

SH1106Wire display(0x3c, 4, 5);
//SSD1306Wire display(0x3c, 4, 5);

// Porta do pino de sinal do DS18B20
#define ONE_WIRE_BUS 0
// Define uma instancia do oneWire para comunicacao com os sensores
OneWire oneWire(ONE_WIRE_BUS);

DallasTemperature sensors(&oneWire);
DeviceAddress Sferm1,Samb,Sgel,Sferm2,Sepp;

WiFiClient espClient;
PubSubClient client(espClient);

Ticker btn_timer;
Ticker beeper;

unsigned long lastMsg = 0,lastAdj = 0, lastConn=0; //Timers
char msg[50];
char value = 0;
// Armazena temperatura ajustada e histerese
volatile float ajuste = 25;
volatile float histerese = 1;

volatile struct
{
  uint8_t cooling: 1;     // modo gelar ou aquecer
  uint8_t geladeira: 1;   // geladeira ligada
  uint8_t aquecedor: 1;   // aquecedir ligado
  uint8_t gel:  1;        // sensor de temp da geladeira
  uint8_t ferm2:  1;      // sensor de temp do fermentador 2
  uint8_t ferm1:  1;      // sensor de temp do fermentador 1
  uint8_t modo: 2;        // modo ajustes
  uint8_t offline: 1;     // modo offline (sem wifi ou sem servidor)
  uint8_t bt: 1;        //botao
  uint8_t savepar:  1;  //Salvar parametros de config
  uint8_t wifi: 1;      // Wifi Conectado 
}
flags;

long oldPosition  = -999;
//boolean isButtonPressed = false;
//long lastUpdateMillis = 0;
int bt_count = 0;

char temp[5][8]{"00.0°C","00.0","A: 00.0","I: 00.0","2: 00.0"};


void setup() {
  
  Serial.begin(115200);
  //Serial.println("Basic Encoder Test:");
  EEPROM.begin(EPPsize);
  display.init();

  display.flipScreenVertically();

  display.setContrast(255);

  pinMode(OutGeladeira, OUTPUT);     // Initialize the gelareira out pin as an output
  pinMode(OutAquecedor, OUTPUT);     // Initialize the aquecedor out pin as an output
  pinMode(Button, INPUT);            // initialize the button input
  pinMode(OutBeep, OUTPUT);             // initialize the beep as output

  digitalWrite(OutGeladeira, LOW );
  digitalWrite(OutAquecedor, LOW );
  digitalWrite(OutBeep, LOW ); 

  if( readepp_sig())  readepp_mqttpar();      // busca nome server e login caso a Epprom já esteja inicializada
  
  
  flags.savepar=false; // não precisa salvar config
  flags.wifi=false;
  //WiFi.persistent(true);
  if(digitalRead(Button)==LOW) {  // é para entrar modo de configuracao
    
   Serial.println("Criando portal de configuração");
   WiFiManagerParameter custom_mqtt_server("Server", "mqtt server", mqtt_server, 40);
   WiFiManagerParameter custom_user("User", "user", user, 40);
   WiFiManagerParameter custom_pass("Password", "pass", pass, 40); 

    WiFiManager wifiManager;
    
    wifiManager.setConfigPortalTimeout(180);
    wifiManager.setRemoveDuplicateAPs(true);

    //set config save notify callback
    wifiManager.setSaveConfigCallback(saveConfigCallback);
    //add all your parameters here
   
   // wifiManager.setDebugOutput(true);
    wifiManager.addParameter(&custom_mqtt_server);
    wifiManager.addParameter(&custom_user);
    wifiManager.addParameter(&custom_pass);


    display.clear();
    display.drawRect(0,0,DISPLAY_WIDTH,DISPLAY_HEIGHT);
    display.setTextAlignment(TEXT_ALIGN_CENTER);
    display.setFont(ArialMT_Plain_10);
    display.drawString(63, 3, "CONFIGURAÇÃO");
    display.setFont(ArialMT_Plain_10);
    display.drawString(63, 18, "Conecte ao Wifi");
    display.drawString(63, 31, "Hordabier");
    display.drawString(63, 45, "e acesse 192.168.4.1");
    display.display();
  
    if (!wifiManager.startConfigPortal(Channel)) {
      Serial.println("falha em conectar, reiniciando");
      delay(3000);
      //reset and try again, 
      ESP.reset();
      delay(5000);
    }
    //if you get here you have connected to the WiFi
    if(flags.savepar) {
      Serial.println("SSID e Senha aceitos");
      Serial.println("Atualizar configuração");
      strcpy(mqtt_server, custom_mqtt_server.getValue());
      Serial.print("Servidor : ");
      Serial.println(mqtt_server);
      strcpy(user, custom_user.getValue());
      Serial.print("user : ");
      Serial.println(user);
      strcpy(pass, custom_pass.getValue());
      Serial.print("pass : ");
      Serial.println(pass);
      writeepp_sig();     // grava assinatura de EEP atualizada
      writeepp_mqttpar();  // guarda novos parametros do mqtt
   }
    delay(3000);
      //reset and connect
    //ESP.reset();
    //delay(5000);
  }
  display.clear();
  display.drawRect(0,0,DISPLAY_WIDTH,DISPLAY_HEIGHT);
  display.setTextAlignment(TEXT_ALIGN_CENTER);
  display.setFont(ArialMT_Plain_24);
  display.drawString(63, 5, "HordaBier");
  display.setFont(ArialMT_Plain_10);
  display.drawString(63, 32, "Connectando");
  display.display();

  setup_wifi();

  display.clear();
  display.drawRect(0,0,DISPLAY_WIDTH,DISPLAY_HEIGHT);
  display.setTextAlignment(TEXT_ALIGN_CENTER);
  display.setFont(ArialMT_Plain_24);
  display.drawString(63, 5, "HordaBier");
  display.setFont(ArialMT_Plain_10);
  if (!flags.offline) display.drawString(63, 32, "Connectado");
  else display.drawString(63, 32, "Offline");
  display.display();
// inicializa OTA
Serial.println("inicializando OTA");

// Port defaults to 8266
  // ArduinoOTA.setPort(8266);

  // Hostname defaults to esp8266-[ChipID]
  
  ArduinoOTA.setHostname(Channel);  // usa o nome do equipamento como hostname

  // No authentication by default
  ArduinoOTA.setPassword("admin");

  // Password can be set with it's md5 value as well
  // MD5(admin) = 21232f297a57a5a743894a0e4a801fc3
  // ArduinoOTA.setPasswordHash("21232f297a57a5a743894a0e4a801fc3");


ArduinoOTA.onStart([]() {
    String type;
    if (ArduinoOTA.getCommand() == U_FLASH) {
      type = "sketch";
    } else { // U_FS
      type = "filesystem";
    }

    // NOTE: if updating FS this would be the place to unmount FS using FS.end()
    Serial.println("Start updating " + type);
  });
  ArduinoOTA.onEnd([]() {
    Serial.println("\nEnd");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) {
      Serial.println("Auth Failed");
    } else if (error == OTA_BEGIN_ERROR) {
      Serial.println("Begin Failed");
    } else if (error == OTA_CONNECT_ERROR) {
      Serial.println("Connect Failed");
    } else if (error == OTA_RECEIVE_ERROR) {
      Serial.println("Receive Failed");
    } else if (error == OTA_END_ERROR) {
      Serial.println("End Failed");
    }
  });
  ArduinoOTA.begin();
  
  btn_timer.attach(0.05, button);

// Versao nova , usada na geladeira. primeiro sensor é sempre Amb. 2o é sempre gel, 3 ferm 1 e 4 ferm2, caso 2, gel e ferm 1 sao o mesmo

  sensors.begin();
  // Localiza e mostra enderecos dos sensores
  Serial.println("Localizando sensores DS18B20...");
  Serial.print("Foram encontrados ");
  int nsens = sensors.getDeviceCount();
  flags.gel=false;
  flags.ferm2=false;
  flags.ferm1=false;
  Serial.print(nsens, DEC);
  Serial.println(" sensores.");
  readepp_sens(EPPsamb,Samb);  
  Serial.print("Endereco Sensor Ambiente guardados na EEPROM: ");
  mostra_endereco_sensor(Samb);
  Serial.println();
  readepp_sens(EPPsgel,Sgel);  
  Serial.print("Endereco Sensor Interno guardados na EEPROM: ");
  mostra_endereco_sensor(Sgel);
  Serial.println();
  if (nsens==0) Serial.println("Sensores nao encontrados !");
  if (nsens==1) {
    //readepp_sens(EPPsamb,Sepp);  // recupera sensor Amb da epprom
    sensors.getAddress(Sepp, 0);
    Serial.print("Endereco sensor  Temp. Ambiente: ");
    mostra_endereco_sensor(Sepp);
    Serial.println();
    if (!comp_end(Samb,Sepp)) {
      sensors.getAddress(Samb, 0);
      writeepp_sens(EPPsamb,Samb);   // grava se for diferente
      Serial.println(" Novo sensor Temp. Ambiente, gravando sensor EEPROM");
    }
    Serial.println("Somente sensor ambiente, controle de temp desabilitado");
   } 
  if (nsens>1) {   // mais de um sensores , entao tem 1 ambiente e 1 gel. )
    int i=0; // contador
    bool amb=false;
    for (i=0;i<nsens;i++) {
      sensors.getAddress(Sepp, i);
      Serial.print("S: ");
      Serial.print(i, DEC);
      Serial.print(" ID: ");
      mostra_endereco_sensor(Sepp);
      Serial.print(" - ");
      if(!amb && comp_end(Sepp,Samb)) { // encontrou ambiente 
        amb=true;
        Serial.println("Temp. Ambiente");
        continue;
      }
      if(!flags.gel && comp_end(Sepp,Sgel)) { // encontrou sensor interno
        flags.gel=true;
        Serial.println("Interno");
        continue;
      }          
      if(nsens>2 && !flags.ferm1) { // seleciona como Ferm1
        sensors.getAddress(Sferm1, i); // usa comofermaentador 1
        flags.ferm1=true;
        Serial.println("Fermentador 1");
        continue;
      }
      if(nsens>3 && !flags.ferm2) { // seleciona como Ferm2
        sensors.getAddress(Sferm2, i); // usa comofermaentador 1
        flags.ferm2=true;
        Serial.println("Fermentador 2");
        continue;
      }
      Serial.println(); // se não achou, pula linha
    }
     if (amb) { // ambiente ok, mas tem pelo menos mais um;
      if(nsens==2) { //apenas 2 sensores
        if (!flags.gel) { // definir novo sensor
          Serial.println("Definindo novo Sensor Interno");
            for (i=0;i<nsens;i++) {  // 
              sensors.getAddress(Sgel, i);
              Serial.print("S: ");
              Serial.print(i, DEC);
              Serial.print(" ID: ");
              mostra_endereco_sensor(Sgel);
              Serial.print(" - ");
              if(!comp_end(Samb,Sgel)) {   // não é ambiente, então é interno)
               flags.gel=true;
               Serial.println("Interno");
               writeepp_sens(EPPsgel,Sgel);   // grava na eeprom
              } else Serial.println("Temp. Ambiente");
            } 
          Serial.println("Gravado Na EEPROM");  
        }
         flags.ferm1=true;   // habilita Ferm1
         copy_end(Sferm1, Sgel); // copia endereco do sensor de geladeira no mermentador 1
      }
      if (nsens>2 && !flags.gel) { // multiplos sensores e não encontrou o interno
        Serial.println("multiplos sensores e Sensor Interno nao encontrado. ligue apenas com Sensor Temp. Ambiente e Sensor Interno, para redefinir o Sensor Interno"); 
      }
     } else Serial.println("multitplos sensores e Sensor de Temp. Ambiente nao encontrado. ligue apenas com um sensor para definir o Sensor de temp. ambiente"); 
  } 
  sensors.setResolution(12);  // define resolução maxima
  Serial.println();

/* rotina antiga, usada no controlador stand alone
 
  sensors.begin();
  // Localiza e mostra enderecos dos sensores
  Serial.println("Localizando sensores DS18B20...");
  Serial.print("Foram encontrados ");
  int nsens = sensors.getDeviceCount();
  flags.gel=false;
  flags.ferm2=false;
  flags.ferm1=false;
  Serial.print(nsens, DEC);
  Serial.println(" sensores.");
  readepp_sens(EPPsamb,Sepp);  
  Serial.print("Endereco guardado na     EEPROM: ");
  mostra_endereco_sensor(Sepp);
  Serial.println();
  if (nsens==0) {
    Serial.println("Sensores nao encontrados !"); 
  } else
      if (nsens==1) {
            sensors.getAddress(Samb, 0);
            Serial.print("Endereco sensor  Temp. Ambiente: ");
            mostra_endereco_sensor(Samb);
            Serial.println();
            writeepp_sens(EPPsamb,Samb);
            Serial.println("Gravando Sensor Tamb na EEPROM");
      } else {
          Serial.println("procurando sensor Amb");
          for (int i=0;i<nsens;i++) { //procura o Sensor temp ambiente
            sensors.getAddress(Samb, i);
            
            Serial.print("S: ");
            Serial.print(i, DEC);
            Serial.print(" ID: ");
            mostra_endereco_sensor(Samb);
            Serial.println();
            
            if(comp_end(Sepp,Samb) ) {
              Serial.print("Endereco sensor  Temp. Ambiente: "); // enderecos iguas, encontrou
              mostra_endereco_sensor(Samb);
              Serial.println();
              break;
            }
          }
          flags.ferm1=true;     // dois sensores ao menos 2, entao tem 1 externo no minimo (ferm1)
          for (int i=0;i<nsens;i++) {
            sensors.getAddress(Sferm1, i);

            Serial.print("S: ");
            Serial.print(i, DEC);
            Serial.print(" ID: ");
            mostra_endereco_sensor(Sferm1);
            Serial.println();
            
            if(!comp_end(Sferm1,Samb) ) {
              Serial.print("Endereco sensor   Fermentador 1: ");
              mostra_endereco_sensor(Sferm1);
              Serial.println();
              break;
            }
          }
          if (nsens>2) {        // 3 sensores ao menos
            flags.gel=true;          
            for (int i=0;i<nsens;i++) {
              sensors.getAddress(Sgel, i);

              Serial.print("S: ");
              Serial.print(i, DEC);
              Serial.print(" ID: ");
              mostra_endereco_sensor(Sgel);
              Serial.println();
              
              if(!comp_end(Sgel,Sferm1) && !comp_end(Sgel,Samb)) {
                Serial.print("Endereco sensor temp. Geladeira: ");
                mostra_endereco_sensor(Sgel);
                Serial.println();
                break;
              }
            }
          }
          if (nsens>3) {      // 4 sensores
            flags.ferm2=true;          
            for (int i=0;i<nsens;i++) {
              sensors.getAddress(Sferm2, i);

              Serial.print("S: ");
              Serial.print(i, DEC);
              Serial.print(" ID: ");
              mostra_endereco_sensor(Sferm2);
              Serial.println();
              
              if(!comp_end(Sferm2,Sgel) && !comp_end(Sferm2,Sferm1) && !comp_end(Sferm2,Samb)) {
                Serial.print("Endereco sensor   Fermentador 2: ");
                mostra_endereco_sensor(Sferm2);
                Serial.println();
                break;
              }
            }
          }
  }
  sensors.setResolution(12);  // define resolução maxima
  Serial.println();

 */
 
  if(!flags.offline){
    client.setServer(mqtt_server, 1883);
    client.setCallback(callback);
  }

 lastMsg=millis()-T_ATUALIZA-1;
 flags.modo=0;      // normal
 flags.geladeira=false; //geladeira desligada
 flags.aquecedor=false; //aquecedor desligado
 flags.cooling=true;    // modo resfriar
}


void loop() {
  long newPosition = myEnc.read()/4;
  long rssi;
  int d_rssi;
  int varia=0;

/*  // Reset the counter
    myEnc.write(0);
  */

  ArduinoOTA.handle();
  if (!flags.offline){
    if (!client.connected()) {
    reconnect();
  }
  client.loop();
}
  float AjMin= ajuste-histerese/2;
  float AjMax= ajuste+histerese/2;
  
    
  if (millis() - lastMsg > T_ATUALIZA) {  // 1 vez por minuto
    lastMsg = millis();
     // Le Temperaturas e atua nas saidas
    sensors.requestTemperatures();
    float tempC = round(sensors.getTempC(Sferm1)*10)/10.0;
    float tempAmb =round(sensors.getTempC(Samb)*10)/10.0;
    float tempGel,tempF2;
    if (flags.gel) tempGel =round(sensors.getTempC(Sgel)*10)/10.0;
    if (flags.ferm2)tempF2 =round(sensors.getTempC(Sferm2)*10)/10.0;
    // compara temperaturas
    if(flags.cooling){   // modo gelar
      if (flags.geladeira) { // geladeira ligada
        if (tempC <=AjMin){  // temperatura atual menor que min
          flags.geladeira=false;
          digitalWrite(OutGeladeira, LOW ); // deliga geladeira
        }
      } else {
        if (tempC >=AjMax){  // temp atual maior que maximo
          flags.geladeira=true;
          digitalWrite(OutGeladeira, HIGH ); // deliga geladeira
        } else if(tempC<=(AjMin-histerese)) flags.cooling=false;  // abaixo do minimo muda pra modo aquecedor. considere histerese
      }
    } else {      // modo aquecer
      if (flags.aquecedor) { //aquecedor ligado
        if (tempC >=AjMax){
          flags.aquecedor=false;
          digitalWrite(OutAquecedor, LOW ); // deliga aquecedor
        }
      } else {
        if (tempC <=AjMin){
          flags.aquecedor=true;
          digitalWrite(OutAquecedor, HIGH ); // liga aquecedor
        } else if(tempC>=(AjMax+histerese)) flags.cooling=true;  // abaixo do minimo muda pra modo aquecedor. considere histerese
      }
     }
     // Mostra dados no serial monitor e prepara display
     Serial.print("Temp Amb: ");
     Serial.print(tempAmb);
     formattemp(temp[2],tempAmb,"A: ","");
     //Serial.print(temp[2]);
     //sprintf(temp[2], "A: %d.%01d", (int)tempAmb, decround(tempAmb,1));
     Serial.print(" Temp Ferm1: ");
     Serial.print(tempC);
     
     formattemp(temp[0],tempC,"","°C");
     //sprintf(temp[0], "%d.%01d°C", (int)tempC, decround(tempC,1));
     if(flags.gel){
       Serial.print(" Temp Interna: ");
       Serial.print(tempGel);
       formattemp(temp[3],tempGel,"I: ","");
       //sprintf(temp[3], "I: %d.%01d", (int)tempGel, decround(tempGel,1));
     } else strcpy(temp[3],"I: ____");
     if(flags.ferm2){
       Serial.print(" Temp Ferm2: ");
       Serial.print(tempF2);
       formattemp(temp[4],tempF2,"2: ","");
       //sprintf(temp[4], "2: %d.%01d", (int)tempF2, decround(tempF2,1));
     } else strcpy(temp[4],"2: ____");
     Serial.print(" Ajuste : ");
     Serial.println(ajuste);
     formattemp(temp[1],ajuste,"","°C");

     //for (int i=0;i<5;i++)  Serial.println(temp[i]);
     
     //sprintf(temp[1], "%d.%01d°C", (int)ajuste, decround(ajuste,1));
     Serial.print(" Modo : ");
     if (flags.cooling) Serial.print("Refrigera ");
      else Serial.print("Aquece    ");
     Serial.print(" Geladeira : ");
     Serial.print(flags.geladeira);
     Serial.print(" Aquecedor : ");
     Serial.println(flags.aquecedor);
    // update display
     rssi = 0;
     if(!flags.offline) rssi = WiFi.RSSI();
     d_rssi=aj_rssi(rssi);
     Serial.print("RSSI:");
     Serial.print(rssi);
     Serial.print("    Dispaly RSSI :");
     Serial.println(d_rssi);
     //Atualiza o display se não está no mod ajuste
     drawFrame(flags.modo,temp,d_rssi);
  

    // Se não está offline, publica a mensagem
    if (!flags.offline){
     publish(Fermentador1, tempC, Write);
     publish(Ajuste, ajuste, Write);
     publish(Ambiente, tempAmb, Write);
     publish(Interna, tempGel, Write);
     publishbool(Geladeira, flags.geladeira, Write);
     publishbool(Aquecedor,flags.aquecedor,Write);
     publishmode(Modo,flags.geladeira,flags.aquecedor,Write);
     Serial.println("Publish message");
    }
  }
  // Tratamento do botao
  if (flags.bt) { 
    if(flags.modo==0){ // prepare ajustes 
      beep(BCurto);  // beep curto 
      formattemp(temp[1],ajuste,"","°C");
      
      //sprintf(temp[1], "%d.%01d°C", (int)ajuste, decround(ajuste));
      

      Serial.println(temp[1]);

      drawFrame(true,temp,d_rssi); //prepara display
      flags.modo=1;// entra em ajuste
      lastAdj=millis();  // guarda a hora que entrou no ajuste
      // debug
      Serial.print("lastAdj");
      Serial.println(lastAdj);
      
      oldPosition = newPosition;  // zera o encoder
      Serial.println("Entando modo ajuste");
    } else if(flags.modo==1){  // sai do modo ajuste
      beep(BLongo);  // beep longo (
      flags.modo=0;  //sai do ajuste e 
      drawFrame(false,temp,d_rssi); //atualiza display
      lastMsg=millis()-T_ATUALIZA-1; // forca update
      if (!flags.offline){
        publish(AjusteIn, ajuste, Write);  // publica a nova temperatura ajustada
      }
      Serial.println("Saindo modo ajuste");
    } 
    flags.bt=false; // limpa botao
  }
  //Tratamento do modo
  if(flags.modo==1) {
    if (newPosition != oldPosition) {
      varia = newPosition-oldPosition;
      ajuste=ajuste + varia*0.1;
      
 //     ajuste=ajuste +((newPosition-oldPosition)*0.1);
  //  ajuste= ((int) ajuste*10)/10;
      formattemp(temp[1],ajuste,"","°C");
      //sprintf(temp[1], "%d.%01d°C", (int)ajuste, (int)((ajuste)*10)%10);
      display.setColor(WHITE);
      display.setTextAlignment(TEXT_ALIGN_CENTER);
      display.setFont(ArialMT_Plain_24);
      display.fillRect(44,25,83,26);
      display.setColor(BLACK); 
      display.drawString(86, 25, temp[1]);
      display.display();
      Serial.print("posição do encoder :");
      Serial.println(newPosition);
      Serial.print("Ajuste :");
      Serial.println(ajuste);
      oldPosition = newPosition;
      lastAdj=millis();
     
    }
    if (millis() - lastAdj > T_AJUSTE){  // 10 segundos de espera
  // debug    
   /*   Serial.print("now: ");
      Serial.println(now);
      Serial.print("lastAdj ");
      Serial.println(lastAdj);
      Serial.print("now - lastAdj ");
      Serial.println(now - lastAdj);
      */
      beep(BLongo);  // beep longo (100 ms)
      flags.modo=0;  //sai do ajuste e 
      lastMsg=millis()-T_ATUALIZA-1; // forca update
      if (!flags.offline){
        publish(AjusteIn, ajuste, Write);  // publica a nova temperatura ajustada
      }
      Serial.println("Saindo modo ajuste - time out");

  }
 }
 if(flags.modo==0&&flags.offline&&(millis()-lastConn > T_RECONN)){
  Serial.println("Tentando conectar novamente");
  flags.offline=false;  // marca como conectado, caso esteja connectado, mas sem acesso ao servidor
  if (WiFi.status()!=WL_CONNECTED) {
    setup_wifi(); // se nao conectado , tenta conectar
  }
 }
 
}

void setup_wifi() {

  int f=0;
  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(WiFi.SSID());
  //Serial.println(WiFi.psk());


  WiFi.mode(WIFI_STA);
  WiFi.begin();
  while ((WiFi.status() != WL_CONNECTED)&&f++<10) {
    delay(500);
    Serial.print(".");
  }
  flags.offline=false;
  if(f>10) {
    flags.offline=true;
    lastConn=millis();
  }
  
  randomSeed(micros());

  Serial.println("");
  if (!flags.offline){
    Serial.println("WiFi connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
  } else Serial.println("Falha conexão WiFI - Modo Offline");
}

void callback(char* topic, byte* payload, unsigned int length) {
  
  float value;
 
  payload[length] = '\0';
  String message = (char*)payload;

  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
//  for (int i = 0; i < length; i++) {
    Serial.println(message);
 // }
 // Serial.println();
   value = message.toFloat();
  
  if (value>=TEMPMIN && value<=TEMPMAX) ajuste = value;
  
  Serial.print("Temperatura Ajuste : ");
  Serial.println(ajuste);
  lastMsg=millis()-T_ATUALIZA-1; // forca update
}

void reconnect() {
  // Loop until we're reconnected
  
  int tent=1;
   char topic[64];
   sprintf(topic, "%s/%s", Channel, Availability);
   while (!client.connected()&&tent<6) {
    Serial.print("Tentando conectar MQTT server : ");
    Serial.println(mqtt_server);
    Serial.print("User : ");
    Serial.println(user);
    // Create a random client ID
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (client.connect(clientId.c_str(), user, pass,topic,2,true,"offline")) {      // tenta connectar e seta LWT (last will and testment, QoS 2 e retain)
      Serial.println("connected");
       // ... a resubscribe
      publishOnline(Availability,Write);
      sprintf(topic, "%s/%s", Channel, AjusteIn);
      client.subscribe(topic);
      
    } else {
      Serial.print("falha, rc=");
      Serial.print(client.state());
      Serial.print(" Tentativa: ");
      Serial.print(tent);
      Serial.println(" tentando novamente em 5 segundos");
      // Wait 5 seconds before retrying
      tent++;
      delay(5000);
    }
  }
  if (tent>=6){
    flags.offline=true;
    lastConn=millis();
  }
}

void publish(const char* resource, float data, bool persist){
  char buffer[128];
  
  formattemp(buffer,data,"","");
  //sprintf(buffer, "%d.%01d", (int)data, decround(data,1));
  
  // Create the topic to publish to
  char topic[64];
  sprintf(topic, "%s/%s", Channel, resource);
  
//Serial.println(buffer);
  // Now publish the char buffer to Beebotte

//  Serial.print("Publica:  ");
//  Serial.println(buffer); 
  
  client.publish(topic, buffer,persist);
  
}

void publishmode(const char* resource,  bool gela, bool aquece, bool persist){

  
   // Create the topic to publish to
  char topic[64];
  sprintf(topic, "%s/%s", Channel, resource);

  if(gela&&!aquece) client.publish(topic,"cool",persist);
  if (aquece&&!gela) client.publish(topic,"heat",persist);
  if (!aquece&&!gela) client.publish(topic,"off",persist);
  
}

void publishbool(const char* resource, bool data, bool persist){
  

  // Create the topic to publish to
  char topic[64];
  sprintf(topic, "%s/%s", Channel, resource);
  
  client.publish(topic, data ? "1" :"0", persist);
  
}

void publishOnline(const char* resource, bool persist) {
  char topic[64];
  sprintf(topic, "%s/%s", Channel, Availability);
  client.publish(topic,"online",persist);  // avisa que est['a available
  Serial.print("Publica:  ");
  Serial.print(topic);
  Serial.print("/");
  Serial.println("online");
}

void mostra_endereco_sensor(DeviceAddress deviceAddress){
  for (uint8_t i = 0; i < 8; i++)
  {
    // Adiciona zeros se necessário
    if (deviceAddress[i] < 16) Serial.print("0");
    Serial.print(deviceAddress[i], HEX);
  }
}

void formattemp(char * str,float val,char * prefix,char * sufix){
  // assume que o buffter tem espaco

  strcpy(str,prefix);  // comeca com o prefixo
  str+=strlen(prefix); // move o ponteiro pra onde precisa por os dados
  if (val<0) {         // se for negativo addiciona o sinal e faz o valor positivo
    str[0]='-';
    str++;
    str[0]='/0'; // recoloca o terminador
    val=val*-1.0; // abs parce nao funcionar com float 
  }
  val+= 0.05;  //garante o arredondamento
  sprintf(str, "%d.%01d", (int) val, (int)((val)*10)%10);  // formata a string
//  sprintf(str, "%d.%01d", (int) val, (int) ((val+.05)*10)%10);  // formata a string
  strcat(str,sufix);  // adiciona o sufixo
  return;
}

/* desenha o indicador de intensidade com barras coluna, linha e valor
  usa um quadrado de 9 x 8 pizels 
  4 intyensidades 0 a 3
*/



/*void drawImageDemo() {
    // see http://blog.squix.org/2015/05/esp8266-nodemcu-how-to-create-xbm.html
    // on how to create xbm files
    display.drawXbm(34, 14, WiFi_Logo_width, WiFi_Logo_height, WiFi_Logo_bits);
}*/

void drawFrame(bool type,char temps[5][8],int rssi) {  // type = true => ajuste de temp, false => modo normal
   
    display.clear();
    display.setColor(WHITE);
    display.drawRect(0,0,DISPLAY_WIDTH,DISPLAY_HEIGHT); // moldura
    display.drawLine(44, 0, 44, DISPLAY_HEIGHT);   // separador das temperaturas
//    display.drawLine(0, DISPLAY_HEIGHT-16, DISPLAY_WIDTH,DISPLAY_HEIGHT-16);
    display.drawLine(0,16,DISPLAY_WIDTH,16);  // linha de status
    display.setTextAlignment(TEXT_ALIGN_CENTER);
    display.setFont(ArialMT_Plain_24);
    if (!type) {
      display.drawString(86, 25, temps[0]);
      display.setFont(ArialMT_Plain_10);
      display.drawString(22, 2, temps[1]);
      //    if(flags.geladeira) display.drawXbm(45,2,16,16,cool);
      if(flags.geladeira) display.drawXbm(68,2,16,16,cool);
      display.drawXbm(94,2,16,16,cloud);
      drawRSSI(116,3,rssi);

  
    drawbat(48,5,3);
    } else {
      display.fillRect(44,25,83,26);
      display.fillRect(0,0,44,16);
      //    getStringWidth()
      display.setColor(BLACK); 
      display.drawString(86, 25, temps[1]);
      display.setFont(ArialMT_Plain_10);
      display.drawString(22, 2, temps[0]);
      display.setColor(WHITE);
      display.drawString(80,2,"AJUSTE");
    }
    display.drawString(22, 20, temps[2]);
    display.drawString(22, 32, temps[3]);
    display.drawString(22, 44, temps[4]);
    display.setTextAlignment(TEXT_ALIGN_LEFT);
//    display.drawString(0, 53, "Conectado");
        
    
    display.display();

}

void drawRSSI(int c,int l,int valor) {  

 if(valor==0){
  //desenha X
  display.drawLine(c+2,l+2,c+7,l+9);
  display.drawLine(c+2,l+9,c+7,l+2);
  
 } else {
  // barra pequena
  display.drawLine(c+2,l+7,c+2,l+9);
  display.drawLine(c+1,l+7,c+1,l+9);  
 }
 if(valor>1) {
  // barra media
   display.drawLine(c+5,l+4,c+5,l+9);
  display.drawLine(c+4,l+4,c+4,l+9);
 }
 if (valor>2){
 // barra grande
  display.drawLine(c+8,l,c+8,l+9);
  display.drawLine(c+7,l,c+7,l+9);
 }
}
void drawbat(int c,int l,int valor)
{
   // desenha indicador de bateria
  display.drawHorizontalLine(c+1,l,14);  // superior
  display.drawHorizontalLine(c+1,l+7,14); // inferior
  //u8g.drawHLine(batx,baty+2,1);  //ligacao do tip
  //u8g.drawHLine(batx,baty+5,1); //ligacao do tip
  display.drawVerticalLine(c,l+2,4);  // tip
  display.drawVerticalLine(c+1,l+1,2);  // frente cima
  display.drawVerticalLine(c+1,l+5,2); // frente baixo
  display.drawVerticalLine(c+15,l,8);  // fundo
  // carga da bateria
  if(valor>0) display.fillRect(c+11,l+2,3,4);
  if(valor>1) display.fillRect(c+7,l+2,3,4);
  if(valor>2) display.fillRect(c+3,l+2,3,4);
 }

int aj_rssi(long r){
  if (r==0) return 0;
  if (r<=-85) return 1;
  if (r<=-65) return 2;
  return 3;
}

void beep_off(){
  digitalWrite(OutBeep,LOW);
  beeper.detach();
}

void beep(int duracao){
  digitalWrite(OutBeep,HIGH);
  beeper.attach_ms(duracao,beep_off);
}

void button() {
  if (!digitalRead(Button)) {
    bt_count++;
  } 
  else {
    if (bt_count > 1 && bt_count <= 40) {   
      Serial.println("botao");
      flags.bt=true;   //entra ajustes
      }
    bt_count=0;
    }
  if (bt_count >40){
      Serial.println("entrando config"); 
 //     flags.config = true;
      bt_count=0;
  }
}

void readepp_mqttpar() {
 
  for (uint8_t i = 0; i < ServerSize; i++) mqtt_server[i]=EEPROM.read(EPPserver+i);
  for (uint8_t i = 0; i < UserSize; i++) user[i]=EEPROM.read(EPPuser+i);
  for (uint8_t i = 0; i < PassSize; i++) pass[i]=EEPROM.read(EPPpass+i);
}
void writeepp_mqttpar() {
 
  for (uint8_t i = 0; i < ServerSize; i++) EEPROM.write(EPPserver+i,mqtt_server[i]);
  for (uint8_t i = 0; i < UserSize; i++) EEPROM.write(EPPuser+i,user[i]);
  for (uint8_t i = 0; i < PassSize; i++) EEPROM.write(EPPpass+i,pass[i]);
  EEPROM.commit();
  
}

bool comp_end(DeviceAddress a, DeviceAddress b){
  if(memcmp(a,b,8)==0) return true;
    else return false;
}

bool copy_end(DeviceAddress a, DeviceAddress b){ // copies b into a
  if(memcpy(a,b,8)==0) return true;
    else return false;
}

void readepp_sens(int address, DeviceAddress deviceAddress) {
 
  for (uint8_t i = 0; i < 8; i++) deviceAddress[i]=EEPROM.read(address+i);
}

void writeepp_sens(int address, DeviceAddress deviceAddress) {
 
  for (uint8_t i = 0; i < 8; i++) EEPROM.write(address+i,deviceAddress[i]);
  EEPROM.commit();
}

bool readepp_sig(void){
   if (EEPROM.read(EPPsig)==0xaa)
    return true;
   else
    return false;
}

void writeepp_sig(void){   // Nao faz o commit.
   EEPROM.write(EPPsig,0xaa);
}

void saveConfigCallback () {
  Serial.println("Precisa salvar configuração");
  flags.savepar=true;
}
