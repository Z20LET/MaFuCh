#include <Wire.h> 
#include <LiquidCrystal_I2C.h>
#include "WiFi.h"
#include "ESPAsyncWebServer.h"
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <MQ135.h>

LiquidCrystal_I2C lcd(0x27,16,2);

DHT am2301(00,DHT21);

MQ135 mq135(39);

AsyncWebServer server(80);

float humidity, temp, ppm;

const char* ssid = "AutoFungi";

const char* password = "MaFuCh2023";

String readPPM() {
  ppm = mq135.getPPM();
  if (isnan(ppm)) {    
    Serial.println("Failed to read from MQ135 sensor!");
    return "--";
  }
  else {
    Serial.println(ppm*10);
    return String(ppm*10);
  }
}

String readDHTTemperature() {
  float t = am2301.readTemperature();
  if (isnan(t)) {    
    Serial.println("Failed to read from DHT sensor!");
    return "--";
  }
  else {
    Serial.println(t);
    return String(t);
  }
}

String readDHTHumidity() {
  float h = am2301.readHumidity();
  if (isnan(h)) {
    Serial.println("Failed to read from DHT sensor!");
    return "--";
  }
  else {
    Serial.println(h);
    return String(h);
  }
}

const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html>
<head>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <link rel="stylesheet" href="https://use.fontawesome.com/releases/v5.7.2/css/all.css" integrity="sha384-fnmOCqbTlWIlj8LyTjo7mOUStjsKC4pOpQbqyi7RrhN7udi9RwhKkMHpvLbHG9Sr" crossorigin="anonymous">
  <style>
    html {
     font-family: Arial;
     display: inline-block;
     margin: 0px auto;
     text-align: center;
    }
    h2 { font-size: 3.0rem; }
    p { font-size: 3.0rem; }
    .units { font-size: 1.2rem; }
    .dht-labels{
      font-size: 1.5rem;
      vertical-align:middle;
      padding-bottom: 15px;
    }
  </style>
</head>
<body>
  <h2>AutoFungi v1.1</h2>
  <p>
    <i class="fas fa-thermometer-half" style="color:#059e8a;"></i> 
    <span class="dht-labels">Temperature</span> 
    <span id="temperature">%TEMPERATURE%</span>
    <sup class="units">&deg;C</sup>
  </p>
  <p>
    <i class="fas fa-tint" style="color:#00add6;"></i> 
    <span class="dht-labels">Humidity</span>
    <span id="humidity">%HUMIDITY%</span>
    <sup class="units">&percnt;</sup>
  </p>
  <p>
    <i class="fas fa-smog" style="color:#5A5A5A;"></i> 
    <span class="dht-labels">CO2</span>
    <span id="ppm">%PPM%</span>
    <sup class="units">ppm</sup>
  </p>
</body>
<script>
setInterval(function ( ) {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("temperature").innerHTML = this.responseText;
    }
  };
  xhttp.open("GET", "/temperature", true);
  xhttp.send();
}, 5000 ) ;

setInterval(function ( ) {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("humidity").innerHTML = this.responseText;
    }
  };
  xhttp.open("GET", "/humidity", true);
  xhttp.send();
}, 5000 ) ;

setInterval(function ( ) {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("ppm").innerHTML = this.responseText;
    }
  };
  xhttp.open("GET", "/ppm", true);
  xhttp.send();
}, 5000 ) ;
</script>
</html>)rawliteral";

// Replaces placeholder with DHT values
String processor(const String& var){
  //Serial.println(var);
  if(var == "TEMPERATURE"){
    return readDHTTemperature();
  }
  else if(var == "HUMIDITY"){
    return readDHTHumidity();
  }
  else if(var == "PPM"){
    return readPPM();
  }
  return String();
}

void setup(){
  // Serial port for debugging purposes
  Serial.begin(115200);
  
  am2301.begin();
  lcd.init();
  lcd.backlight();
  lcd.setCursor(1,0);
  lcd.print("AutoFungi v1.1");
  
  
  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi..");
  }

  // Print ESP32 Local IP Address
  Serial.println(WiFi.localIP());
  delay(2000);
  lcd.setCursor(0,1);
  lcd.print(WiFi.localIP());
  delay(5000);

  // Route for root / web page
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/html", index_html, processor);
  });
  server.on("/temperature", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", readDHTTemperature().c_str());
  });
  server.on("/humidity", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", readDHTHumidity().c_str());
  });
  server.on("/ppm", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", readPPM().c_str());
  });

  // Start server
  server.begin();
}


void loop(){
  humidity = am2301.readHumidity();
  temp = am2301.readTemperature();
  ppm = mq135.getPPM();

  lcd.clear();
  lcd.setCursor(7,0);
  lcd.print(ppm*10);
  lcd.print("ppm");
  lcd.setCursor(0,1);
  lcd.print(temp);
  lcd.write(223);
  lcd.print("C");
  lcd.setCursor(10,1);
  lcd.print(humidity);
  lcd.print("%");
  delay(2000);
}
