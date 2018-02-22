REUNIO FELIX 22/2/2018 A SANT SADURNI
=====================================

- Sabata (capacitiu, microcom):
  mesura si hi ha aigua tocant al sensor.
  no hi ha llibreria (on/off de digitalRead)

- Nivell: maxbotix.
  Envia caràcters ascii per exemple "12 " en cm
  Serial.read()

- Temperatura (DS18B20):
  va amb una llibreria d'arduino (DallasTemperature.h)
  dins de l'arduino ide:
  - sketch / include library / add .zip library
  - [http://github.com/milesburton/arduino-temperature-control-library][github]

