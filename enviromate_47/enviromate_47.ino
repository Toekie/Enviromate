/* ====================================================================================
  Connect the following ML8511 breakout board to Arduino:
   UV       Arduino
  3.3V =    3.3V
  OUT  =     A0
  GND  =    GND
  EN   =    3.3V

  dht op pin A0

  UVOUT = A0; //Output from the sensor
  3V3 pin verbinden met A1;

  instelingen via seriele poort
   debug=off        | Zet seriele communicatie uit
   timeset=12:00:00 | zet de RTC op 12:00:00
   fanpwm1=100      | stel PWM voor fan 1, 2 of 3 in op 100
   ledpwm1=100      | stel PWM voor led 1, 2 of 3 in op 100
   fog1=12:00-13:00 | stel timer fogger 1, 2 of 3  in
   spray_t1=10:00   | stel spray timer 1, 2 of 3 in op 10:00
   spray_d1=20      | stel spray 1, 2 of 3 duration 20 sec
   test             | spray aan gedurende 10 sec of tot commando test

  onoff commando's, 1=aan, 0=uit
   onoff_led1=1,   onoff_led1 0
   onoff_fog1=1,   onoff_fog1 0
   onoff_fan1=1,   onoff_fan1 0
   onoff_spray1=1, onoff_spray1 0
   onoff_uv=1,     onoff_uv 0
   onoff_warmte=1, onoff_warmte 0



  ===========================================================================================*/
#include <avr/wdt.h>
#include <TimeLib.h>
#include <Dusk2Dawn.h>
#include <DS3231.h>
#include <EEPROM.h>
#include "DHT.h"

DHT dht;

#define rood "63488"
#define wit  "65535"
#define zwart "0"
#define groen "1024"
#define geel  "65125"

#define LEDpinA    10
#define LEDpinB    11
#define LEDpinC    12
#define pinFogger   2
#define pinFanA     5
#define pinFanB     6
#define pinFanC     7
#define pinRelaisA 22
#define pinRelaisB 24
#define pinRelaisC 26
#define pinRelaisD 28
#define pinBeeper  31

#define LEDlicht    0

#define heat_lamp  23
#define UV_lamp    25
#define sprayPomp  27


/* EEPROM adressen */
#define mem_timers  1
#define mem_spray  45
#define mem_PWM    65
#define mem_dht    85
#define mem_coord  95
#define mem_UVcal 115

/* initiële tijdwaarden */
#define LED_OnA "08:00-20:18"
#define UV      "08:00-18:00"
#define Warmte  "08:00-18:00"
#define tsw_3   "10:00-11:00"
#define tsw_4   "12:00-13:00"
#define tsw_5   "14:00-15:00"
#define tsw_6   "16:00-17:00"
#define tsw_7   "17:00-18:00"
#define tsw_8   "18:00-19:00"
#define tsw_9   "19:00-20:00"

/* contole of er al gegevens in de eeprom zijn opgeslagen
door de waarde van check te veranderen zullen na reboot de EEPROM waarden
opnieuw gesaved worden met de deafult waarden.
*/
#define check 28

word LDRdrempel = 600;
word displayLight = 50;
word lastdisplayLight;

//pin definitions UVI module
int UVOUT = A0; //Output from the sensor
int REF_3V3 = A1; //3.3V power on the Arduino board
float UVIcal = 0.92;
float minUVindx = 28.0;
float maxUVindx = 30.0;

boolean testled = false;
boolean Debug = true;
word debugcounter = 0;
// schakelt seriele communicatie uit indien niet gebruikt
# define debugtimeout 3600

//String TijdStr = "";
//String DatumStr = "";
byte zomer = 0; // 0 = winter, 1 = zomer
boolean autozomertijd = true;

// Init the DS3231 using the hardware interface
DS3231  rtc(SDA, SCL);
Time  t;

byte nextionvenster = 0; // beginscherm nextiondisplay

boolean zonUpDown = true; // auto led verlichting

byte DHTcount = 30; // DHT interval in sec
float humidity;     //Stores humidity value
float temperature;  //Stores temperature value
float uvIndex;
float mintemp = 18.0;
float maxtemp = 24.0;
byte DHTalarm = 0;
byte UVIalarm = 0;
boolean beep = false;

word lastSecond; // tbv secondenloop
word tijdcode;   // huidige tijd in minuten tov middernacht

String latitude = "51.977901";
String longitude = "5.673233";
String zonOp;    // tijdstip zon op in stringformaat
String zonOnder; // tijdstip zon onder in stringformaat

byte infrarood = 0; // status warmtelamp. 1 = aan, 0 = uit
byte UVlamp = 0;  // status UV lamp, 1 = aan, 0 = uit

byte maxPWMval[3] = {255, 150, 100}; // max PWM waarde ledlamp. instelbaar in pag LED nextion
byte tijdverloop = 30;               // 30 min van 0 naar max en omgekeerd.
unsigned long PWM_timer[3];          // variabele tbv interval PWM stappen
unsigned long interval[3];           // aantal msec tussen PWM stappen
unsigned long fadetime = 0;

