#include <ESP8266WiFi.h>

const char* ssid = "Redmi Note 13";
const char* password = "z4j23yq93524rgm";

WiFiServer server(80);

String header;
String output5State = "off";
String output4State = "off";
String buttonMessage = "";

const int output5 = 5;
const int output4 = 4;
const int buttonPin = D5;

unsigned long currentTime = millis();
unsigned long previousTime = 0;
const long timeoutTime = 1000;

bool lastButtonState = HIGH;
bool buttonPressed = false;

void setup() {
  Serial.begin(115200);

  pinMode(output5, OUTPUT);
  pinMode(output4, OUTPUT);
  pinMode(buttonPin, INPUT_PULLUP);
  digitalWrite(output5, LOW);
  digitalWrite(output4, LOW);

  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected.");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  server.begin();
}

void loop() {
  // Check button state
  bool currentButtonState = digitalRead(buttonPin);
  if (lastButtonState == HIGH && currentButtonState == LOW) {
    buttonPressed = true;
    // Додаємо відправку повідомлення через Serial при натисканні кнопки
    Serial.println("Button pressed!");  // Це повідомлення з'явиться у вікні C# програми
  }
  lastButtonState = currentButtonState;
  WiFiClient client = server.available();

  if (client) {
    currentTime = millis();
    previousTime = currentTime;
    Serial.println("New Client.");
    String currentLine = "";
    while (client.connected() && currentTime - previousTime <= timeoutTime) {
      currentTime = millis();
      if (client.available()) {
        char c = client.read();
        Serial.write(c);
        header += c;
        if (c == '\n') {
          if (currentLine.length() == 0) {
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println("Connection: close");
            client.println();

            if (header.indexOf("GET /5/on") >= 0) {
              Serial.println("GPIO 5 on");
              output5State = "on";
              digitalWrite(output5, HIGH);
            } else if (header.indexOf("GET /5/off") >= 0) {
              Serial.println("GPIO 5 off");
              output5State = "off";
              digitalWrite(output5, LOW);
            } else if (header.indexOf("GET /4/on") >= 0) {
              Serial.println("GPIO 4 on");
              output4State = "on";
              digitalWrite(output4, HIGH);
            } else if (header.indexOf("GET /4/off") >= 0) {
              Serial.println("GPIO 4 off");
              output4State = "off";
              digitalWrite(output4, LOW);
            }

            client.println("<!DOCTYPE html><html>");
            client.println("<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
            client.println("<meta http-equiv='refresh' content='1'>");  // Auto-refresh every second
            client.println("<link rel=\"icon\" href=\"data:,\">");
            client.println("<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}");
            client.println(".button { background-color: #195B6A; border: none; color: white; padding: 16px 40px;");
            client.println("text-decoration: none; font-size: 30px; margin: 2px; cursor: pointer;}");
            client.println(".button2 {background-color: #77878A;}</style></head>");

            client.println("<body><h1>ESP8266 Web Server</h1>");

            // GPIO 5
            client.println("<p>GPIO 5 - State " + output5State + "</p>");
            if (output5State == "off") {
              client.println("<p><a href=\"/5/on\"><button class=\"button\">ON</button></a></p>");
            } else {
              client.println("<p><a href=\"/5/off\"><button class=\"button button2\">OFF</button></a></p>");
            }

            // GPIO 4
            client.println("<p>GPIO 4 - State " + output4State + "</p>");
            if (output4State == "off") {
              client.println("<p><a href=\"/4/on\"><button class=\"button\">ON</button></a></p>");
            } else {
              client.println("<p><a href=\"/4/off\"><button class=\"button button2\">OFF</button></a></p>");
            }

            // Button message
            if (buttonPressed) {

              client.println("<p>fagbefhuabfys: </p>");
              buttonPressed = false;
            }

            client.println("</body></html>");
            client.println();
            break;
          } else {
            currentLine = "";
          }
        } else if (c != '\r') {
          currentLine += c;
        }
      }
    }
    header = "";
    client.stop();
    Serial.println("Client disconnected.");
    Serial.println("");
  }

  // Check for serial data from PC program
  if (Serial.available() > 0) {
    String command = Serial.readStringUntil('\n');
    command.trim();

    if (command == "5ON") {
      digitalWrite(output5, HIGH);
      output5State = "on";
      Serial.println("GPIO 5 ON via Serial");
    } else if (command == "5OFF") {
      digitalWrite(output5, LOW);
      output5State = "off";
      Serial.println("GPIO 5 OFF via Serial");
    } else if (command == "4ON") {
      digitalWrite(output4, HIGH);
      output4State = "on";
      Serial.println("GPIO 4 ON via Serial");
    } else if (command == "4OFF") {
      digitalWrite(output4, LOW);
      output4State = "off";
      Serial.println("GPIO 4 OFF via Serial");
    }
  }
}
