/* opslaan en inlezen settings in EEPROM */

void uvicalibrate(String input){
  UVIcal = input.toFloat();
  UVIcal /= 1000;
  EEPROM.write(mem_UVcal,   input[0]);
  EEPROM.write(mem_UVcal+1, input[1]);
  EEPROM.write(mem_UVcal+2, input[2]); 
  Serial.println(UVIcal,3);
}

void readUVIcal(){
   String val = "";
   char data;
   data = EEPROM.read(mem_UVcal);
   val += data;
   data = EEPROM.read(mem_UVcal+1);
   val += data;
   data = EEPROM.read(mem_UVcal+2);
   val += data;
   UVIcal = val.toFloat();
   UVIcal /= 1000;
}

void storeMinuvIndex(String input){
  word val = input.toInt();
  byte hb = val / 100; // highbit
  byte lb = val % 100; // lowbit
  
  EEPROM.write(mem_UVcal+3, hb);
  EEPROM.write(mem_UVcal+4, lb);
  minUVindx = input.toFloat();
  minUVindx /= 100;  
}

void storeMaxuvIndex(String input){
  word val = input.toInt();
  byte hb = val / 100; // highbit
  byte lb = val % 100; // lowbit
  
  EEPROM.write(mem_UVcal+5, hb);
  EEPROM.write(mem_UVcal+6, lb);
  maxUVindx = input.toFloat();
  maxUVindx /= 100;  
}

void readMinMaxuvIndex(){
  float val;
  minUVindx = EEPROM.read(mem_UVcal+3);
  val=        EEPROM.read(mem_UVcal+4);
  val /= 100;
  minUVindx+=val;

  maxUVindx = EEPROM.read(mem_UVcal+5);
  val=        EEPROM.read(mem_UVcal+6);
  val /= 100;
  maxUVindx+=val;

  Serial.print("minUVindx: ");
  Serial.print(minUVindx,2);
  Serial.print(", maxUVindx: ");
  Serial.println(maxUVindx,2);
}


void storeMaxtemp(String input){
  word val = input.toInt();
  byte hb = val / 10; // highbit
  byte lb = val % 10; // lowbit
  
  EEPROM.write(mem_dht, hb);
  EEPROM.write(mem_dht + 1, lb);
  maxtemp = input.toFloat();
  maxtemp /= 10;  
}

void storeMintemp(String input){
  word val = input.toInt();
  byte hb = val / 10; // highbit
  byte lb = val % 10; // lowbit
  
  EEPROM.write(mem_dht + 2, hb);
  EEPROM.write(mem_dht + 3, lb);
  mintemp = input.toFloat();
  mintemp /= 10;  
}

void readTemp(){
  float val;
  maxtemp = EEPROM.read(mem_dht);
  val =     EEPROM.read(mem_dht+1);
  val /= 10;
  maxtemp+=val;
  
  mintemp =  EEPROM.read(mem_dht+2);
  val=       EEPROM.read(mem_dht+3);
  val /= 10;
  mintemp+=val;
  Serial.print("mintemp: ");
  Serial.print(mintemp,1);    
  Serial.print(", maxtemp: ");
  Serial.println(maxtemp,1);
}

/* Spray time settings begin ====================*/
void storeSprayTimers(byte nr){
  byte adr = mem_spray;
  byte H;
  byte L;

  for (int i = 0; i <= 2; i++) {
    if (i == nr){
      H = spray[i] / 100;
      L = spray[i] % 100;
      EEPROM.write(adr, H);
      EEPROM.write(adr+1, L);
    }
    adr+=3;
  }
}

void storeSprayDuration(byte nr){
  byte adr = mem_spray;

  for (int i = 0; i <= 2; i++) {
    if (i == nr){
     // Serial.print("saveSprayDuration ");
     // Serial.println(i);
      EEPROM.write(adr+2, sprayduration[i]);
    }
    adr+=3;
  }
}

void readSprayTimers(){
  byte adr = mem_spray;
  byte val;
  word tval;
  Serial.println("spraytimers");
  for (int i = 0; i <= 2; i++) {
    // tijdstip aan
    val = EEPROM.read(adr);
    tval = val * 100; // H
    adr++;
    val = EEPROM.read(adr);
    tval += val; // L
    spray[i] = tval;
    adr++;
    sprayduration[i] = EEPROM.read(adr);
    adr++;
    Serial.print(i+1);
    Serial.print(": ");
    Serial.print(MinutesToStr(spray[i]));
    Serial.print(" - ");
    Serial.print(sprayduration[i]);
    Serial.print(" - ");
    Serial.println(sprayOnOff[i]);

  }
}
/* Spray time and duration settings end ====================*/