//========================
// tijden in seconden
word timers[10][3] = {
  //st,On,Off
  {0, 0, 0}, // actief 1/0, ZonOp, ZonOnder, led1,2,3
  {0, 0, 0}, // actief 1/0, ZonOp,ZonOnder, UV
  {0, 0, 0}, // actief 1/0, ZonOp,ZonOnder, Warmte
  {0, 0, 0}, // actief 1/0, tsw_3 fogger1
  {0, 0, 0}, // actief 1/0, tsw_4 fogger2
  {0, 0, 0}, // actief 1/0, tsw_5 fogger3
  {0, 0, 0}, // tsw_6
  {0, 0, 0}, // tsw_7
  {0, 0, 0}, // tsw_8
  {0, 0, 0}  // tsw_9
};

byte PWMval[10] = {0, 0, 0, 1, 1, 1, 1, 1, 1, 1}; // PWM waarden
byte OnOff[10]  = {1, 1, 1, 1, 1, 1, 1, 1, 1, 1}; // 0= uit, 1 = aan
byte LedOnOff[3]  = {1, 1, 1};                    // led1 tm 3
byte FanPWM[3]  = {110, 120, 130};                // variabele PWM waarden voor fan
byte FanOnOff[3] = {1, 1, 1};                     // fan aan/uit

word spray[3] = {450, 750, 1020};                 // tijden waarop spray ingeschakeld word in minuten
byte sprayduration[3] = {10, 20, 30};             // tijdsduur sprayen in sec
byte sprayActief[2] = {0, 0};                     // status aan/uit ,secondenteller
byte sprayOnOff[3] = {1, 1, 1};                   // spray aan/uit
boolean spraytest = false;                        // spray test aan/uit

word timesyncCounter = 0;
boolean timesync = false;

//=======================

/*===== Print info bij opstaten ====*/
void fileversion() {
  String filenm = String(__FILE__);
  String datum = String(__DATE__) + " " + String(__TIME__);
  int x = filenm.lastIndexOf('\\');
  Serial.print("\nTerrarium controller versie: ");
  Serial.print(filenm.substring(x + 1));
  Serial.print(" ");
  Serial.println(datum);
}

void setup()  {
  pinMode(13, OUTPUT);
  pinMode(LEDpinA, OUTPUT);
  pinMode(LEDpinB, OUTPUT);
  pinMode(LEDpinC, OUTPUT);
  pinMode(pinFogger, OUTPUT);
  pinMode(pinFanA, OUTPUT);
  pinMode(pinFanB, OUTPUT);
  pinMode(pinFanC, OUTPUT);
  pinMode(heat_lamp, OUTPUT);
  pinMode(sprayPomp, OUTPUT);
  pinMode(pinRelaisA, OUTPUT);
  pinMode(pinRelaisB, OUTPUT);
  pinMode(pinRelaisC, OUTPUT);
  pinMode(pinRelaisD, OUTPUT);
  pinMode(UV_lamp, OUTPUT);
  pinMode(pinBeeper, OUTPUT);
  pinMode(UVOUT, INPUT);
  pinMode(REF_3V3, INPUT);

  Serial.begin(115200);
  Serial1.begin(115200);
  fileversion();

  dht.setup(A2); // data pin 2

  if (readCheck() == false) { // variabelen nog niet opgeslagen in EEPROM
    SetDefaultValues();
  }
  else {
    // lees waarden uit EEPROM
    readTimers();
    readSprayTimers();
    readPWM();
    readCoord();
    readTemp();
    readUVIcal();
    readMinMaxuvIndex();
  }



  // Initialize the rtc object
  rtc.begin();
  setRTCtime();
  String zt = (zomer == 1) ? "Zomertijd" : "Wintertijd";
  Serial.println(zt);
  ZonOpZonOnder();
  calctijdcode();
  setInterval();
  PWM_timer[0] = millis(); // start PWM intervaltimer
  PWM_timer[1] = millis();
  PWM_timer[2] = millis();

  Serial1.print("page Home"); // start nextion homepage
  endString();

  wdt_enable(WDTO_2S); // watchdogtimer 2sec
}

void setInterval() {
  /* bij reset of veranderen led sliders in de periode tussen Dawn+tijdverloop en Dusk,
    is de interval 1ms. De ledverlichting gaat dan direct naar maxPWM. Anders dimmen de leds
    gelijdelijk van 0 naar maxPWM of omgekeerd gedurende de tijdverloop.*/

  word tv = tijdcode + tijdverloop;
  if ((tv > timers[0][1]) && (tijdcode < timers[0][2])) {
    interval[0] = 50;
    interval[1] = 50;
    interval[2] = 50;
    Serial.println("interval_A");
  }
  else
  {
    interval[0] = tijdverloop * 60000 / maxPWMval[0]; // omrekenen min naar msec tijd tussen PWM stappen
    interval[1] = tijdverloop * 60000 / maxPWMval[1];
    interval[2] = tijdverloop * 60000 / maxPWMval[2];
    Serial.print(tijdcode);
    Serial.print(" == ");
    Serial.println(timers[0][2]);

  }
}

