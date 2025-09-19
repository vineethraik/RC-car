#include <Arduino.h>
#include <WiFi.h>
#include <WebSocketsServer.h>
#include <WebServer.h>
#include <ArduinoJson.h>

WebServer server(80);
WebSocketsServer webSocket = WebSocketsServer(81);



#define EnableA 32
#define EnableB 26
#define MB1 14 //26
#define MB2 27 //25
#define MA1 33 //27
#define MA2 25 //14

#define INPUT_MIN -100
#define INPUT_MAX 100
#define OUTPUT_MAX 255
#define DEADZONE 20

void xy_to_lr(int x, int y, float *pwmA, float *pwmB, int *ma1, int *ma2, int *mb1, int *mb2) {
int speed = max(abs(x),abs(y));

*pwmA = (speed * 255.0)/100.0;
*pwmB = (speed * 255.0)/100.0;
if(y<0){
  *ma1 = 1;
  *ma2 = 0;
  *mb1 = 1;
  *mb2 = 0;
}else{
  *ma1 = 0;
  *ma2 = 1;
  *mb1 = 0;
  *mb2 = 1;
}

if(x<0){
  int uX = abs(x) - 50;
  *pwmA = (*pwmA) * (abs(uX)/50.0);
  if (uX >= 0)
  {
    *ma1 = !*ma1;
    *ma2 = !*ma2;
  }
}else{
  int uX = abs(x) - 50;
  *pwmB = (*pwmB) * (abs(uX) / 50.0);

  if (uX >= 0)
  {
    *mb1 = !*mb1;
    *mb2 = !*mb2;
  }
}

};