/* timer settings begin ====================================*/
void storeTimers(){
  byte adr = mem_timers;
  byte h;
  byte m;

  for (int i = 0; i <= 9; i++) {
    // tijstip aan
    h = timers[i][1] / 100; // uur
    m = timers[i][1] % 100; // min
    EEPROM.write(adr + 0, h);
    EEPROM.write(adr + 1, m);

    // tijdstip uit
    h = timers[i][2] / 100; // uur
    m = timers[i][2] % 100; // min
    EEPROM.write(adr + 2, h);
    EEPROM.write(adr + 3, m);
    adr += 4;
  }
}


boolean readTimers() {
  byte adr = mem_timers;
  byte val;
  word tval;
  Serial.println("timers");
  for (int i = 0; i <= 9; i++) {
    // tijdstip aan
    val = EEPROM.read(adr + 0);
    tval = val * 100; // uur
    val = EEPROM.read(adr + 1);
    tval += val; // min
    timers[i][1] = tval;

    // tijdstip uit
    val = EEPROM.read(adr + 2);
    tval = val * 100; // uur
    val = EEPROM.read(adr + 3);
    tval += val; // min
    timers[i][2] = tval;
    adr += 4;
    Serial.print(MinutesToStr(timers[i][1]));
    Serial.print(" - ");
    Serial.println(MinutesToStr(timers[i][2]));
  }
  return true;
}
/* timer settings end ====================================*/


/* PWM settings begin ====================================*/
// PWMval[10]
// FanPWM[3]

void storePWM(){
  byte adr = mem_PWM;
  byte i;
  for ( i = 0; i <= 9; i++) {
    EEPROM.write(adr + i, PWMval[i]);
  }
}

void storeFanPWM(){
  EEPROM.write(mem_PWM +10, FanPWM[0]);
  EEPROM.write(mem_PWM +11, FanPWM[1]);
  EEPROM.write(mem_PWM +12, FanPWM[2]);
}

void storeMaxPWM(){
  EEPROM.write(mem_PWM +15, maxPWMval[0]);
  EEPROM.write(mem_PWM +16, maxPWMval[1]);
  EEPROM.write(mem_PWM +17, maxPWMval[2]); 
}

void readPWM(){
  byte adr = mem_PWM;
  byte i;
  for ( i = 0; i <= 9; i++) {
    PWMval[i] = EEPROM.read(adr + i);
  }
  FanPWM[0] = EEPROM.read(mem_PWM + 10);
  FanPWM[1] = EEPROM.read(mem_PWM + 11);
  FanPWM[2] = EEPROM.read(mem_PWM + 12);

  maxPWMval[0] = EEPROM.read(mem_PWM + 15);
  maxPWMval[1] = EEPROM.read(mem_PWM + 16);
  maxPWMval[2] = EEPROM.read(mem_PWM + 17);

}
/* FanPWM settings end ===================================*/


/* latitude/longitude settings begin======================*/
void saveCoord(String coord) {
  byte adr = mem_coord;
  coord +="#";
  //coord = latitude+";"+longitude+"#";
  byte L = coord.length() + 1;
/*
  Serial.print("coordinates: ");
  Serial.print(coord);
  Serial.print(" L: ");
  Serial.println(L);
*/  
  for (word i = 0; i <= L; i++)
  {
    EEPROM.write(mem_coord +i, coord[i]);
    adr++;
  }
}


void readCoord(){
  byte adr = 0;
  latitude = "";
  longitude = "";
  
  for (byte i = 0; i <= 12; i++)
  {
    char data = EEPROM.read(mem_coord +i);
    if (data != ';'){
      latitude += data;
    }
    else {break;}
   adr++;
  }
  
  adr += mem_coord;
  
  for (byte i = 1; i <= 12; i++)
  {
    char data = EEPROM.read(adr +i);
    if (data != '#'){
      longitude += data;
    }
    else {break;}
  }

  Serial.println("latitude: "+latitude);
  Serial.println("longitude: "+longitude);
}
/* latitude/longitude settings end  ======================*/


void SetDefaultValues(){
  /* omzetten tijd tekststrings naar minuten
  en plaatsen in array timers[] */
  convertTxt(0, LED_OnA);
  convertTxt(1, UV);
  convertTxt(2, Warmte);
  convertTxt(3, tsw_3);
  convertTxt(4, tsw_4);
  convertTxt(5, tsw_5);
  convertTxt(6, tsw_6);
  convertTxt(7, tsw_7);
  convertTxt(8, tsw_8);
  convertTxt(9, tsw_9);   
  // bewaar in EEPROM
  for (int i = 0; i <= 2; i++) {
    storeSprayTimers(i);
    storeSprayDuration(i);
  }
  storeTimers();
  storePWM();
  storeMaxtemp("240");
  storeMintemp("180");
  saveCoord(latitude + ";" + longitude);
  uvicalibrate("920");
  storeMinuvIndex("2800");
  storeMaxuvIndex("3000");
  
  storeCheck();
  asm volatile ( "jmp 0"); // reboot arduino
}

boolean readCheck(){
  if (EEPROM.read(0) != check) {
    Serial.println("Store default val in eeprom");
    return false;
  }
  else { return true;}
}

void storeCheck(){
  EEPROM.write(0, check);
}
