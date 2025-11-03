#ifndef ARDUINO_H
#define ARDUINO_H
#include <Arduino.h>
#endif
#include <WiFi.h>
#include <WebSocketsServer.h>
#include <WebServer.h>
#include <ArduinoJson.h>
#include <ArduinoOTA.h>
#include "telnet.h"
#include "htmlData.h"

WebServer server(80);
WebSocketsServer webSocket = WebSocketsServer(81);
telnet t;

#define EnableA 32
#define EnableB 26
#define MB1 14 // 26
#define MB2 27 // 25
#define MA1 33 // 27
#define MA2 25 // 14

#define TRIG 17 // TX2 pin
#define ECHO 5  // D5 pin

#define CH_A 0
#define CH_B 1

#define INPUT_MIN -100
#define INPUT_MAX 100
#define OUTPUT_MAX 255
#define DEADZONE 20

int irPins[8] = {36, 39, 34, 35, 12, 13, 4, 15};
bool irState[8] = {0, 0, 0, 0, 0, 0, 0, 0};

void xy_to_lr(int x, int y, float *pwmA, float *pwmB, int *ma1, int *ma2, int *mb1, int *mb2)
{
  int speed = max(abs(x), abs(y));

  *pwmA = (speed * 255.0) / 100.0;
  *pwmB = (speed * 255.0) / 100.0;
  if (y < 0)
  {
    *ma1 = 1;
    *ma2 = 0;
    *mb1 = 1;
    *mb2 = 0;
  }
  else
  {
    *ma1 = 0;
    *ma2 = 1;
    *mb1 = 0;
    *mb2 = 1;
  }

  if (x < 0)
  {
    int uX = abs(x) - 50;
    *pwmA = (*pwmA) * (abs(uX) / 50.0);
    if (uX >= 0)
    {
      *ma1 = !*ma1;
      *ma2 = !*ma2;
    }
  }
  else
  {
    int uX = abs(x) - 50;
    *pwmB = (*pwmB) * (abs(uX) / 50.0);

    if (uX >= 0)
    {
      *mb1 = !*mb1;
      *mb2 = !*mb2;
    }
  }
};

int result[2];
// String get_xy_from_n_array(int n, bool *n_array)
// {
//   int midpoint = (n + 1) / 2;
//   int ac = 0;
//   int max = 0;
//   for (int i = 0; i < sizeof(n_array) / sizeof(n_array[0]); i++)
//   {
//     if (n_array[i])
//     {
//       ac += i + 1 - midpoint;
//     }
//     if (i + 1 > midpoint)
//     {
//       max += i + 1 - midpoint;
//     }
//   }

//   int x = 0, y = 0;
//   int speed_multiplier = 30;

//   x = (int)(((float)ac / (float)max) * speed_multiplier);
//   y = -speed_multiplier + abs(x);
//   JsonDocument doc;
//   doc["x"] = x;
//   doc["y"] = y;
//   String result;
//   serializeJson(doc, result);
//   Serial.println(result);

//   return result;
//}

String get_xy_from_n_array(int n, bool *n_array)
{
  int midpoint = (n + 1) / 2;
  int ac = 0;
  int max = 0;
  for (int i = 0; i<n; i++)
  {
    if (!n_array[i])
    {
      ac += i + 1 - midpoint;
    }
    if (i + 1 > midpoint)
    {
      max += i + 1 - midpoint;
    }
  }
  int x = 0, y = 0;
  max=max==0?1:max;
  int speed_multiplier = 100;
  x = (int)(((float)ac / (float)max) * speed_multiplier);
  y =-30;
  JsonDocument doc;
  doc["x"] = x;
  doc["y"] = y;
  String result;
  serializeJson(doc, result);
  // Serial.println(result);
  return result;
}
enum codeTypes
{
  JOYSTICK,
  OAC,
  LINE_FOLLOWER
};
int codeType = JOYSTICK;
enum packetType
{
  JOYSTICKPACKET,
  OACPACKET,
  LINEFOLLOWERPACKET,
};