void DebugPWM() {
  Serial.print("PWM a=");
  Serial.print(PWMval[0]);
  Serial.print(",");
  Serial.print(maxPWMval[0]);
  Serial.print(",");
  Serial.print(interval[0]);
  Serial.print(",");
  Serial.print(fadetime);
  Serial.println();
}

void DebugOutput() {
  byte x;
  Serial.print(TijdStr());
  Serial.print(" ");
  Serial.print(DatumStr());
  Serial.print(" ");
  Serial.print(tijdcode); // tijd in minuten

  Serial.print(", zon: ");
  Serial.print(zonOp);
  Serial.print("-");
  Serial.print(zonOnder);

  Serial.print(", PWM a=");
  Serial.print(PWMval[0]);
  Serial.print(",");
  Serial.print(LedOnOff[0]);

  Serial.print(", b=");
  Serial.print(PWMval[1]);
  Serial.print(",");
  Serial.print(LedOnOff[1]);

  Serial.print(", c=");
  Serial.print(PWMval[2]);
  Serial.print(",");
  Serial.print(LedOnOff[2]);

  Serial.print(", UVlamp=");
  Serial.print(UVlamp);

  Serial.print(" indx=");
  Serial.print(uvIndex);

  Serial.print(", warmte=");
  Serial.print(infrarood);

  Serial.print(", fogger a=");
  x = timers[3][0] & OnOff[3];
  Serial.print(x);

  Serial.print(", b=");
  x = timers[4][0] & OnOff[4];
  Serial.print(x);

  Serial.print(", c=");
  x = timers[5][0] & OnOff[5];
  Serial.print(x);

  Serial.print(", Spray=");
  Serial.print(sprayActief[1]);

  Serial.print(", fan=");
  byte fs = (FanOnOff[0] == 1) ? FanPWM[0] : 0;
  Serial.print(fs);
  Serial.print(",");

  fs = (FanOnOff[1] == 1) ? FanPWM[1] : 0;
  Serial.print(fs);
  Serial.print(",");

  fs = (FanOnOff[2] == 1) ? FanPWM[2] : 0;
  Serial.print(fs);
  Serial.print(" ");

  Serial.println();
}


/* Serial input Begin ==================================*/
String inputString = "";
boolean inputComplete = false;

boolean serialInput() {
  while (Serial.available()) {
    char inChar = (char)Serial.read();
    if (isAscii(inChar)) {
      if (inChar != '\r') {
        inputString += inChar;
      }
    }
    if (inChar == '\r') {
      inputString.replace("\n", "");
      inputString += "~";
      debugcounter = 0;
      Debug = true;
      inputComplete = true;
    }
  }
  // return false;
}

// nextion seriele input
boolean nexserialInput() {
  while (Serial1.available()) {
    char inChar = (char)Serial1.read();
    if (isAscii(inChar)) {
      if (inChar != '\r') {
        inputString += inChar;
      }
    }
    if (inChar == '\r') {
      inputString.replace("\n", "");
      inputString += "~";
      inputComplete = true;
    }
  }
  //return false;
}


void loop() {
  /* begin 1 seconde loop */
  if (second() != lastSecond) { /* check of secode is verstreken*/
    lastSecond = second();
    /* knipperled ter controle dat programma loopt.*/
    testled = !testled;
    digitalWrite(13, testled);
/*
    if (debugcounter < debugtimeout) { // schakel seriele com uit na xsec geen activiteiten
      debugcounter++;
      if (debugcounter == debugtimeout) {
        Debug = false;
      }
    }
*/
    if (fadetime > 0){fadetime--;}

    wdt_reset();    // reset watchdogtimer
    timeSync();     // sync arduino time met rtc module
    autoreset();    // restart arduino dagelijks om 3:00
    readDHT();      // lees DHT waarden
    UVI();          // lees UVindex
    calctijdcode(); // Zet actuele tijd om in minuten per dag
    timeSwitches(); // Controleer op in-, en uitschakelmomenten. actuele tijd vs timers
    fogger();       // aansturen foggers
    fanPWM();       // regelen en aansturen PWM fan
    spraytime();    // regelen en aansturen spraypomp
    relais();       // in-, uitschakelen timer relais (vrij te gebruiken)
    UV_OnOff();     // in-, uitschakelen UV lamp
    Warmte_OnOff(); // in-, uitschakelen warmte lamp
    alarmBeep();    // Alarm beeper
    backlight();    // regel nextion backlight

    if (Debug) {
      DebugOutput(); // seriële info naar monitor
    }
   // DebugPWM();

    /* pageHome van nextion is de enige pagina die elke sec word ge-updated
       de overige pagina's krijgen alleen een update bij het openen van de pagina */
    if (nextionvenster == 0) {
      nxHome();
    }
    if (nextionvenster == 5) {
      nxLEDval();
    }

  }
  /* einde 1 seconde loop */

  // dit deel draait op max snelheid
  nexserialInput();
  if (inputComplete == false) {
    serialInput();
  }
  while (inputComplete) {
    SerialCmd(); // handle serial commands
  }

  intervalcheck();  // tbv stapsgewijs ophogen of verlagen PWM
}
