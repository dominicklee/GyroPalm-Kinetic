#include <M5StickCPlus.h>
#include "OneButton.h"
#include <ESP32Servo.h>

#include <WiFi.h>
#include <esp_now.h>
#define WIFI_CHANNEL 1 

bool isTriggered = false;

OneButton btnB(G39, true);

// ESP-NOW Declarations -------------------------------------------------------
static uint8_t broadcast_mac[] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };

typedef struct __attribute__((packed)) esp_now_msg_t
{
  String wearableID;
  String action;
  // Can put lots of things here...
} esp_now_msg_t;

static void handle_error(esp_err_t err)
{
  switch (err)
  {
    case ESP_ERR_ESPNOW_NOT_INIT:
      Serial.println("Not init");
      break;

    case ESP_ERR_ESPNOW_ARG:
      Serial.println("Argument invalid");
      break;

    case ESP_ERR_ESPNOW_INTERNAL:
      Serial.println("Internal error");
      break;

    case ESP_ERR_ESPNOW_NO_MEM:
      Serial.println("Out of memory");
      break;

    case ESP_ERR_ESPNOW_NOT_FOUND:
      Serial.println("Peer is not found");
      break;

    case ESP_ERR_ESPNOW_IF:
      Serial.println("Current WiFi interface doesn't match that of peer");
      break;

    default:
      break;
  }
}

static void msg_recv_cb(const uint8_t *mac_addr, const uint8_t *data, int len)
{
  if (len == sizeof(esp_now_msg_t))
  {
    esp_now_msg_t msg;
    memcpy(&msg, data, len);

    Serial.print("WearableID: ");
    Serial.println(msg.wearableID);
    Serial.print("Action: ");
    Serial.println(msg.action);

    if (msg.action == "launch") {
      M5.Lcd.setCursor(10, 30);
      M5.Lcd.setTextColor(RED, BLACK);
      M5.Lcd.print("Launching...");
      isTriggered = true;
    }
    
  }
}

static void msg_send_cb(const uint8_t* mac, esp_now_send_status_t sendStatus)
{

  switch (sendStatus)
  {
    case ESP_NOW_SEND_SUCCESS:
      Serial.println("Send success");
      break;

    case ESP_NOW_SEND_FAIL:
      Serial.println("Send Failure");
      break;

    default:
      break;
  }
}

static void send_msg(esp_now_msg_t * msg)
{
  // Pack
  uint16_t packet_size = sizeof(esp_now_msg_t);
  uint8_t msg_data[packet_size];
  memcpy(&msg_data[0], msg, sizeof(esp_now_msg_t));

  esp_err_t status = esp_now_send(broadcast_mac, msg_data, packet_size);
  if (ESP_OK != status)
  {
    Serial.println("Error sending message");
    handle_error(status);
  }
}

static void network_setup(void)
{
  //Puts ESP in STATION MODE
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();

  if (esp_now_init() != 0)
  {
    return;
  }

  esp_now_peer_info_t peer_info;
  peer_info.channel = WIFI_CHANNEL;
  memcpy(peer_info.peer_addr, broadcast_mac, 6);
  peer_info.ifidx = ESP_IF_WIFI_STA;
  peer_info.encrypt = false;
  esp_err_t status = esp_now_add_peer(&peer_info);
  if (ESP_OK != status)
  {
    Serial.println("Could not add peer");
    handle_error(status);
  }

  // Set up callback
  status = esp_now_register_recv_cb(msg_recv_cb);
  if (ESP_OK != status)
  {
    Serial.println("Could not register callback");
    handle_error(status);
  }

  status = esp_now_register_send_cb(msg_send_cb);
  if (ESP_OK != status)
  {
    Serial.println("Could not register send callback");
    handle_error(status);
  }
}

// ESP-NOW Declarations -------------------------------------------------------

Servo myservo;  // create servo object to control a servo
// 16 servo objects can be created on the ESP32

int pos = 0;    // variable to store the servo position
// Recommended PWM GPIO pins on the ESP32 include 2,4,12-19,21-23,25-27,32-33 
int servoPin = G26;

void setup() {
  M5.begin();
  M5.Lcd.fillScreen(BLACK);
  M5.Lcd.setCursor(10, 10);
  M5.Lcd.setTextColor(WHITE);
  M5.Lcd.setTextSize(1);
  M5.Lcd.printf("GyroPalm Kinetic");

  //Show OFF instructions
  M5.Lcd.setTextColor(CYAN);
  M5.Lcd.setTextSize(1);
  M5.Lcd.setCursor(85, 105);
  M5.Lcd.printf("Off ->");
  
  pinMode(M5_BUTTON_HOME, INPUT);
  
	myservo.setPeriodHertz(50);    // standard 50 hz servo
	myservo.attach(servoPin, 1000, 2000); // attaches the servo on pin 18 to the servo object
	// using default min/max of 1000us and 2000us
	// different servos may require different min/max settings
	// for an accurate 0 to 180 sweep

 myservo.write(10);
 delay(2000);
 network_setup();

 btnB.attachClick(btnBClick);  //BtnB handle
 btnB.setDebounceTicks(25);
}

void loop() {
  btnB.tick();

  if (digitalRead(M5_BUTTON_HOME) == LOW) {
    isTriggered = true;
    M5.Lcd.setCursor(10, 30);
    M5.Lcd.setTextColor(RED, BLACK);
    M5.Lcd.print("Launching...");
    delay(3000);
  }

  if (isTriggered) {
    isTriggered = false;
    myservo.write(180);
    delay(2000);
    myservo.write(10);
    M5.Lcd.setCursor(10, 30);
    M5.Lcd.setTextColor(RED, BLACK);
    M5.Lcd.print("             ");
  }
  
  delay(10);
}

void btnBClick()
{
  M5.Axp.PowerOff();
}
