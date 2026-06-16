#include <Arduino.h>
#include <WiFi.h>
#include <WebSocketsServer.h>
#include <WebServer.h>
#include <ArduinoJson.h>
#include "htmlData.h"

WebServer server(80);
WebSocketsServer webSocket = WebSocketsServer(81);

#define EnableA 32
#define EnableB 26
#define MB1 14
#define MB2 27
#define MA1 33
#define MA2 25

#define INPUT_MIN -100
#define INPUT_MAX 100
#define OUTPUT_MAX 255
#define DEADZONE 20

enum packetType
{
  JOYSTICKPACKET
};

void xy_to_lr(int x, int y, float *pwmA, float *pwmB, int *ma1, int *ma2, int *mb1, int *mb2)
{
  x = constrain(x, INPUT_MIN, INPUT_MAX);
  y = constrain(y, INPUT_MIN, INPUT_MAX);

  if (abs(x) < DEADZONE)
  {
    x = 0;
  }
  if (abs(y) < DEADZONE)
  {
    y = 0;
  }

  int throttle = -y;
  int leftCommand = constrain(throttle - x, INPUT_MIN, INPUT_MAX);
  int rightCommand = constrain(throttle + x, INPUT_MIN, INPUT_MAX);

  *pwmA = (abs(leftCommand) * OUTPUT_MAX) / 100.0;
  *pwmB = (abs(rightCommand) * OUTPUT_MAX) / 100.0;

  if (leftCommand > 0)
  {
    *ma1 = 1;
    *ma2 = 0;
  }
  else if (leftCommand < 0)
  {
    *ma1 = 0;
    *ma2 = 1;
  }
  else
  {
    *ma1 = 0;
    *ma2 = 0;
  }

  if (rightCommand > 0)
  {
    *mb1 = 1;
    *mb2 = 0;
  }
  else if (rightCommand < 0)
  {
    *mb1 = 0;
    *mb2 = 1;
  }
  else
  {
    *mb1 = 0;
    *mb2 = 0;
  }
}

void handleRoot()
{
  server.send(200, "text/html", indexHTML);
}

void handleMotor(int x, int y, int speedPercent = 100)
{
  int ma1 = 0, ma2 = 0, mb1 = 0, mb2 = 0;
  float pwmA = 0.0, pwmB = 0.0;

  speedPercent = constrain(speedPercent, 0, 100);
  xy_to_lr(x, y, &pwmA, &pwmB, &ma1, &ma2, &mb1, &mb2);
  pwmA = (pwmA * speedPercent) / 100.0;
  pwmB = (pwmB * speedPercent) / 100.0;

  analogWrite(EnableA, pwmA);
  digitalWrite(MA1, ma1);
  digitalWrite(MA2, ma2);
  analogWrite(EnableB, pwmB);
  digitalWrite(MB1, mb1);
  digitalWrite(MB2, mb2);

  Serial.printf("x:%d,y:%d,ma1:%d,ma2:%d,mb1:%d,mb2:%d,pwmA:%f,pwmB:%f\n", x, y, ma1, ma2, mb1, mb2, pwmA, pwmB);
}

void webSocketEvent(uint8_t num, WStype_t type, uint8_t *payload, size_t length)
{
  switch (type)
  {
  case WStype_DISCONNECTED:
    handleMotor(0, 0);
    break;

  case WStype_CONNECTED:
  {
    IPAddress ip = webSocket.remoteIP(num);
    Serial.printf("[%u] Connected from %d.%d.%d.%d\n", num, ip[0], ip[1], ip[2], ip[3]);
    handleMotor(0, 0);
  }
  break;

  case WStype_TEXT:
  {
    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, payload, length);
    if (error)
    {
      Serial.println("Invalid websocket JSON");
      return;
    }

    int type = doc["type"] | JOYSTICKPACKET;
    if (type == JOYSTICKPACKET)
    {
      int x = doc["x"] | 0;
      int y = doc["y"] | 0;
      int speedPercent = doc["speed"] | 100;
      handleMotor(x, y, speedPercent);
    }
  }
  break;

  case WStype_BIN:
    break;
  }
}

void setup()
{
  Serial.begin(115200);

  pinMode(EnableA, OUTPUT);
  pinMode(EnableB, OUTPUT);
  pinMode(MB1, OUTPUT);
  pinMode(MB2, OUTPUT);
  pinMode(MA1, OUTPUT);
  pinMode(MA2, OUTPUT);

  handleMotor(0, 0);

  WiFi.mode(WIFI_AP_STA);
  WiFi.softAP("RC_car", "");

  Serial.println("RC car ready");
  Serial.println(WiFi.localIP());

  server.on("/", handleRoot);
  server.begin();
  webSocket.begin();
  webSocket.onEvent(webSocketEvent);
}

void loop()
{
  server.handleClient();
  webSocket.loop();
}