void handleRoot()
{
  server.send(200, "text/html", F("<!DOCTYPE html>\n<html lang=\"en\">\n<head>\n    <meta charset=\"UTF-8\">\n    <meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0, maximum-scale=1.0, user-scalable=no\">\n    <title>RC Car Control</title>\n    <style>\n        body {\n            font-family: Arial, sans-serif;\n            display: flex;\n            flex-direction: column;\n            align-items: center;\n            justify-content: center;\n            min-height: 100vh;\n            margin: 0;\n            background-color: #f0f0f0;\n            touch-action: manipulation; /* Prevent browser gestures */\n            -webkit-user-select: none; /* Safari */\n            -moz-user-select: none; /* Firefox */\n            -ms-user-select: none; /* IE10+ */\n            user-select: none; /* Standard syntax */\n        }\n        .controls {\n            margin-bottom: 20px;\n            display: flex;\n            flex-direction: column;\n            gap: 10px;\n            width: 80%;\n            max-width: 400px;\n        }\n        .controls input {\n            padding: 10px;\n            border: 1px solid #ccc;\n            border-radius: 5px;\n            font-size: 16px;\n        }\n        .buttons {\n            display: flex;\n            gap: 10px;\n        }\n        .buttons button {\n            flex: 1;\n            padding: 10px 20px;\n            border: none;\n            border-radius: 50px;\n            font-size: 16px;\n            cursor: pointer;\n            color: white;\n            box-shadow: 0 4px 6px rgba(0, 0, 0, 0.1);\n            transition: background-color 0.3s ease;\n        }\n        #startButton {\n            background-color: #28a745;\n        }\n        #startButton:hover:not(:disabled) {\n            background-color: #218838;\n        }\n        #stopButton {\n            background-color: #dc3545;\n        }\n        #stopButton:hover:not(:disabled) {\n            background-color: #c82333;\n        }\n        button:disabled {\n            background-color: #6c757d;\n            cursor: not-allowed;\n        }\n        #joystickContainer {\n            width: 200px;\n            height: 200px;\n            background-color: #e0e0e0;\n            border-radius: 50%;\n            border: 2px solid #ccc;\n            position: relative;\n            margin-top: 20px;\n            box-shadow: inset 0 0 10px rgba(0, 0, 0, 0.1);\n        }\n        #joystick {\n            width: 80px;\n            height: 80px;\n            background-color: #007bff;\n            border-radius: 50%;\n            position: absolute;\n            top: 50%;\n            left: 50%;\n            transform: translate(-50%, -50%);\n            cursor: grab;\n            box-shadow: 0 4px 8px rgba(0, 0, 0, 0.2);\n            transition: background-color 0.3s ease;\n        }\n        #joystick.active {\n            background-color: #0056b3;\n            cursor: grabbing;\n        }\n        #status {\n            margin-top: 20px;\n            font-size: 1.1em;\n            color: #333;\n            font-weight: bold;\n        }\n    </style>\n</head>\n<body>\n    <div class=\"controls\">\n        <input type=\"text\" id=\"ipInput\" placeholder=\"IP Address (e.g., 192.168.1.100)\" value=\"192.168.4.1\">\n        <input type=\"text\" id=\"portInput\" placeholder=\"Port (e.g., 8080)\" value=\"81\">\n        <div class=\"buttons\">\n            <button id=\"startButton\">Start</button>\n            <button id=\"stopButton\" disabled>Stop</button>\n        </div>\n    </div>\n    <div id=\"joystickContainer\">\n        <div id=\"joystick\"></div>\n    </div>\n    <div id=\"status\">Disconnected</div>\n\n    <script>\n        let ws;\n        let sendInterval;\n        let joystickData = { x: 0, y: 0 };\n        const statusDiv = document.getElementById('status');\n        const startButton = document.getElementById('startButton');\n        const stopButton = document.getElementById('stopButton');\n        const ipInput = document.getElementById('ipInput');\n        const portInput = document.getElementById('portInput');\n\n        const joystickContainer = document.getElementById('joystickContainer');\n        const joystick = document.getElementById('joystick');\n        let isDragging = false;\n        const containerRect = joystickContainer.getBoundingClientRect();\n        const containerCenterX = containerRect.width / 2;\n        const containerCenterY = containerRect.height / 2;\n        const maxDistance = containerRect.width / 2 - (joystick.offsetWidth / 2); // Max movement radius for the joystick\n\n        function updateJoystickPosition(clientX, clientY) {\n            const containerX = containerRect.left + window.scrollX;\n            const containerY = containerRect.top + window.scrollY;\n\n            let x = clientX - containerX - containerCenterX;\n            let y = clientY - containerY - containerCenterY;\n\n            // Limit joystick movement within the container\n            const distance = Math.sqrt(x * x + y * y);\n            if (distance > maxDistance) {\n                const angle = Math.atan2(y, x);\n                x = maxDistance * Math.cos(angle);\n                y = maxDistance * Math.sin(angle);\n            }\n\n            joystick.style.left = `${containerCenterX + x}px`;\n            joystick.style.top = `${containerCenterY + y}px`;\n\n            // Normalize x and y to a range of -100 to 100\n            joystickData.x = Math.round((x / maxDistance) * 100);\n            joystickData.y = Math.round((y / maxDistance) * 100);\n        }\n\n        function resetJoystick() {\n            joystick.style.left = '50%';\n            joystick.style.top = '50%';\n            joystickData = { x: 0, y: 0 };\n            joystick.classList.remove('active');\n        }\n\n        // Mouse events\n        joystick.addEventListener('mousedown', (e) => {\n            if (startButton.disabled) { // Only allow dragging if connected\n                isDragging = true;\n                joystick.classList.add('active');\n                e.preventDefault(); // Prevent text selection\n            }\n        });\n\n        document.addEventListener('mousemove', (e) => {\n            if (isDragging) {\n                updateJoystickPosition(e.clientX, e.clientY);\n            }\n        });\n\n        document.addEventListener('mouseup', () => {\n            if (isDragging) {\n                isDragging = false;\n                resetJoystick();\n            }\n        });\n\n        // Touch events\n        joystick.addEventListener('touchstart', (e) => {\n            if (startButton.disabled) { // Only allow dragging if connected\n                isDragging = true;\n                joystick.classList.add('active');\n                e.preventDefault(); // Prevent scrolling and other touch gestures\n            }\n        });\n\n        document.addEventListener('touchmove', (e) => {\n            if (isDragging && e.touches.length === 1) {\n                updateJoystickPosition(e.touches[0].clientX, e.touches[0].clientY);\n                e.preventDefault();\n            }\n        });\n\n        document.addEventListener('touchend', () => {\n            if (isDragging) {\n                isDragging = false;\n                resetJoystick();\n            }\n        });\n\n        startButton.addEventListener('click', () => {\n            const ip = ipInput.value;\n            const port = portInput.value;\n            if (!ip || !port) {\n                alert('Please enter both IP address and Port.');\n                return;\n            }\n\n            const wsUrl = `ws://${ip}:${port}/ws`; // Assuming WebSocket path is /ws\n            ws = new WebSocket(wsUrl);\n\n            ws.onopen = () => {\n                statusDiv.textContent = 'Connected';\n                startButton.disabled = true;\n                stopButton.disabled = false;\n                ipInput.disabled = true;\n                portInput.disabled = true;\n                resetJoystick(); // Ensure joystick is centered on connect\n                sendInterval = setInterval(() => {\n                    if (ws.readyState === WebSocket.OPEN) {\n                        ws.send(JSON.stringify(joystickData));\n                    }\n                }, 100);\n            };\n\n            ws.onmessage = (event) => {\n                console.log('Message from server:', event.data);\n            };\n\n            ws.onerror = (error) => {\n                statusDiv.textContent = 'Connection Error';\n                console.error('WebSocket Error:', error);\n                stopConnection();\n            };\n\n            ws.onclose = () => {\n                statusDiv.textContent = 'Disconnected';\n                stopConnection();\n            };\n        });\n\n        stopButton.addEventListener('click', stopConnection);\n\n        function stopConnection() {\n            if (ws) {\n                ws.close();\n            }\n            clearInterval(sendInterval);\n            startButton.disabled = false;\n            stopButton.disabled = true;\n            ipInput.disabled = false;\n            portInput.disabled = false;\n            resetJoystick(); // Reset joystick position and data on disconnect\n            statusDiv.textContent = 'Disconnected';\n        }\n\n        // Initial state\n        stopButton.disabled = true;\n        resetJoystick(); // Ensure joystick is centered initially\n    </script>\n</body>\n</html>\n"));
}

void handleMotor(int x,  int y){
    int ma1 = 0,ma2=0,mb1=0,mb2=0;
    float pwmA=0.0,pwmB=0.0;

    xy_to_lr(x,y,&pwmA,&pwmB,&ma1,&ma2,&mb1,&mb2);

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

    Serial.printf("x:%d,y:%d,ma1:%d,ma2:%d,mb1:%d,mb2:%d,pwmA:%f,pwmB:%f,\n", x, y, ma1, ma2, mb1, mb2, pwmA, pwmB);
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
      // Serial.printf("[%u] get Text: %s\n", num, payload);
    JsonDocument doc;
    deserializeJson(doc, payload);
    int x = doc["x"];
    int y = doc["y"];
    handleMotor(x, y);}
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
}

void loop(

)
{
  server.handleClient();
  webSocket.loop();
}

// put function definitions here:
