# Recerca per configurar gateway LoRaWAN a ICRA

## 0. Aconseguit
- 23/11/2017: enviar bytes amb seeeduino lorawan via ABP amb comandes AT des de l'arduino ide:
	'''
		at+id
		at+id=appeui,"00 00 00 00 00 00 00 01"
		at+mode=lwabp
		at+cmsghex="00 01 02 03 04 05 06 07 08 09 10"
	'''

## 1. Links
* model gateway que tenim: rhf0m31
http://www.risinghf.com/product/rhf0m301/?lang=en
	* user manual (fet)
	https://github.com/SeeedDocument/LoRaWAN_Gateway-868MHz_Kit_with_Raspberry_Pi_3/raw/master/res/%5BRHF-UM01649%5DIoT%20Discovery%20User%20Manual-seeed-v2.1.pdf
	* forum sobre rhf0m31 a la web TTN (no llegit)
	https://www.thethingsnetwork.org/forum/t/has-anyone-tried-the-risinghf-gateway-boards/3281/9

* model gw molt utilitzat a TTN: ic880
https://wireless-solutions.de/products/radiomodules/ic880a
	* ic880 configuració (gonzalo casas)
	https://github.com/ttn-zh/ic880a-gateway/wiki

* model gw libelium waspmote (model que fan servir EAWAG)
http://www.libelium.com/products/waspmote/

* bateria
https://www.cooking-hacks.com/6600ma-h-rechargeable-battery

* seeeduino lorawan (en tenim 3): pels sensors
https://www.seeedstudio.com/Seeeduino-LoRaWAN-p-2780.html

## 2. Paràmetres necessaris per configurar xarxa
	- DEVICE EUI
		- 8 bytes
		- Set by the user. If not, the preprogrammed is used (00-00-00-00-00-00-00-00).
		- Example for user-provided device EUI:
			LoRaWAN.setDeviceEUI('0102030405060708');

	- DEVICE ADDRESS
		- 4 bytes (0 to ff-ff-ff-ff)
		- has to be unique inside network
		- if not set, the last 4 bytes of device EUI are used.
		- exemple:
			LoRaWAN.setDeviceAddr('01020304');

	- APPLICATION SESSION KEY
		- 16 bytes (128 bits) AES algorithm
		- each end device has its own unique app session key
		- is a secret key: only known by end devices and application server
		- exemple:
			LoRaWAN.setAppSessionKey(“00102030405060708090A0B0C0D0E0F”);

	- NETWORK SESSION KEY
		- 16 bytes
		- set by user
		- obtinguda a partir de la App Sess Key
		- each end device has its own NSK, only known by the device and network server.
		- exemple:
			LoRaWAN.setNwkSessionKey(“00102030405060708090A0B0C0D0E0F”);

	- APPLICATION EUI
		- 8 bytes
		- set by user
		- global application identifier
		- exemple:
			LoRaWAN.setAppEUI(“1112131415161718”);

	- APPLICATION KEY
		- 16 bytes
		- set by user
		- Whenever an end-device joins a network via OTAA, the Application Key is used to derive the session keys, Network Session Key and Application Session Key, which are speci c for that end-device to encrypt and verify network communication and application data.
		- exemple:
			LoRaWAN.setAppKey(“0102030405060708090A0B0C0D0E0F”);

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
		LoRaWAN.joinOTAA();

### 3.2. ABP
	Each module should have a unique set of Network Session Key and Application Session Key.
	The ABP join procedure requires the module to be personalized with the following information before its starts the join procedure:
	- Device address (32-bit)
	- Network Session Key (128-bit key) ensures security on network level
	- Application Session Key (128-bit key) ensures end-to-end security on application level

	- join:
		LoRaWAN.joinABP();

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
