# LoRaWan recerca configuració gateway

## Links
	- model gateway que tenim: rhf0m31
	[http://www.risinghf.com/product/rhf0m301/?lang=en](a)


	- forum sobre rhf0m31 a la web TTN
	https://www.thethingsnetwork.org/forum/t/has-anyone-tried-the-risinghf-gateway-boards/3281/9

	- model gw estandar a TTN: ic880
	https://wireless-solutions.de/products/radiomodules/ic880a

	- ic880 configuració (gonzalo casas)
	https://github.com/ttn-zh/ic880a-gateway/wiki

	- model gw libelium waspmote
	http://www.libelium.com/products/waspmote/

## Paràmetres necessaris per configurar xarxa
	- device EUI
		- 8 bytes
		- Set by user, if not, the preprogrammed EUI is used.
		- Example for user-provided device EUI:
			{
				LoRaWAN.setDeviceEUI('0102030405060708');
				LoRaWAN.getDeviceEUI();
			}

	- device address
	- application session key
	- network session key
	- application EUI
	- application key


