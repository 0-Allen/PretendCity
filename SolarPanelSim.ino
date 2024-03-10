#include <WiFi.h>
#include <EEPROM.h>

//const char *ssid = "ESP32_LED_Controller";  // Access Point SSID
//const char *password = "password";          // Access Point Password

WiFiServer server(80);

int ledPin = 13;                   // Default LED pin
int storedPin = 13;                // Initially stored pin
int brightness = 0;                // LED brightness
bool increasingBrightness = true;  // Flag to control brightness direction

int secondLedPin = 12;        // Second LED pin
bool secondLedState = false;  // State of the second LED pin

int buttonPin = 14;

void setup() {
  Serial.begin(115200);
  EEPROM.begin(512);

  // Start the Access Point
//  WiFi.softAP(ssid, password);
//  Serial.println("Access Point started");
//  Serial.print("IP Address: ");
//  Serial.println(WiFi.softAPIP());

  // Start the server
//  server.begin();

  // Initialize LED pin
  pinMode(ledPin, OUTPUT);
  pinMode(secondLedPin, OUTPUT);
  pinMode(buttonPin, INPUT_PULLDOWN);
}

void loop() {
  //  serveWebPage();
  updateLED();
}

void serveWebPage() {
  WiFiClient client = server.available();
  if (client) {
    // Wait until the client sends some data
    while (client.connected()) {
      if (client.available()) {
        // Read the first line of the request
        String request = client.readStringUntil('\r');
        Serial.println(request);
        client.flush();

        // Match the request to change LED pin
        if (request.indexOf("/pinChange") != -1) {
          int newPin = request.substring(request.indexOf("pin=") + 4).toInt();
          changeLED(newPin);
        }

        // Prepare the response
        client.println("HTTP/1.1 200 OK");
        client.println("Content-Type: text/html");
        client.println("Connection: close");
        client.println();
        client.println("<!DOCTYPE HTML>");
        client.println("<html>");
        client.println("<head><title>ESP32 LED Controller</title></head>");
        client.println("<body>");
        client.println("<h1>ESP32 LED Controller</h1>");
        client.println("<p>Current LED Pin: " + String(storedPin) + "</p>");
        client.println("<form action='/pinChange'>");
        client.println("New LED Pin: <input type='number' name='pin' min='0' max='40'>");
        client.println("<input type='submit' value='Change Pin'>");
        client.println("</form>");
        client.println("</body>");
        client.println("</html>");

        delay(1);
        break;
      }
    }
    // Close the connection
    client.stop();
  }
}

void updateLED() {
  while (digitalRead(buttonPin)) {
    // PWM the LED brightness on and off
    analogWrite(ledPin, brightness);
    if (increasingBrightness) {
      brightness += 5;
      delay(50);
      if (brightness >= 255) {
        increasingBrightness = false;
        // Turn on the second LED pin when brightness is fully on
        digitalWrite(secondLedPin, HIGH);
        secondLedState = true;
      }
    }
    // else {
    // brightness -= 5;
    // if (brightness <= 0) {
    //   increasingBrightness = true;
    //   // Turn off the second LED pin when brightness is fully off
    //   digitalWrite(secondLedPin, LOW);
    //   secondLedState = false;
    // }
  }

   delay(50);

  if (!digitalRead(buttonPin)) {
    if (brightness > 0)
      brightness -= 5;
    analogWrite(ledPin, brightness);
    if (brightness == 0) {
      delay(1000);
      digitalWrite(secondLedPin, LOW);
      increasingBrightness = true;
    }
  }
}

void changeLED(int newPin) {
  if (newPin >= 0 && newPin <= 40 && newPin != ledPin) {
    // Deinitialize current LED pin
    pinMode(ledPin, INPUT);

    // Initialize new LED pin
    ledPin = newPin;
    storedPin = newPin;
    pinMode(ledPin, OUTPUT);

    EEPROM.write(0, newPin);
    EEPROM.commit();
    Serial.println("LED pin changed to " + String(newPin));
  }
}
