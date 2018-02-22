#include <SoftwareSerial.h>
#include "FVlib_Arduino_SIM900_Ubidots_Client.h"
#include "FVlib_SerialConfig.h"
#include "FVlib_OpenLog.h"
#include "FVlib_DS3232RTC_JChristensenFork.h" //"felix_RTC_ds3231.h" 
#include "OneWire.h"
#include <DallasTemperature.h>
#include <SHT1x.h>
#include "MaxBotix_Ultrasonic_Sensor.h"
#include <TimeLib.h>

// Degines generals del programa:
  #define placaArduino 2 // 1:UNO 2:DUE/MEGA
  #define intentsEnviarDadesPerGPRS 5

// Defines referents al patillatge
  #define ONE_WIRE_BUS 11 // Pota on hi han els sensor de temperatura DS18B20
  #define SHT10Exterior_dataPin  52 // Sensor exterior SHT10
  #define SHT10Exterior_clockPin 53 // Sensor exterior SHT10
  #define RxPinMaxBotix 50
  #define TxPinMaxBotix 51
  #define pinSensorCapacitatiu 10

// Definicio d'estructures:
  struct estructuraDades {
    int distancia; // Distancia retornada pel MaxBotix
    float temperatures[3]; // On emagatzamarem les temperatures dels DS18D20
    float tempAmbientalExterior, humiAmbientalExterior; // Temp i Humi del sensor ambiental exterior
    float tempRTC;
    byte capacitatiu;
    int coberturadB;
  };


// Definicio objectes:
  estructuraDades dadesSensors; // Estructura de totes les dades que es recopilen
  sim900 modem1(TOKEN);
  DS3232RTC DS3231;
  tmElements_t tm;
  OneWire oneWire(ONE_WIRE_BUS); // Setup a oneWire instance to communicate with any OneWire devices (not just Maxim/Dallas temperature ICs)
  DallasTemperature sensorTemp(&oneWire); // Pass our oneWire reference to Dallas Temperature.
  SHT1x sht1x(SHT10Exterior_dataPin, SHT10Exterior_clockPin);
  MaxBotix ultrasonicSensor(RxPinMaxBotix, TxPinMaxBotix);
  openLog sd(Serial2, 8, false); 
  serialConfig SC(Serial);

// Variables:
  unsigned long ultimBatec;
  int batecConta = 0;
  unsigned long ultimMillis = 0; // Per contar si ha passat 1 segon (loop)
  byte serialConfigID, serialConfigIdParametre; // Les variables que passem per referencia l'objecte serialConfig

// Debugger:
  #define carregaModem // Si activem això es programa el codi del modem, sino no. Es per fer proves sense modem, per no perdre el temps.
  #define verboseModem false

void setup() {

  Serial.begin(9600); // UART PC
  sensorTemp.begin();
  pinMode(pinSensorCapacitatiu, INPUT_PULLUP);

  #ifdef carregaModem
    Serial.println("Engegant Modem i Port Serie del Modem...");
    while(modem1.begin() != 0);
    Serial.println("Modem i Port Serie engegat");
    while(modem1.setModemOnline() != 0);
  #endif

  // printejaInforme(false, false);
}

//=================================================
//============ INICI LOOP =========================
//=================================================

void loop() {
  byte resultatBuscaComanda;
  int quantsMillis = millis() - ultimMillis;

  if ((quantsMillis > 3000) || (quantsMillis < 0)) {
    ultimMillis = millis();
    enviaInforme(true, true, true); // boolean envial_per_Serail, boolean envial_per_GSM, boolean guarda_log_a_SD
  }


  resultatBuscaComanda = SC.buscaComanda(&serialConfigID, &serialConfigIdParametre);
  if (resultatBuscaComanda == 1) analitzaResultatComanda();
  else if (resultatBuscaComanda == 254) Serial.println("No s'ha trobat cap operador a la comanda");
  else if (resultatBuscaComanda == 255) Serial.println("No s'ha reconegut la comanda");
  


}

//=================================================
//============ FI LOOP ============================
//=================================================
void llegeixRTC() {
  DS3231.read(tm);
  dadesSensors.tempRTC = (DS3231.temperature() / 4.0);
}

