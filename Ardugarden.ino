#include "WiFiS3.h"
#include <Wire.h>
#include <DS3231.h>
///////please enter your sensitive data in the Secret tab/arduino_secrets.h
char ssid[] = "";        // your network SSID (name)
char pass[] = "";    // your network password (use for WPA, or use as key for WEP)
int keyIndex = 0;                 // your network key index number (needed only for WEP)
DS3231 rtc;

unsigned long seconds = 1000L;
unsigned long minutes = seconds * 60;
int pump = 12;
bool pump_status = 0;
int dht11 = 5;
int led_strip = 8;
bool led_manual = 0;
bool led_strip_status = 0;
bool h12Flag;
bool pmFlag;
bool century = false;
int led =  LED_BUILTIN;
int status = WL_IDLE_STATUS;

unsigned long active = 5 * minutes;   // 5 min
unsigned long sleeping = 40 * minutes; // 20 min
unsigned long nextLedActivationTime = 0;  // Tempo dell'ultima attivazione

unsigned long lastActivationTime = 0;  // Tempo dell'ultima attivazione
unsigned long lastDeactivationTime = 5*minutes+1; // Tempo dell'ultima disattivazione
unsigned long uptime = 0; // Tempo di uptime (in millisecondi)

bool clientConnected = false;
unsigned long connectionStartTime = 0;
unsigned long connectionTimeout = 3000; // Timeout per la connessione (in millisecondi)

WiFiServer server(80);


void sendResponse(WiFiClient client) {
  client.println("HTTP/1.1 200 OK");
  client.println("Content-Type: text/html");
  client.println("Connection: close");
  client.println();

  client.println("<!DOCTYPE html>");
  client.println("<html>");
  client.println("<head>");
  client.println("<title>ArduGarden</title>");
  client.println("<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">");
  client.println("<style>");
  client.println("body {");
  client.println("  font-family: Arial, sans-serif;");
  client.println("  text-align: center;");
  client.println("  background-color: #f4f4f4;");
  client.println("  display: flex;");
  client.println("  min-height: 100vh;");
  client.println("  flex-direction: column;");
  client.println("}");
  client.println(".container {");
  client.println("  flex: 1;");
  client.println("  max-width: 600px;");
  client.println("  margin: 0 auto;");
  client.println("  padding: 20px;");
  client.println("  background-color: #fff;");
  client.println("  border-radius: 10px;");
  client.println("  box-shadow: 0 0 10px rgba(0, 0, 0, 0.1);");
  client.println("}");
  client.println(".button {");
  client.println("  display: inline-block;");
  client.println("  padding: 10px 20px;");
  client.println("  background-color: #4caf50;");
  client.println("  color: #fff;");
  client.println("  text-decoration: none;");
  client.println("  border-radius: 5px;");
  client.println("  margin: 10px;");
  client.println("  transition: background-color 0.3s;");
  client.println("}");
  client.println(".button:hover {");
  client.println("  background-color: #45a049;");
  client.println("}");
  client.println(".footer {");
  client.println("  background-color: #333;");
  client.println("  color: #fff;");
  client.println("  padding: 10px 0;");
  client.println("  position: fixed;");
  client.println("  bottom: 0;");
  client.println("  width: 100%;");
  client.println("}");
  client.println("</style>");
  client.println("</head>");
  client.println("<body>");
  client.println("<div class=\"container\">");
  client.println("<h1 style=\"color: #4caf50;\">ArduGarden</h1>");
  
  // Displaying date and time in the required format
  client.print("<p style=\"font-size: 20px;\">Data e Ora: ");
  if (rtc.getDate() < 10) {
    client.print("0"); // Adding leading zero for minutes less than 10
  }
  client.print(rtc.getDate(), DEC);
  
  client.print("/");
  if (rtc.getMonth(century) < 10) {
    client.print("0"); // Adding leading zero for minutes less than 10
  }
  client.print(rtc.getMonth(century), DEC);
  client.print("/");
  client.print(rtc.getYear(), DEC);
  client.print(" ");
  
  
  if (rtc.getHour(h12Flag, pmFlag) < 10) {
    client.print("0"); // Adding leading zero for minutes less than 10
  }
  client.print(rtc.getHour(h12Flag, pmFlag), DEC);
  
  client.print(":");
  if (rtc.getMinute() < 10) {
    client.print("0"); // Adding leading zero for minutes less than 10
  }
  client.print(rtc.getMinute(), DEC);
  client.println("</p>");

  // Displaying remaining time for the pump
  unsigned long remainingPumpTime = pump_status ? (active - (millis() - lastActivationTime)) / 1000 : (sleeping - (millis() - lastDeactivationTime)) / 1000;
  int pumpMinutes = remainingPumpTime / 60;
  int pumpSeconds = remainingPumpTime % 60;
  client.print("<p>Tempo rimanente per la pompa: ");
  client.print(pumpMinutes);
  client.print(":");
  client.print(pumpSeconds);
  client.println("</p>");

  // Button to activate the pump
  if (pump_status == 0) {
    client.println("<a class=\"button\" href=\"/activate\">Accendi Pompa</a>");
  } else {
    client.println("<a class=\"button\" href=\"/deactivate\">Spegni Pompa</a>");
  }

  // Button to control the LED strip
  /*if (led_strip_status == 0) {
    client.println("<a class=\"button\" href=\"/led_on\">Accendi Luci</a>");
  } else {
    client.println("<a class=\"button\" href=\"/led_off\">Spegni Luci</a>");
  }*/

  client.println("</div>");  
  client.println("<div class=\"footer\">Thank's to Davide :)</div>");

  client.println("</body>");
  client.println("</html>");
}


void activatePump() {
  lastActivationTime = millis();
  digitalWrite(pump, HIGH); // Accendi la pompa
  digitalWrite(led, HIGH);
  pump_status = 1;
}

void deactivatePump() {
  lastDeactivationTime = millis();
  digitalWrite(pump, LOW); // Spegni la pompa
  digitalWrite(led, LOW);
  pump_status = 0;
}

void activateLed(){
  digitalWrite(led_strip, HIGH);
  led_strip_status = 1;
  nextLedActivationTime = 0;
}

void deactivateLed(){
  nextLedActivationTime = millis() + 30 * minutes;
  led_strip_status = 0;
  digitalWrite(led_strip, LOW);
}

void printWifiStatus() {
  // print the SSID of the network you're attached to:
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  // print your board's IP address:
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);

  // print the received signal strength:
  long rssi = WiFi.RSSI();
  Serial.print("signal strength (RSSI):");
  Serial.print(rssi);
  Serial.println(" dBm");
  // print where to go in a browser:
  Serial.print("To see this page in action, open a browser to http://");
  Serial.println(ip);
}


