#include <PubSubClient.h>
#include <ESP8266WiFi.h>

#define LED (2)
#define MAX_MSG_LEN (128)

/* WiFi config */
const char* ssid = "wifi ssid";
const char* password = "wifi password";

// const char *serverHostname = "207.154.218.89";
const IPAddress serverIPAddress(207, 154, 218, 89);

const char *topic = "led";

WiFiClient espClient;
PubSubClient client(espClient);

void setup()
{
	pinMode(LED, OUTPUT);
	digitalWrite(LED, HIGH);

	/* config debugging serial port */
	Serial.begin(115200);

	/* init wifi connection. will wait until connected */
	connectWifi();

	/* connect to mqtt server */
	client.setServer(serverIPAddress, 1883);
	client.setCallback(callback);
}

void loop()
{
	if (!client.connected()) {
		connectMQTT();
	}

	client.loop();

	/* idle */
	delay(500);
}

void connectWifi() {
	delay(10);

	/* connecting to wifi network */
	Serial.printf("\nConnecting to %s\n", ssid);
	WiFi.begin(ssid, password);
	while(WiFi.status() != WL_CONNECTED){
		delay(250);
		Serial.print(".");
	}
	Serial.println("");
	Serial.print("WiFi connected on IP addr ");
	Serial.println(WiFi.localIP());
}

void connectMQTT() {
	while (!client.connected()){
		String clientid = "ESP8266-";
		clientid += String(random(0xffff), HEX);

		Serial.printf("MQTT connecting as client %s...\n", clientid.c_str());
		if (client.connect(clientid.c_str())) {
			Serial.println("MQTT connected");

			client.publish(topic, "hello from NodeMCU");
			client.subscribe(topic);
		} else {
			Serial.printf("MQTT failed, state %s, retrying...\n", client.state());
			delay(2500);
		}
	}
}

/* as defined by the PubSubClient library */
void callback(char *msgTopic, byte *msgPayload, unsigned int msgLength){
	static char message[MAX_MSG_LEN+1];
	if(msgLength > MAX_MSG_LEN) {
		msgLength = MAX_MSG_LEN;
	}

	strncpy(message, (char *)msgPayload, msgLength);
	message[msgLength] = '\0';

	Serial.printf("topic %s, message recieved %s\n", topic, message);

	/* decode message */
	if (strcmp(message, "off") == 0) {
		setLedState(false);
	} else if (strcmp(message, "on") == 0) {
		setLedState(true);
	}
}

void setLedState(boolean state) {
	/* LED logic is inverted, low means on */
	digitalWrite(LED, !state);
}