void recopilaDades() {
  byte tamanyVector = sizeof(dadesSensors.temperatures) / 4; // Cada float ocupa 4 bytes
  
  Serial.println("\n");
  Serial.print("Recopilant dades...");
  
  // Les DS18B20 (Waterproof)
  sensorTemp.requestTemperatures();
  for (byte contador = 0; contador < tamanyVector; contador++) {
    dadesSensors.temperatures[contador] = sensorTemp.getTempCByIndex(contador);
  }
  // Les SHT10 Exterior:
  dadesSensors.tempAmbientalExterior = sht1x.readTemperatureC();
  dadesSensors.humiAmbientalExterior = sht1x.readHumidity();

  // MaxBotix:
  dadesSensors.distancia = ultrasonicSensor.getDistance();

  // El RTC:
  llegeixRTC();

  // El Microcom:
  dadesSensors.capacitatiu = digitalRead(10);

  // Cobertura:
  dadesSensors.coberturadB = modem1.getSignalStrengthRSSI(3);
}

void enviaInforme(boolean envial_per_Serail, boolean envial_per_GSM, boolean guarda_log_a_SD) {
  String comandaSD; // Construimg la String per operar amb la SD
  char lectura; // char que llegim del UART del OpenLog
  long tamanyFitxer; // guardem el tamany dels fitxers que consultem
  int conta; // Pel bucle de contar numeros de fitxer
  byte resultatLogging; // Resultat que dona la llibreria OpenLog de l'accio d'escriure-hi
  byte intents; // Quants intents fem d'enviar les dades per GPRS
  
  recopilaDades();

  // Enviem les dades al terminal serie Seral:
  if (envial_per_Serail == true) {
    // RTC
    Serial.println("RTC:");
    Serial.print("-> Hora: ");
    Serial.print(tm.Hour, DEC);
    Serial.print(':');
    Serial.print(tm.Minute,DEC);
    Serial.print(':');
    Serial.println(tm.Second,DEC);
    Serial.print("-> Data: ");
    Serial.print(tm.Day, DEC);
    Serial.print('/');
    Serial.print(tm.Month,DEC);
    Serial.print('/');
    Serial.println((tm.Year + 1970),DEC);
    Serial.print("-> Temperatura RTC: ");
    Serial.print(dadesSensors.tempRTC);
    Serial.println(" ºC");
  
    // Temperatures DS18B20, sondes Waterproof:
    Serial.println("Temperatura Sondes de Comparacio (Waterproof):");
    Serial.print("-> Temperatura 0 = ");
    Serial.print(dadesSensors.temperatures[2]);
    Serial.println(" ºC");
    Serial.print("-> Temperatura 1 = ");
    Serial.print(dadesSensors.temperatures[1]);
    Serial.println(" ºC");
    Serial.print("-> Temperatura 2 = ");
    Serial.print(dadesSensors.temperatures[0]);
    Serial.println(" ºC");
  
    // Temperatures SHT10 Exterior
    Serial.println("Sonda Temp./Humi. Exterior Caixa:");
    Serial.print("-> Temperatura Exterior = ");
    Serial.print(dadesSensors.tempAmbientalExterior);
    Serial.println(" ºC");
    Serial.print("-> Humitat Exterior = ");
    Serial.print(dadesSensors.humiAmbientalExterior);
    Serial.println(" %RH");
  
    // Sensor MaxBotix
    Serial.println("Distancia sensor d'ultrasons:");
    Serial.print("-> ");
    Serial.print(dadesSensors.distancia);
    Serial.println(" mm");
  
    // Sensor Microcom 
    Serial.println("Hi ha aigua al sensor capacitatiu:");
    Serial.print("-> ");
    Serial.println(dadesSensors.capacitatiu);

    // Cobertura dB 
    Serial.println("Cobertura (dB):");
    Serial.print("-> ");
    Serial.println(dadesSensors.coberturadB);
  }

// Guardem les dades a la SD del OpenLog:
  if (guarda_log_a_SD == true) {
    Serial.println("\n\nComandaSD:");
    comandaSD = String(String(tm.Day) + "/" + String(tm.Month) + String("/") + (tm.Year + 1970) + "," 
      + tm.Hour + ":" + tm.Minute + ":" + tm.Second + "," + dadesSensors.tempRTC + "," 
      + dadesSensors.temperatures[0] + "," + dadesSensors.temperatures[1] + ',' + dadesSensors.temperatures[2] 
      + ',' + dadesSensors.tempAmbientalExterior + ',' + dadesSensors.humiAmbientalExterior
      + ',' + dadesSensors.distancia + ',' + dadesSensors.capacitatiu + ',' + dadesSensors.coberturadB);
    Serial.println(comandaSD);
    resultatLogging = sd.appendToLastLoggingSession("Mesures", comandaSD, 3500);
    if (resultatLogging == 0) Serial.println("Dades logejades!");
    else if (resultatLogging == 1) Serial.println("ERROR SD!");
    else if (resultatLogging == 2) Serial.println("No hi ha espai a la SD!");
  }

  // Enviem les dades per GSM a Ubidots:
  if (envial_per_GSM == true) {
    comandaSD = "";
    
    //batec();

    // Temperatura CPU RTC:
    for (intents = intentsEnviarDadesPerGPRS; intents != 0; intents--) {
      Serial.print("\n\n Enviant Temperatura CPU RTC. Intents que queden: ");
      Serial.println(intents);
      if (modem1.ubidotsSendData(dadesSensors.tempRTC, UbidotsID_temperatura_CPU_RTC, verboseModem) != 0) {
        modem1.begin();
        modem1.setModemOnline();
        Serial.println("SIM900 Err al enviar Temp CPU RTC");
      }
      else {
        break;
      }  
    }
    llegeixRTC();
    comandaSD = String(comandaSD + String(tm.Day) + "/" + String(tm.Month) + String("/") + (tm.Year + 1970) + "," 
      + tm.Hour + ":" + tm.Minute + ":" + tm.Second + "," + String(intents) + ',');
    
    
    // Temperatura Comparacio 0:
    for (intents = intentsEnviarDadesPerGPRS; intents != 0; intents--) {
      Serial.print("\n\n Enviant TEMP.0. Intents que queden: ");
      Serial.println(intents);
      if (modem1.ubidotsSendData(dadesSensors.temperatures[2], UbidotsID_temp_comp_0, verboseModem) != 0) {
        modem1.begin();
        modem1.setModemOnline();
        Serial.println("SIM900 Err al enviar TEMP.0");
      }
      else {
        break;
      }
    }
    llegeixRTC();
    comandaSD = String(comandaSD + String(tm.Day) + "/" + String(tm.Month) + String("/") + (tm.Year + 1970) + "," 
      + tm.Hour + ":" + tm.Minute + ":" + tm.Second + "," + String(intents) + ',');
    
    // Temperatura Comparacio 1:
    for (intents = intentsEnviarDadesPerGPRS; intents != 0; intents--) {
      Serial.print("\n\n Enviant TEMP.1. Intents que queden: ");
      Serial.println(intents);
      if (modem1.ubidotsSendData(dadesSensors.temperatures[1], UbidotsID_temp_comp_1, verboseModem) != 0) {
        modem1.begin();
        modem1.setModemOnline();
        Serial.println("SIM900 Err al enviar TEMP.1");
      }
      else {
        break;
      }
    }
    llegeixRTC();
    comandaSD = String(comandaSD + String(tm.Day) + "/" + String(tm.Month) + String("/") + (tm.Year + 1970) + "," 
      + tm.Hour + ":" + tm.Minute + ":" + tm.Second + "," + String(intents) + ',');
    
    // Temperatura Comparacio 2:
    for (intents = intentsEnviarDadesPerGPRS; intents != 0; intents--) {
      Serial.print("\n\n Enviant TEMP.2. Intents que queden: ");
      Serial.println(intents);
      if (modem1.ubidotsSendData(dadesSensors.temperatures[0], UbidotsID_temp_comp_2, verboseModem) != 0) {
        modem1.begin();
        modem1.setModemOnline();
        Serial.println("SIM900 Err al enviar TEMP.2");
      }
      else {
        break;
      }
    }
    llegeixRTC();
    comandaSD = String(comandaSD + String(tm.Day) + "/" + String(tm.Month) + String("/") + (tm.Year + 1970) + "," 
      + tm.Hour + ":" + tm.Minute + ":" + tm.Second + "," + dadesSensors.tempRTC + "," + String(intents) + ',');

    // Temperatura Ambiental exterior
    for (intents = intentsEnviarDadesPerGPRS; intents != 0; intents--) {
      Serial.print("\n\n Enviant Temperatura Ambiental Exterior. Intents que queden: ");
      Serial.println(intents);
      if (modem1.ubidotsSendData(dadesSensors.tempAmbientalExterior, UbidotsID_temperatura_ambiental_exterior_caixa, verboseModem) != 0) {
        modem1.begin();
        modem1.setModemOnline();
        Serial.println("SIM900 Err al enviar Temperatura Ambiental Exterior");
      }
      else {
        break;
      }
    }
    llegeixRTC();
    comandaSD = String(comandaSD + String(tm.Day) + "/" + String(tm.Month) + String("/") + (tm.Year + 1970) + "," 
      + tm.Hour + ":" + tm.Minute + ":" + tm.Second + "," + String(intents) + ',');
    
    // Humitat Ambiental Exterior
    for (intents = intentsEnviarDadesPerGPRS; intents != 0; intents--) {
      Serial.print("\n\n Enviant Humitat Ambiental Exterior. Intents que queden: ");
      Serial.println(intents);
      if (modem1.ubidotsSendData(dadesSensors.humiAmbientalExterior, UbidotsID_humitat_ambiental_exterior_caixa, verboseModem) != 0) {
        modem1.begin();
        modem1.setModemOnline();
        Serial.println("SIM900 Err al enviar Humitat Ambiental Exterior");
      }
      else {
        break;
      }
    }
    llegeixRTC();
    comandaSD = String(comandaSD + String(tm.Day) + "/" + String(tm.Month) + String("/") + (tm.Year + 1970) + "," 
      + tm.Hour + ":" + tm.Minute + ":" + tm.Second + "," + String(intents) + ',');
    
    // Distancia Maxbotics
    for (intents = intentsEnviarDadesPerGPRS; intents != 0; intents--) {
      Serial.print("\n\n Enviant Distancia Maxbotics. Intents que queden: ");
      Serial.println(intents);
      if (modem1.ubidotsSendData(dadesSensors.distancia, UbidotsID_distancia_maxbotix, verboseModem) != 0) { 
        modem1.begin();
        modem1.setModemOnline();
        Serial.println("SIM900 Err al enviar distancia maxbotix");
      }
      else {
        break;
      }
    }
    llegeixRTC();
    comandaSD = String(comandaSD + String(tm.Day) + "/" + String(tm.Month) + String("/") + (tm.Year + 1970) + "," 
      + tm.Hour + ":" + tm.Minute + ":" + tm.Second  + "," + String(intents) + ',');

    // Sensor capacitatiu
    for (intents = intentsEnviarDadesPerGPRS; intents != 0; intents--) {
      Serial.print("\n\n Enviant Sensor Capacitatiu. Intents que queden: ");
      Serial.println(intents);
      if (modem1.ubidotsSendData(dadesSensors.capacitatiu, UbidotsID_sensor_capacitatiu, verboseModem) != 0) { 
        modem1.begin();
        modem1.setModemOnline();
        Serial.println("SIM900 Err al enviar Capacitatiu");
      }
      else {
        break;
      }
    }
    llegeixRTC();
    comandaSD = String(comandaSD + String(tm.Day) + "/" + String(tm.Month) + String("/") + (tm.Year + 1970) + "," 
      + tm.Hour + ":" + tm.Minute + ":" + tm.Second + "," + String(intents) + ',');
    
    // Logging SD
    for (intents = intentsEnviarDadesPerGPRS; intents != 0; intents--) {
      Serial.print("\n\n Enviant Errors d'escriptura en SD. Intents que queden: ");
      Serial.println(intents);
      if (modem1.ubidotsSendData(resultatLogging, UbidotsID_error_SD, verboseModem) != 0) { 
        modem1.begin();
        modem1.setModemOnline();
        Serial.println("SIM900 Err al escriure en SD");
      }
      else {
        break;
      }  
    }
    llegeixRTC();
    comandaSD = String(comandaSD + String(tm.Day) + "/" + String(tm.Month) + String("/") + (tm.Year + 1970) + "," 
      + tm.Hour + ":" + tm.Minute + ":" + tm.Second + "," + String(intents) + ',');

    // cobertura
    for (intents = intentsEnviarDadesPerGPRS; intents != 0; intents--) {
      Serial.print("\n\n Enviant cobertura modem. Intents que queden: ");
      Serial.println(intents);
      if (modem1.ubidotsSendData(dadesSensors.coberturadB, UbidotsId_coberturadB, verboseModem) != 0) { 
        modem1.begin();
        modem1.setModemOnline();
        Serial.println("SIM900 Err al enviar cobertura");
      }
      else {
        break;
      }  
    }
    llegeixRTC();
    comandaSD = String(comandaSD + String(tm.Day) + "/" + String(tm.Month) + String("/") + (tm.Year + 1970) + "," 
      + tm.Hour + ":" + tm.Minute + ":" + tm.Second + "," + String(intents) + ',');
      
    Serial.println("");

       
    Serial.println(comandaSD);
    resultatLogging = sd.appendToLastLoggingSession("Sistema", comandaSD, 3500);
    if (resultatLogging == 0) Serial.println("Dades logejades!");
    else if (resultatLogging == 1) Serial.println("ERROR SD!");
    else if (resultatLogging == 2) Serial.println("No hi ha espai a la SD!");
  }
}

