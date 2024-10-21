#include <ESP8266WiFi.h>

const char* ssid = "Redmi Note 13";
const char* password = "z4j23yq93524rgm";

WiFiServer server(80);

String header;
String output5State = "off";
String output4State = "off";
String buttonMessage = "";  // Змінна для зберігання повідомлення про кнопку

const int output5 = 5;
const int output4 = 4;
const int buttonPin = D5;

unsigned long currentTime = millis();
unsigned long previousTime = 0;
const long timeoutTime = 1000;

bool lastButtonState = HIGH;

void setup() {
  Serial.begin(115200);

  pinMode(output5, OUTPUT);
  pinMode(output4, OUTPUT);
  pinMode(buttonPin, INPUT_PULLUP);
  digitalWrite(output5, LOW);
  digitalWrite(output4, LOW);

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }
  server.begin();
}

void loop() {
  // Перевірка стану кнопки
  bool currentButtonState = digitalRead(buttonPin);
  if (lastButtonState == HIGH && currentButtonState == LOW) {
    Serial.println("Physical Button Pressed");
    buttonMessage = "Physical Button Pressed"; // Зберігаємо повідомлення для відображення на веб-сторінці
  }
  lastButtonState = currentButtonState;

  if (Serial.available() > 0) {
    String command = Serial.readStringUntil('\n');
    command.trim();

    if (command == "5ON") {
      digitalWrite(output5, HIGH);
      output5State = "on";
      Serial.println("LED 1 turned ON");
    } 
    else if (command == "5OFF") {
      digitalWrite(output5, LOW);
      output5State = "off";
      Serial.println("LED 1 turned OFF");
    } 
    else if (command == "4ON") {
      digitalWrite(output4, HIGH);
      output4State = "on";
      Serial.println("LED 2 turned ON");
    } 
    else if (command == "4OFF") {
      digitalWrite(output4, LOW);
      output4State = "off";
      Serial.println("LED 2 turned OFF");
    }
  }
  
  // Обробка веб-клієнтів
  WiFiClient client = server.available();
  if (client) {
    currentTime = millis();
    previousTime = currentTime;
    String currentLine = "";
    
    while (client.connected() && currentTime - previousTime <= timeoutTime) {
      currentTime = millis();
      if (client.available()) {
        char c = client.read();
        header += c;
        if (c == '\n') {
          if (currentLine.length() == 0) {
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println("Connection: close");
            client.println();

            if (header.indexOf("GET /5/on") >= 0) {
              digitalWrite(output5, HIGH);
              output5State = "on";
            } 
            else if (header.indexOf("GET /5/off") >= 0) {
              digitalWrite(output5, LOW);
              output5State = "off";
            } 
            else if (header.indexOf("GET /4/on") >= 0) {
              digitalWrite(output4, HIGH);
              output4State = "on";
            } 
            else if (header.indexOf("GET /4/off") >= 0) {
              digitalWrite(output4, LOW);
              output4State = "off";
            }

            // Відправка HTML сторінки
            client.println("<!DOCTYPE html><html>");
            client.println("<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
            client.println("<meta http-equiv='refresh' content='1'>");
            client.println("<link rel=\"icon\" href=\"data:,\">");
            client.println("<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}");
            client.println(".button { background-color: #195B6A; border: none; color: white; padding: 16px 40px;");
            client.println("text-decoration: none; font-size: 30px; margin: 2px; cursor: pointer;}");
            client.println(".button2 {background-color: #77878A;}");
            client.println(".message { color: #195B6A; font-size: 20px; margin-top: 20px; }</style></head>");

            client.println("<body><h1>ESP8266 Web Server</h1>");

            // Виведення стану GPIO 5
            client.println("<p>GPIO 5 - State " + output5State + "</p>");
            if (output5State == "off") {
              client.println("<p><a href=\"/5/on\"><button class=\"button\">ON</button></a></p>");
            } else {
              client.println("<p><a href=\"/5/off\"><button class=\"button button2\">OFF</button></a></p>");
            }

            // Виведення стану GPIO 4
            client.println("<p>GPIO 4 - State " + output4State + "</p>");
            if (output4State == "off") {
              client.println("<p><a href=\"/4/on\"><button class=\"button\">ON</button></a></p>");
            } else {
              client.println("<p><a href=\"/4/off\"><button class=\"button button2\">OFF</button></a></p>");
            }

            // Виведення повідомлення про натискання кнопки
            if (buttonMessage != "") {
              client.println("<p class=\"message\">" + buttonMessage + "</p>");
              buttonMessage = ""; // Очищаємо повідомлення після відображення
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
  }
}
