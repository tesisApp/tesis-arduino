#define TINY_GSM_MODEM_SIM808
//#define TINY_GSM_RX_BUFFER 256

#include <SoftwareSerial.h>
#include <TinyGsmClient.h> //httpsgithub.comvshymanskyyTinyGSM
#include <ArduinoHttpClient.h> //httpsgithub.comarduino-librariesArduinoHttpClient

//Seleccion de los pines 7 como Rx y 8 como Tx
SoftwareSerial SIM808(7, 8);

#define DEBUG true
String latitude,longitude;
String data[5];

const char FIREBASE_HOST[]  = "tesisarduinogps-default-rtdb.firebaseio.com";
const String FIREBASE_AUTH  = "w3LGXdduvaYMg7uqsvLK4bsHTHtpghzVDCCmtOem";
const String FIREBASE_PATH  = "location";
const int SSL_PORT          = 443;
 
char apn[]  = "internet.movil";
char user[] = "internet";
char pass[] = "internet";

TinyGsm modem(SIM808);
 
TinyGsmClientSecure gsm_client_secure_modem(modem, 0);
HttpClient http_client = HttpClient(gsm_client_secure_modem, FIREBASE_HOST, SSL_PORT);

unsigned long previousMillis = 0;

void setup()
{
  SIM808.begin(19200);
  Serial.begin(19200);
  delay(100);
  Serial.println("Initializing modem...");
  //modem.restart();
  String modemInfo = modem.getModemInfo();
  Serial.print("Modem: ");
  Serial.println(modemInfo);
 
  http_client.setHttpResponseTimeout(10 * 1000); //^0 secs timeout
}

void loop()
{
  Serial.print(F("Connecting to "));
  Serial.print(apn);
  if (!modem.gprsConnect(apn, user, pass))
  {
    Serial.println(" fail");
    delay(1000);
    return;
  }
  Serial.println(" OK");
 
  http_client.connect(FIREBASE_HOST, SSL_PORT);
 
  while (true) {
    if (!http_client.connected())
    {
      Serial.println();
      http_client.stop();// Shutdown
      Serial.println("HTTP  not connect");
      break;
    }
    else
    {
      dht_loop();
    }
 
  }
 
}

void dht_loop()
{
  
  /*la= la + 2;
  lo = lo + 5;
  String h = String(la);
  String t = String(lo);
  delay(100);*/

 

 
  


  clearData();
  sendTabData("AT+CGNSINF",1000,DEBUG); //,DEBUG

  Serial.print("Latitud = ");
  Serial.print(latitude);
  Serial.println(" °ºS");
  Serial.print("Longitud = ");
  Serial.print(longitude);
  Serial.println(" ºW");
  String Data = "{";
  Data += "\"latitude\":" + latitude + ",";
 
  Data += "\"longitude\":" + longitude + "";
  Data += "}";
  
  PostToFirebase("PATCH", FIREBASE_PATH, Data, &http_client);
 
 
}

void PostToFirebase(const char* method, const String & path , const String & data, HttpClient* http)
{
  String response;
  int statusCode = 0;
  http->connectionKeepAlive(); // Currently, this is needed for HTTPS
 
  String url;
  if (path[0] != '/')
  {
    url = "/";
  }
  url += path + ".json";
  url += "?auth=" + FIREBASE_AUTH;
  Serial.print("POST:");
  Serial.println(url);
  Serial.print("Data:");
  Serial.println(data);
 
  String contentType = "application/json";
  http->put(url, contentType, data);
  //Impresion de url, contentType y Data     DESPUES BORRAR
  Serial.print("URL: ");
  Serial.println(url);
  Serial.print("contentType: ");
  Serial.println(contentType);
  Serial.print("data: ");
  Serial.println(data);
  Serial.println();
 
  statusCode = http->responseStatusCode();
  Serial.print("Status code: ");
  Serial.println(statusCode);
  response = http->responseBody();
  Serial.print("Response: ");
  Serial.println(response);
 
  if (!http->connected())
  {
    Serial.println();
    http->stop();// Shutdown
    Serial.println("HTTP POST disconnected");
  }
 
}


void sendTabData(String command , const int timeout , boolean debug){

SIM808.println(command);
long int time = millis();
int i = 0;

while((time+timeout) > millis()){
while(SIM808.available()){
char c = SIM808.read();
if (c != ',') {
data[i] +=c;
delay(100);
} else {
i++;
}
if (i == 5) {
delay(100);
goto exitL;
}
}
}exitL:
if (debug) {
  latitude = data[3];
  longitude = data[4];
}
}

void clearData (){
  
  int i=0;
  while ( i < 5 ){
    data[i]="";
    i++;
  }
}
