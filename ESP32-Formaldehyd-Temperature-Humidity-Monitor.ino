#include <SensirionCore.h>
#include <WebServer.h>
#include <ESPmDNS.h>
#include <SensirionI2CSfa3x.h>

SensirionI2CSfa3x sfa3x;

const char *ssid = "Studienprojekt_demo";
const char *password = "ESP32_2024_demo";

WebServer server(80);

void handleRoot() {
  char msg[6000];
  
    float hcho = SFA_Aldehyde();
    float temp = SFA_Temperature();
    float hum = SFA_Humidity();

    const char* tempColor;
    const char* tempTextColor;
    if (temp < 18) {
    tempColor = "#FFEB3B";  // Gelb
    tempTextColor = "black"; // Schwarz
    } else if (temp <= 26) {  // 18 bis 25 
        tempColor = "#4CAF50";  // Grün
        tempTextColor = "white";
    } else if (temp <= 31) {  // 26.01 bis 31
        tempColor = "#FFEB3B";  // Gelb
        tempTextColor = "black";
    } else {  // 31.01 Grad und darüber
        tempColor = "#F44336";  // ROT
        tempTextColor = "white";
    }
    const char* humColor = (hum >= 0 && hum < 61) ? "#4CAF50" : (hum >= 61 && hum <= 80) ? "#FFEB3B" : "#F44336";
    const char* humTextColor = (hum >= 61 && hum < 80) ? "black" : "white";
    const char* hchoColor = (hcho <= 0.1) ? "#4CAF50" : "#F44336";
    const char* hchoTextColor = (hcho <= 0.1) ? "white" : "white";

    snprintf(msg, 6000,
             "<html lang='de'>\
    <head>\
      <meta charset='UTF-8'>\
      <meta http-equiv='refresh' content='4'/>\
      <meta name='viewport' content='width=device-width, initial-scale=1.0'>\
      <link rel='stylesheet' href='https://use.fontawesome.com/releases/v5.7.2/css/all.css' integrity='sha384-fnmOCqbTlWIlj8LyTjo7mOUStjsKC4pOpQbqyi7RrhN7udi9RwhKkMHpvLbHG9Sr' crossorigin='anonymous'>\
      <title>ESP32 Webserver mit SFA30</title>\
      <style>\
      html { font-family: Arial; display: inline-block; margin: 20px auto; text-align: center;}\
      h2 { font-size: 3.0rem; }\
      p { font-size: 3.0rem; }\
      footer { margin-top: auto; color: #333; text-align: center; padding: 12px 0; font-size: 12px;}\
      .logo1 {color: #003399; font-size: 29px; text-align: left; }\
      .logo2 {color: #003399; font-size: 35px; font-weight: bold; text-align: left; }\
      .subtext {color: #cc0000; font-size: 20px; text-align: left; }\
      img {width: 100%; max-width: 300px; height: auto; }\
      .units { font-size: 1.5rem; }\
      .esp32-labels{ font-size: 2.3rem; vertical-align:middle; padding-bottom: 15px;}\
      .value { padding: 5px; border-radius: 5px; color: %s; background-color: %s; }\
      </style>\
    </head>\
    <body>\
      <div class='logo1'>HOCHSCHULE </div>\
      <div class='logo2'>ESSLINGEN </div>\
      <div class='subtext'>Informatik und <br/> Informationstechnik</div>\
      <h2>Studienprojekt <br/> ESP32 Webserver mit SFA30</h2>\
        <p>\
          <img src='https://upload.wikimedia.org/wikipedia/commons/0/0f/Formaldehyde-2D.png' alt='Aldehyd Icon'>\
          <span class='esp32-labels'>Formaldehyde</span>\
          <span class='value' style='color: %s; background-color: %s;'>%.3f ppm</span>\
        </p>\
        <p>\
          <i class='fas fa-thermometer-half' style='color:#ca3517;'></i>\
          <span class='esp32-labels'>Temperatur</span>\
          <span class='value' style='color: %s; background-color: %s;'>%.2f°C</span>\
        </p>\
        <p>\
          <i class='fas fa-tint' style='color:#00add6;'></i>\
          <span class='esp32-labels'>Luftfeuchtigkeit</span>\
          <span class='value' style='color: %s; background-color: %s;'>%.2f%%</span>\
        </p>\
        <footer>\
        von Halit Osman Efkere\
        </footer>\
    </body>\
</html>", tempTextColor, tempColor, hchoTextColor, hchoColor, hcho, tempTextColor, tempColor, temp, humTextColor, humColor, hum);
    server.send(200, "text/html", msg);
}