void setup() {
  Serial.begin(9600);      // initialize serial communication
  pinMode(led, OUTPUT);      // set the LED pin mode
  pinMode(led_strip, OUTPUT);
  pinMode(pump, OUTPUT);
  Wire.begin();
  digitalWrite(led_strip, LOW);
  digitalWrite(pump, LOW);
  // check for the WiFi module:
  if (WiFi.status() == WL_NO_MODULE) {
    Serial.println("Communication with WiFi module failed!");
    // don't continue
    while (true);
  }

  String fv = WiFi.firmwareVersion();
  if (fv < WIFI_FIRMWARE_LATEST_VERSION) {
    Serial.println("Please upgrade the firmware");
  }

  // attempt to connect to WiFi network:
  while (status != WL_CONNECTED) {
    Serial.print("Attempting to connect to Network named: ");
    Serial.println(ssid);                   // print the network name (SSID);

    // Connect to WPA/WPA2 network. Change this line if using open or WEP network:
    status = WiFi.begin(ssid, pass);
    // wait 10 seconds for connection:
    delay(10000);
  }
  server.begin();                           // start the web server on port 80
  printWifiStatus();                        // you're connected now, so print out the status
}


void loop() {
  WiFiClient client = server.available();   // listen for incoming clients
  
  if (client) {                            // if you get a client,
    Serial.println("new client");           // print a message out the serial port
    String currentLine = "";                // make a String to hold incoming data from the client
    while (client.connected()) {            // loop while the client's connected
      if (client.available()) {             // if there's bytes to read from the client,
        char c = client.read();             // read a byte, then
        Serial.write(c);                    // print it out to the serial monitor
        if (c == '\n') {                    // if the byte is a newline character

          // if the current line is blank, you got two newline characters in a row.
          // that's the end of the client HTTP request, so send a response:
          if (currentLine.length() == 0) {
            sendResponse(client);
            break;
          } else {    // if you got a newline, then clear currentLine:
            currentLine = "";
          }
        } else if (c != '\r') {  // if you got anything else but a carriage return character,
          currentLine += c;      // add it to the end of the currentLine
        }

        // Check to see if the client request was "GET /H" or "GET /L":
        if (currentLine.endsWith("GET /activate")) {
          activatePump();
        }
        else if (currentLine.endsWith("GET /deactivate")) {
          deactivatePump();                
        }
        else if (currentLine.endsWith("GET /led_on")) {
          Serial.println("Accendo Led");
          activateLed();
          led_manual = 1;
        }
        else if (currentLine.endsWith("GET /led_off")) {
          Serial.println("Spengo Led");
          deactivateLed();
          led_manual = 1;                
        }
      }
      
    }
    // close the connection:
    client.stop();
    Serial.println("client disconnected");
  } else {
    unsigned long connectionDuration = millis() - connectionStartTime;

    if (connectionDuration >= connectionTimeout) {
      clientConnected = false;
      //return; THIS SHOULDNT BE REMOVED BUT IF SET ALL THE REST LOOP DOESNT PERFORM
    }

    WiFiClient client = server.available();

    if (client) {
      Serial.println("Closed");
      client.stop();
      clientConnected = false;
    }
  }
  unsigned long currentMillis = millis();
  int currentHour = rtc.getHour(h12Flag, pmFlag); 
  if (led_manual){
    if (currentMillis > nextLedActivationTime){
        activateLed();
        led_manual = 0;
      }
    else{
      deactivateLed();
    }
  }
  if (!led_manual){
    if ((currentHour >= 8 && currentHour < 21)) {
      activateLed();
    } 
    else {
      deactivateLed();
    }
  }
  if (pump_status == 0 && currentMillis - lastDeactivationTime >= sleeping) {
    activatePump();
  }

  if (pump_status == 1 && currentMillis - lastActivationTime >= active) {
    deactivatePump();
  }
}