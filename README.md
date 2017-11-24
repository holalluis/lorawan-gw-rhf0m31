# Recerca per configurar gateway LoRaWAN a ICRA (model rhf0m31)

Esquema aparells connectats
```
	+--------+-----------+           +---------+--------------+----------+
	| Sensor | Seeeduino | ~~LoRa~~> | Gateway | Raspberry Pi | Internet |
	+--------+-----------+           +---------+--------------+----------+
```

## 0. Aconseguit
- Enviats al gateway 4 bytes des del Seeeduino (via ABP) amb comandes AT, des del monitor serial de l'Arduino IDE:
	- Sketch arduino per enviar-li comandes AT:
	```c++
		void setup() {
			Serial1.begin(9600);
			SerialUSB.begin(115200);
		}

		void loop() {
			while(Serial1.available()) {
				SerialUSB.write(Serial1.read());
			}
			while(SerialUSB.available()) {
				Serial1.write(SerialUSB.read());
			}
		}
	```
	- Comandes AT per configuració:
	```
		at+ver
		at+id //necessari per veure les adreces per configurar el seeeduino dins el gateway
		at+mode=lwabp
		at+id=appeui,"00 00 00 00 00 00 00 01"
	```
	- Comanda AT per enviar 4 bytes de prova:
	```
		at+cmsghex="00 01 02 03"
	```
- Enviat string "Hello World" amb la llibreria LoRaWAN.h d'arduino
	```c++
		#include <LoRaWan.h>
		void setup(void) {
			lora.init();
			lora.getVersion(buffer, 256, 1);
			lora.getId(buffer, 256, 1);
			lora.setKey("2B7E151628AED2A6ABF7158809CF4F3C", "2B7E151628AED2A6ABF7158809CF4F3C", "2B7E151628AED2A6ABF7158809CF4F3C");
			lora.setDeciveMode(LWABP);
			lora.setDataRate(DR0, EU868);
			lora.setChannel(0, 868.1);
			lora.setChannel(1, 868.3);
			lora.setChannel(2, 868.5);
			lora.setReceiceWindowFirst(0, 868.1);
			lora.setReceiceWindowSecond(869.5, DR3);
			lora.setDutyCycle(false);
			lora.setJoinDutyCycle(false);
			lora.setPower(14);
		}

		void loop(void) {   
			bool result = false;
			result = lora.transferPacket("Hello World!", 10);
		}
	```

## 1. Links
* model gateway que tenim: rhf0m31
http://www.risinghf.com/product/rhf0m301/?lang=en
	* user manual (serveix per configurar el raspberry+gateway)(tot fet menys la part de l'empresa Loriot)
	https://github.com/SeeedDocument/LoRaWAN_Gateway-868MHz_Kit_with_Raspberry_Pi_3/raw/master/res/%5BRHF-UM01649%5DIoT%20Discovery%20User%20Manual-seeed-v2.1.pdf
	* forum sobre rhf0m31 a TTN.org (no llegit)
	https://www.thethingsnetwork.org/forum/t/has-anyone-tried-the-risinghf-gateway-boards/3281/9

* seeeduino lorawan (en tenim 3) (és l'aparell que envia les dades al gateway)
	* aparell
	https://www.seeedstudio.com/Seeeduino-LoRaWAN-p-2780.html
	* instruccions
	http://wiki.seeed.cc/Seeeduino_LoRAWAN/

* Altres aparells (que no tenim)
	* model gw molt utilitzat a TTN: ic880 (possible compra)
		* aparell
		https://wireless-solutions.de/products/radiomodules/ic880a
		* configuració (gonzalo casas)
		https://github.com/ttn-zh/ic880a-gateway/wiki
	* model gw libelium waspmote (model que fan servir EAWAG) (possible compra)
	http://www.libelium.com/products/waspmote/


## 2. Paràmetres necessaris per configurar xarxa
nota: els exemples de codi corresponen al model Waspmote, no al rhf0m31
- DEVICE EUI
	- 8 bytes
	- Set by the user. If not, the preprogrammed is used (00-00-00-00-00-00-00-00).
	- Example for user-provided device EUI:
		```
		setDeviceEUI('0102030405060708');
		```

- DEVICE ADDRESS
	- 4 bytes (0 to ff-ff-ff-ff)
	- has to be unique inside network
	- if not set, the last 4 bytes of device EUI are used.
	- exemple:
		```
		setDeviceAddr('01020304');
		```

- APPLICATION SESSION KEY
	- 16 bytes (128 bits) AES algorithm
	- each end device has its own unique app session key
	- is a secret key: only known by end devices and application server
	- exemple:
		```
		setAppSessionKey(“00102030405060708090A0B0C0D0E0F”);
		```

- NETWORK SESSION KEY
	- 16 bytes
	- set by user
	- obtinguda a partir de la App Sess Key
	- each end device has its own NSK, only known by the device and network server.
	- exemple:
		```
		setNwkSessionKey(“00102030405060708090A0B0C0D0E0F”);
		```

- APPLICATION EUI
	- 8 bytes
	- set by user
	- global application identifier
	- exemple:
		```
		setAppEUI(“1112131415161718”);
		```

- APPLICATION KEY
	- 16 bytes
	- set by user
	- Whenever an end-device joins a network via OTAA, the Application Key is used to derive the session keys, Network Session Key and Application Session Key, which are speci c for that end-device to encrypt and verify network communication and application data.
	- exemple:
		```
		setAppKey(“0102030405060708090A0B0C0D0E0F”);
		```

## 3. Activació d'una xarxa
To participate in a LoRaWAN network, each module has to be personalized and activated.
Activation of a module can be achieved in two ways, 
either via Over-The-Air Activation (OTAA) when an end-device is deployed or reset, 
or via Activation By Personalization (ABP) 
in which the two steps of end-device personalization and activation are done as one step.

### 3.1. OTAA
The OTAA join procedure requires the module to be personalized with the following information before its starts the join procedure:
- Device EUI      ( 64 bit ==  8 byte)
- Application EUI ( 64 bit ==  8 byte)
- Application Key (128 bit == 16 byte)

After joining through OTAA, the module and the network exchanged the Network Session Key and the Application Session Key which are needed to perform communications.

- join:
	```
	LoRaWAN.joinOTAA();
	```

### 3.2. ABP
Each module should have a unique set of Network Session Key and Application Session Key.
The ABP join procedure requires the module to be personalized with the following information before its starts the join procedure:
- Device address (32-bit)
- Network Session Key (128-bit key) ensures security on network level
- Application Session Key (128-bit key) ensures end-to-end security on application level
- join:
	```
	LoRaWAN.joinABP();
	```

## 4. Enviar dades al gateway

### 4.1. Send unconfirmed data (sense ACK del servidor)
Example of use:
{
	uint8_t port = 1;
	char data[]  = “010203040506070809”;
	LoRaWAN.sendUnconfirmed(port, data);
}
	
### 4.2. Send confirmed data (amb ACK del servidor)
Example of use:
{
	uint8_t port = 1;
	char data[]  = “010203040506070809”;
	LoRaWAN.sendConfirmed(port, data);
}