void setup(void) {

  Serial.begin(115200);

  pinMode(2, OUTPUT);  

  while (!Serial) {
    delay(100);
  }

  Wire.begin();

  uint16_t error;
  char errorMessage[256];

  sfa3x.begin(Wire);

  // Start Measurement
  error = sfa3x.startContinuousMeasurement();
  if (error) {
  Serial.print("Fehler beim Ausführen von startContinuousMeasurement(): ");
  errorToString(error, errorMessage, 256);
  Serial.println(errorMessage);
  blinking_sensor_error();
  }

  
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.println("");

  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    blinking_network_error();
    Serial.print(".");
  }

  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  blinking_connected_network();

  /*http://esp32.local/ as
  set the hostname to "esp32.local"
  Initialize multicast DNS*/
  if (MDNS.begin("esp32")) {
    Serial.println("MDNS responder started");
  }
  server.on("/", handleRoot);

  server.begin();
  Serial.println("HTTP server started");
}

void loop(void) {
  server.handleClient();
  delay(2); 
  blinking_client();

}

float SFA_Aldehyde(){
  int16_t hcho;
  int16_t humidity;
  int16_t temperature;
  uint16_t error;


  error=sfa3x.readMeasuredValues(hcho, humidity, temperature);
  if(error) {
    return 0;
  } 
  else {
    return ((hcho/5.0)/1000); /*ppb_to_ppm*/
    }
}

float SFA_Humidity(){
  int16_t hcho;
  int16_t humidity;
  int16_t temperature;
    uint16_t error;


  error=sfa3x.readMeasuredValues(hcho, humidity, temperature);
  if(error) {
    return 0;
  } 
  else {
    return humidity/100.0;
    }
}

float SFA_Temperature(){
  int16_t hcho;
  int16_t humidity;
  int16_t temperature;
  uint16_t error;

  error=sfa3x.readMeasuredValues(hcho, humidity, temperature);
  if(error) {
    return 0;
  } 
  else {
    return temperature/200.0;
    }
}

void blinking_sensor_error(){
    for(int i = 0; i < 10; i++ ){
          digitalWrite(2, HIGH);               
          delay(1000);
          digitalWrite(2, LOW);               
          delay(1000); 
        }
}

void blinking_network_error(){
      digitalWrite(2, HIGH);               
      delay(100);
      digitalWrite(2, LOW);               
      delay(100); 
}

void blinking_client(){   
      digitalWrite(2, HIGH);               
      delay(1000);
      digitalWrite(2, LOW);               
      delay(3000);
}

void blinking_connected_network(){
      digitalWrite(2, HIGH);               
      delay(2000);
      digitalWrite(2, LOW);               
      delay(200);
      digitalWrite(2, HIGH);
      delay(200);
      digitalWrite(2, LOW);
      delay(200);
      digitalWrite(2, HIGH);
      delay(200);
      digitalWrite(2, LOW);
      delay(200);
      digitalWrite(2, HIGH);
      delay(200);
      digitalWrite(2, LOW);
      delay(200);
      digitalWrite(2, HIGH);
      delay(200);
      digitalWrite(2, LOW);
      delay(200);
      digitalWrite(2, HIGH);
      delay(200);
      digitalWrite(2, LOW);
      delay(200);
      digitalWrite(2, HIGH);
      delay(200);
      digitalWrite(2, LOW);
      delay(200);
      digitalWrite(2, HIGH);
      delay(1000);
      digitalWrite(2, LOW);  
}