#include <Arduino.h>
#ifdef ESP32
#include <WiFi.h>
#else
#include <ESP8266WiFi.h>
#endif
#include <properties.h>
#include <PubSubClient.h>
#include <pump_control.h>

WiFiClient espClient;
PubSubClient client(espClient);
bool isPumpRunning = false;
void callback(char *topic, byte *payload, unsigned int length)
{
  bool oldState = isPumpRunning;
  char message[length + 1];
  for (unsigned int i = 0; i < length; i++)
  {
    message[i] = (char)payload[i];
  }
  message[length] = '\0';
  if (strcmp(message, "ON") == 0)
  {
    isPumpRunning = true;
    Serial.println("Turn the pump on");
  }
  else if (strcmp(message, "OFF") == 0)
  {
    isPumpRunning = false;
    Serial.println("Turn the pump off");
  }
  else if (strcmp(message, "STATE") == 0)
  {
    client.publish(MQTTTOPICSTATE, isPumpRunning ? "ON" : "OFF");
  }
  else
    Serial.println("Unknown command");
  if (oldState != isPumpRunning)
  {
    pumpControl(isPumpRunning, RELAYPIN);
    client.publish(MQTTTOPICSTATE, isPumpRunning ? "ON" : "OFF");
  }
}

void connectMqtt()
{
  while (!client.connected())
  {
    String client_id = "esp32-client-";
    client_id += String(WiFi.macAddress());
    Serial.printf("The client %s connects to the %s mqtt broker\n", client_id.c_str(), MQTTHOSTNAME);
    if (client.connect(client_id.c_str(), MQTTUSERNAME, MQTTPASSWORD))
    {
      Serial.println("Connected to MQTT broker");
    }
    else
    {
      Serial.print("failed with state ");
      Serial.print(client.state());
      delay(2000);
    }
  }
  client.subscribe(MQTTTOPICCONTROL);
}

void setup()
{
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(RELAYPIN, OUTPUT);
  Serial.begin(9600);
  delay(10);
  digitalWrite(LED_BUILTIN, LOW);
  Serial.println('\n');

  WiFi.begin(WIFISSID, WIFIPWD);
  Serial.print("Connecting to ");
  Serial.print(WIFISSID);
  Serial.println(" ...");

  int i = 0;
  while (WiFi.status() != WL_CONNECTED)
  { // Wait for the Wi-Fi to connect
    delay(1000);
    Serial.print(++i);
    Serial.print(' ');
  }

  Serial.println('\n');
  Serial.print("Connection established! IP address:\t");
  Serial.println(WiFi.localIP());

  client.setServer(MQTTHOSTNAME, MQTTPORT);
  client.setCallback(callback);
  connectMqtt();
}

void loop()
{
  client.loop();
  if (!client.connected())
    connectMqtt();
}