void handleRoot()
{
  server.send(200, "text/html",indexHTML );
}

void handleMotor(int x, int y)
{
  int ma1 = 0, ma2 = 0, mb1 = 0, mb2 = 0;
  float pwmA = 0.0, pwmB = 0.0;

  xy_to_lr(x, y, &pwmA, &pwmB, &ma1, &ma2, &mb1, &mb2);

  // if (x < -threshold)
  // {
  //   pwmA = (-x)*1.0;
  //   ma1 = 0;
  //   ma2 = 1;
  //   pwmB = -x * 1.0;
  //   mb1 = 1;
  //   mb2 = 0;
  // }
  // if (x >threshold)
  // {
  //   pwmA = x * 1.0;
  //   ma1 = 1;
  //   ma2 = 0;
  //   pwmB = x * 1.0;
  //   mb1 = 0;
  //   mb2 = 1;
  // }
  // if (y < -threshold)
  // {
  //   pwmA = -y * 1.0;
  //   ma1 = 1;
  //   ma2 = 0;
  //   pwmB = -y * 1.0;
  //   mb1 = 1;
  //   mb2 = 0;
  // }
  // if (y > threshold)
  // {
  //   pwmA = y * 1.0;
  //   ma1 = 0;
  //   ma2 = 1;
  //   pwmB = y * 1.0;
  //   mb1 = 0;
  //   mb2 = 1;
  // }

  // xy_to_motors(x, y, &pwmA, &ma1, &pwmB, &mb1);
  // ma2 = !ma1;
  // mb2 = !mb1;
  // if (x < - threshold && y < - threshold)
  // {

  // }
  // else if (x > threshold && y < - threshold)
  // {
  // }
  // else if (x > threshold && y > threshold)
  // {
  // }
  // else if (x < - threshold && y > threshold)
  // {
  // }

  // analogWrite(EnableA, (abs(pwmA)/100.0)*255.0);
  // digitalWrite(MA1, ma1);
  // digitalWrite(MA2, ma2);
  // analogWrite(EnableB, (abs(pwmB)/100.0)*255.0);
  // digitalWrite(MB1, mb1);
  // digitalWrite(MB2, mb2);

  analogWrite(EnableA, pwmA);
  digitalWrite(MA1, ma1);
  digitalWrite(MA2, ma2);
  analogWrite(EnableB, pwmB);
  digitalWrite(MB1, mb1);
  digitalWrite(MB2, mb2);

  // Serial.printf("x:%d,y:%d,ma1:%d,ma2:%d,mb1:%d,mb2:%d,pwmA:%f,pwmB:%f,\n", x, y, ma1, ma2, mb1, mb2, pwmA, pwmB);
}

long getDistance()
{
  digitalWrite(TRIG, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG, LOW);

  long duration = pulseIn(ECHO, HIGH, 30000); // timeout 30ms (~5m)
  if (duration == 0)
    return 222;                         // no echo
  long distance = duration * 0.034 / 2; // in cm
  return distance;
}

