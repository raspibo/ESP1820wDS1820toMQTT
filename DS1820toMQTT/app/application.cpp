#include <user_config.h>
#include <SmingCore/SmingCore.h>
#include <Libraries/OneWire/OneWire.h>

#include <Libraries/DS18S20/ds18s20.h>


// If you want, you can define WiFi settings globally in Eclipse Environment Variables
#ifndef WIFI_SSID
	//#define WIFI_SSID "PleaseEnterSSID" // Put you SSID and Password here
	//#define WIFI_PWD "PleaseEnterPass"
#endif

// ... and/or MQTT username and password
#ifndef MQTT_USERNAME
	#define MQTT_USERNAME ""
	#define MQTT_PWD ""
#endif

// ... and/or MQTT host and port
#ifndef MQTT_HOST
	#define MQTT_HOST "level1"
	#define MQTT_PORT 1883
#endif

/* (My) User settings */
#define MQTT_PUBLISH "I/Mobile/Mobile/Mobile/Temperatura"	// Percorso di pubblicazione dei dati MQTT
#define MQTT_SUBSCRIBE "O/Mobile/Mobile/Mobile/#"	// Percorso di lettura dei dati MQTT
//#define MQTT_ID "DS18_20"	// Identificatore, che non usero`, usero` la sua stringa se posso.

#define INTPIN 2   // GPIO 2 // DS18_20


// Forward declarations
void startMqttClient();
//void onMessageReceived(String topic, String message);	// Non la uso
void publishMessage();	// Dicharazione

DS18S20 ReadTemp;
Timer procTimer;

// MQTT client
// For quick check you can use: http://www.hivemq.com/demos/websocket-client/ (Connection= test.mosquitto.org:8080)
//MqttClient mqtt(MQTT_HOST, MQTT_PORT, onMessageReceived);
MqttClient mqtt(MQTT_HOST, MQTT_PORT);

/*	Ho disattivato queste parti di reconnect, credo non ofssero presenti nella vecchia versione
// Check for MQTT Disconnection
void checkMQTTDisconnect(TcpClient& client, bool flag){
	
	// Called whenever MQTT connection is failed.
	if (flag == true)
		Serial.println("MQTT Broker Disconnected!!");
	else
		Serial.println("MQTT Broker Unreachable!!");
	
	// Restart connection attempt after few seconds
	procTimer.initializeMs(2 * 1000, startMqttClient).start(); // every 2 seconds
	
}
*/

// Publish our message
void publishMessage()
{
	// Definisco variabili
	String MQTT_ID="";
	String Message="";
	// Adesso copio tutta la "void readData" del DS1820 (esempio Sming)
	uint8_t a;
	uint64_t info;

	if (!ReadTemp.MeasureStatus())  // the last measurement completed
	{
      if (ReadTemp.GetSensorsCount())   // is minimum 1 sensor detected ?
		Serial.println("******************************************");
	    Serial.println(" Reading temperature DEMO");
	    for(a=0;a<ReadTemp.GetSensorsCount();a++)   // prints for all sensors
	    {
	      Serial.print(" T");
	      Serial.print(a+1);
	      int NrT=a+1;				//
		  MQTT_ID = "ST"+String(NrT);	//
	      Serial.print(" = ");
	      if (ReadTemp.IsValidTemperature(a))   // temperature read correctly ?
	        {
			  Message=ReadTemp.GetCelsius(a);
	    	  Serial.print(Message);	// ho aggiunto "Message="
	    	  Serial.print(" Celsius, (");
	    	  Serial.print(ReadTemp.GetFahrenheit(a));
	    	  Serial.println(" Fahrenheit)");
	        }
	      else
			{
	    	  Serial.println("Temperature not valid");
			  Message="err";	// Scrivo err (dovrei interpretarlo correttamente mi sembra .. verificare)
			}

	      Serial.print(" <Sensor id.");

	      info=ReadTemp.GetSensorID(a)>>32;
	      Serial.print((uint32_t)info,16);
	      Serial.print((uint32_t)ReadTemp.GetSensorID(a),16);
	      Serial.println(">");
		  // Mi sembra di capire che ora devo inserire l'invio del dato MQTT
			if (mqtt.getConnectionState() != eTCS_Connected)
				startMqttClient(); // Auto reconnect

			Serial.println("Let's publish message now!");
			// "{ \"ID\" : \"" + String(ID) + "\", \"Valore\" : \"" + String(Message) + "\" }"
			//mqtt.publish(MQTT_PUBLISH, "{ \"ID\" : \"" + String(MQTT_ID) + "\", \"Valore\" : \"" + String(Message) + "\" }"); // or publishWithQoS
			mqtt.publish(MQTT_PUBLISH, "{ \"ID\" : \"" + MQTT_ID + "\", \"Valore\" : \"" + String(Message) + "\" }"); // or publishWithQoS
			Serial.println(String(MQTT_PUBLISH) + " ID:" + String(MQTT_ID) + " Valore:" + String(Message));
	    }
		Serial.println("******************************************");
		ReadTemp.StartMeasure();  // next measure, result after 1.2 seconds * number of sensors
	}
	else
		Serial.println("No valid Measure so far! wait please");


}

/*	Non la uso, l'ho disattivata
// Callback for messages, arrived from MQTT server
void onMessageReceived(String topic, String message)
{
	Serial.print(topic);
	Serial.print(":\r\n\t"); // Pretify alignment for printing
	Serial.println(message);
}
*/

// Run MQTT client
void startMqttClient()
{
	//procTimer.stop();									// Questa non c'era
	if(!mqtt.setWill("last/will","The connection from this device is lost:(", 1, true)) {
		debugf("Unable to set the last will and testament. Most probably there is not enough memory on the device.");
	}
	mqtt.connect("esp8266", MQTT_USERNAME, MQTT_PWD);
	// Assign a disconnect callback function
	//mqtt.setCompleteDelegate(checkMQTTDisconnect);	// Questa non c'era
	//mqtt.subscribe(MQTT_SUBSCRIBE);					// Questa non mi serve

}

// Will be called when WiFi station was connected to AP
void connectOk()
{
	Serial.println("I'm CONNECTED");

	// Run MQTT client
	Serial.println("Start MQTT client ...");
	startMqttClient();

	// Start publishing loop
	procTimer.initializeMs(60 * 1000, publishMessage).start(); // every 60 seconds
}

// Will be called when WiFi station timeout was reached
void connectFail()
{
	Serial.println("I'm NOT CONNECTED. Need help :(");

	// .. some you code for device configuration ..
}


void init()
{
	Serial.begin(SERIAL_BAUD_RATE); // 115200 by default
	Serial.systemDebugOutput(true); // Debug output to serial

	WifiStation.config(WIFI_SSID, WIFI_PWD);
	WifiStation.enable(true);
	WifiAccessPoint.enable(false);

	// Sonda/e temperatura
    ReadTemp.Init(INTPIN);  			// select PIN It's required for one-wire initialization!
	ReadTemp.StartMeasure(); // first measure start,result after 1.2 seconds * number of sensors
	//


	// Run our method when station was connected to AP (or not connected)
	WifiStation.waitConnection(connectOk, 20, connectFail); // We recommend 20+ seconds for connection timeout at start

}