void batec() {
  if ((millis() - ultimBatec) > (60000)) {
    ultimBatec = millis();
    if (modem1.ubidotsSendData(batecConta++, UbidotsID_batec, true) != 0) {
      modem1.begin();
      modem1.setModemOnline();
      Serial.println("XXXXXXXXXX MODEM OFFILNE XXXXXXXXXX");
    }
    Serial.print("Batec# ");
    Serial.println(batecConta);
  }
}

// S'executa despres del SC.buscaComanda(), i en funcio del que retorna, farem una cosa o una altra:
void analitzaResultatComanda() {
  recopilaDades(); // Recopilem les dades dels sensors i les carreguem a les variables.
  
  if (serialConfigID == 0) {
    Serial.println("RTC.hora OK");
    if (serialConfigIdParametre != 255) {
      tm.Hour = serialConfigIdParametre;
      Serial.print("Parametre rebut: ");
      DS3231.write(tm);
    }
    Serial.println(tm.Hour);
  }
  else if (serialConfigID == 1) {
    Serial.println("RTC.minut OK");
    if (serialConfigIdParametre != 255) {
      tm.Minute = serialConfigIdParametre;
      Serial.print("Parametre rebut: ");
      DS3231.write(tm);
    }
    Serial.println(tm.Minute);
  }
  else if (serialConfigID == 2) {
    Serial.println("RTC.dia OK");
    if (serialConfigIdParametre != 255) {
      tm.Day = serialConfigIdParametre;
      Serial.print("Parametre rebut: ");
      DS3231.write(tm);
    }
    Serial.println(tm.Day);
  }
  else if (serialConfigID == 3) {
    Serial.println("RTC.mes OK");
    if (serialConfigIdParametre != 255) {
      tm.Month = serialConfigIdParametre;
      Serial.print("Parametre rebut: ");
      DS3231.write(tm);
    }
    Serial.println(tm.Month);
  }
  else if (serialConfigID == 4) {
    Serial.println("RTC.any OK");
    if (serialConfigIdParametre != 255) {
      tm.Year = (serialConfigIdParametre + 2000) - 1970;
      Serial.print("Parametre rebut: ");
      DS3231.write(tm);
    }
    Serial.println(tm.Year + 1970);
  }
  else if (serialConfigID == 5) {
    Serial.println("TEMP.0 OK");
    Serial.println(dadesSensors.temperatures[0]);
  }
  else if (serialConfigID == 6) {
    Serial.println("TEMP.1 OK");
    Serial.println(dadesSensors.temperatures[1]);
  }
  else if (serialConfigID == 7) {
    Serial.println("TEMP.exterior OK");
    Serial.println(dadesSensors.tempAmbientalExterior);
  }
  else if (serialConfigID == 8) {
    Serial.println("HUMI.exterior OK");
    Serial.println(dadesSensors.humiAmbientalExterior);
  }
  else if (serialConfigID == 9) {
    Serial.println("ULTRASO.distancia OK");
    Serial.println(dadesSensors.distancia);
  }
  else if (serialConfigID == 10) {
    Serial.println("infrome OK");
    enviaInforme(true, false, false);
  }
  else if (serialConfigID == 11) { // RTC.temp
    Serial.println("RTC.temp OK");
    Serial.print(dadesSensors.tempRTC);
    Serial.println(" ºC");
  }
  else if (serialConfigID == 12) { // CAPACITATIU.hihacontacte
    Serial.println("CAPACITATIU.hihacontacte OK");
    Serial.print(dadesSensors.capacitatiu);
  }
  else if (serialConfigID == 13) { // TMP.2
    Serial.println("TEMP.2 OK");
    Serial.println(dadesSensors.temperatures[2]);
  }
  else {
    Serial.println("Comanda no carregada!");
  }
}