void webSocketEvent(uint8_t num, WStype_t type, uint8_t *payload, size_t length)
{
  // Serial.printf("webSocketEvent %u\n", num);
  // Serial.printf("type: %u\n", type);
  // Serial.printf("length: %u\n", length);
  // Serial.printf("payload: %s\n", payload);
  switch (type)
  {
  case WStype_DISCONNECTED:
    // Serial.printf("[%u] Disconnected!\n", num);
    break;
  case WStype_CONNECTED:
  {
    IPAddress ip = webSocket.remoteIP(num);
    // Serial.printf("[%u] Connected from %d.%d.%d.%d url: %s\n", num, ip[0], ip[1], ip[2], ip[3], payload);
  }
  break;
  case WStype_TEXT:
  {
    Serial.printf("[%u] get Text: %s\n", num, payload);
    JsonDocument doc;
    deserializeJson(doc, payload);
    int type = doc["type"];
    switch (type)
    {
    case JOYSTICKPACKET:
    {
      int x = doc["x"];
      int y = doc["y"];
      if (codeType == JOYSTICK)
      {
        handleMotor(x, y);
      }
      else
      {
      }
    }
    break;
    case OACPACKET:
    {
      int value = doc["value"];
      if (value == true)
      {
        codeType = OAC;
      }
      else
      {
        codeType = JOYSTICK;
      }
    }
    break;
    case LINEFOLLOWERPACKET:
    {
      int value = doc["value"];
      if (value == true)
      {
        codeType = LINE_FOLLOWER;
      }
      else
      {
        codeType = JOYSTICK;
      }
    }
    break;
    }
  }
  break;
  case WStype_BIN:
    // Serial.printf("[%u] get binary length: %u\n", num, length);
    break;
  }
  // delete [] payload;
}

void setup()
{
  pinMode(EnableA, OUTPUT);
  pinMode(EnableB, OUTPUT);
  pinMode(MB1, OUTPUT);
  pinMode(MB2, OUTPUT);
  pinMode(MA1, OUTPUT);
  pinMode(MA2, OUTPUT);
  pinMode(TRIG, OUTPUT);
  pinMode(ECHO, INPUT);

  for (int i = 0; i < sizeof(irPins) / sizeof(irPins[0]); i++)
  {
    pinMode(irPins[i], INPUT);
  }

  // WiFi.mode(WIFI_STA);
  WiFi.mode(WIFI_AP_STA);
  // WiFi.begin(SSID, PASSWORD);
  WiFi.softAP("RC_car", "");

  Serial.begin(115200);
  Serial.println("Connecting to WiFi..");

  // while (WiFi.status() != WL_CONNECTED)
  // {
  // delay(1000);
  // analogWrite(EnableA, 255);
  // digitalWrite(MA1, 1);
  // digitalWrite(MA2, 0);
  // analogWrite(EnableB, 255);
  // digitalWrite(MB1, 1);
  // digitalWrite(MB2, 0);
  //   Serial.println(".");
  // }
  Serial.println(WiFi.localIP());

  server.on("/", handleRoot);
  server.begin();
  webSocket.begin();
  webSocket.onEvent(webSocketEvent);
  t.init();
}

void sdelay(int ms)
{
  for (int i = 0; i < ms; i += 10)
  {
    delay(10);
    server.handleClient();
    webSocket.loop();
    t.handle();
  }
}
void loop()
{
  server.handleClient();
  webSocket.loop();
  t.handle();

  if (codeType == OAC)
  {
    long distance = getDistance();
    Serial.print("Distance: ");
    Serial.print(distance);
    Serial.println(" cm");

    // Ignore noise below 5 cm and above 200 cm
    if (distance > 2 && distance < 20)
    {
      handleMotor(0, 0);

      sdelay(300);
      handleMotor(0, 50);

      sdelay(750);
      handleMotor(0, 0);

      sdelay(200);
      handleMotor(-50, 0);

      sdelay(500);
      handleMotor(0, 0);
      sdelay(500);
    }
    else
    {
      // forward(200); // clear path
      handleMotor(0, -100);
    }
  }
  else if (codeType == LINE_FOLLOWER)
  {

    String s = "";
    for (int i = 0; i < sizeof(irPins) / sizeof(irPins[0]); i++)
    {
      irState[i] = (!digitalRead(irPins[i]));
      s += String(irState[i]);
    }
    t.print(s);
    String xy = get_xy_from_n_array(sizeof(irState) / sizeof(irState[0]), irState);
   JsonDocument doc;
   deserializeJson(doc,xy);
    handleMotor(doc["x"], doc["y"]);
    t.println(xy );
    // sdelay(100);
    // int x=xy[0],y=xy[1];
    // handleMotor(xy[0], xy[1]);
  }
}

// put function definitions here:
