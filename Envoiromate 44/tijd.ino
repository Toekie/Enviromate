/* Bereken tijd zon op en zon onder ==========*/
void ZonOpZonOnder() {
  float latitudef;
  float longitudef;
  latitudef = latitude.toFloat();
  longitudef = longitude.toFloat();
  
  Dusk2Dawn  Wageningen(latitudef, longitudef, 1);
  int SunRise  = Wageningen.sunrise(year(), month(), day(), autozomertijd);
  int SunSet   = Wageningen.sunset(year(), month(), day(), autozomertijd);

  char time1[] = "00:00";
  Dusk2Dawn::min2str(time1, SunRise);
  zonOp = String(time1);

  char time2[] = "00:00";
  Dusk2Dawn::min2str(time2, SunSet);
  zonOnder = String(time2);

// timers[0][1] en [0][2] zijn de enige die dusk en dawn toegewezen krijgen
// de overige timers worden handmatig ingesteld.
// led 2 en 3 zijn indien automatisch, gekoppeld aan timers[0][1] en [0][2].
// indien led 2 en 3 op manueel, worden deze leds gekoppeld aan timers[1][1] en [1][2]
  if (zonUpDown == true){
    timers[0][1] = TimeToMin(zonOp); // in minuten
    timers[0][2] = TimeToMin(zonOnder); // in minuten
  }
  else 
  {
    // indien zonUpDown == false
    convertTxt(0, LED_OnA);
  }
}


void autoreset(){
  // om 3:00 autorestart 
  if (hour() != 3) {return;}
  if (minute() != 0) {return;}
  if (second() == 0){
    asm volatile ( "jmp 0");
  }
}

/* ===========================================*/


/* Detects if we are in summer time*/
boolean inSummerTime(int uur, int dag, int maand, int jaar, int dw) {
  if (!autozomertijd){ return false;}
  if (dag < 0 || maand < 0 || jaar < 0) return false;
  if ((maand >= 3) && (maand <= 10)) { // March to October inclusive   
    if ((maand > 3) && (maand < 10)) { // Definitely
      return true;
    }

  //int dw = weekday();    
    if (maand == 3) { // Have we passed the last Sunday of March, 1am UT ?
      if (dag >= 25) { // It's getting interesting
        // When is the next sunday ?
        int dts = 7 - dw; // Number of days before the next sunday
        if (dts == 6) dts = 7; // We are really speaking of the next sunday, not the current one
        if ((dag + dts) > 31) { // The next sunday is next month !
          if (dw != 7 || uur > 0) // We finally check that we are not on the day of the change before the time of the change
            return true;
        }
      }
    }
    if (maand == 10) { // Have we passed the last Sunday of October 1am UT ?
      if (dag >= 25) { // It's getting interesting
        // When is the next sunday ?
        int dts = 7 - dw; // Number of days before the next sunday
        if (dts == 6) dts = 7; // We are really speaking of the next sunday, not the current one
        if ((dag + dts) > 31) { // The next sunday is next month !
          if (dw != 7 || uur > 0) // We finally check that we are not on the day of the change before the time of the change
            return false; // We have passed the change
          else return true;
        } else return true;
      } else return true;
    }
  }
  return false;
}


// de RTC module staat altijd ingesteld op de wintertijd
// de interne arduino klok wordt gelijkgezet met de RTC module en aangepast aan zomer,- en wintertijd
// De interne klok word gebruikt als tijd en periodiek gelijk gezet met de RTC module

/* set RTC module */
void SerialRTCtime(String input){
  input.replace("timeset=","");
  input.replace("-",":");
  input +="~";
  byte commaPosition;  /* the position of the next comma in the string */
  word data[6];
  for (byte i = 0; i <= 5; i++) {
    commaPosition = input.indexOf(':');
    data[i] = input.substring(0, commaPosition).toInt();
    input = input.substring(commaPosition + 1, input.length());
    Serial.println(data[i]);
  }
  // controleer op zomertijd
  if ((data[0] == 0) && (zomer == 1)) {data[0] = 23;}
  else { data[0] = data[0] - 1*zomer;}
  
  rtc.setTime(data[0], data[1], data[2]);  
  rtc.setTime(19, 56, 50) ; //24h format
  //rtc.setDate(15, 12, 2020);   // Set the date to January 1st, 2014
  rtc.setDate(data[3],data[4],data[5]);
  setRTCtime();
}


/*set arduino clock*/
void setRTCtime(){
  t = rtc.getTime();
  setTime(t.hour, t.min, t.sec, t.date, t.mon, t.year);
  
  if (inSummerTime(t.hour, t.date, t.mon, t.year, t.dow)){
    adjustTime(3600);
    zomer = 1;
   Serial.println("zomertijd");
  }
  else {
    zomer = 0;
   Serial.println("wintertijd");
   }
}

// sync arduino time met rtc module
void timeSync(){
  int s = second();
  timesyncCounter++;
  if ((timesyncCounter == 300) && (timesync == false)){
    if ((second() > 55) || (second()<5)){
      timesyncCounter = 290;
    }
    else {  
      setRTCtime();
      timesync = true;
      s-=second();
    //  Serial.println(s);
    }
  }
  if (timesyncCounter == 420){
    timesync = false;
    timesyncCounter = 120;
  }
}


/* converteren tijdwaarden begin ===================*/

/* tijdstring naar minuten*/
word TimeToMin(String val) {
  word t;
  String tmp = val.substring(0, 2);
  t = tmp.toInt() * 60;
  tmp = val.substring(3, 5);
  t += tmp.toInt();
  return t;
}

/* minuten naar tijdstring*/
String MinutesToStr(word val) {
  int h = val / 60;
  int m = val % 60;
  String hm;
  hm = (h < 10) ? "0" : "";
  String tmp = hm+String(h) + ":";
  hm = (m < 10) ? "0" : "";
  tmp += hm + String(m);
  return tmp;
}

// actuele tijd in minuten
void calctijdcode() {
  tijdcode = hour() * 60;
  tijdcode += minute();
}
/* converteren tijdwaarden einde ===================*/


void maakTijdStr(){
  TijdStr = "";
  byte tt = hour();
  if (tt < 10) {TijdStr += "0";} 
  TijdStr+= tt;
  TijdStr+= ":";
  tt = minute();
  if (tt < 10) {TijdStr += "0";}
  TijdStr+=tt;
  TijdStr+=":";
  tt = second();
  if (tt < 10) {TijdStr += "0";}
  TijdStr+=tt;  
}

void maakDatumStr(){
  DatumStr = "";
  byte dd = "";
  dd = day();
  if (dd < 10) {DatumStr += "0";} 
  DatumStr+= dd;
  DatumStr+= "-";  
  dd = month();
  if (dd < 10) {DatumStr += "0";} 
  DatumStr+= dd;
  DatumStr+= "-";  
  DatumStr+= year();
}

/* convert(timer[nr][veld])*/
word convertTxt(byte nr, String input){
  input.replace("-",":");
  input.replace(" ",":");
  word valA;
  word valB;

  int x = input.indexOf(':');
  valA = input.substring(0,x).toInt();
  input = input.substring(x+1,input.length()); 
  valA = valA*60;
  x = input.indexOf(':');
  valA += input.substring(0,x).toInt();
  input = input.substring(x+1,input.length()); 

  x = input.indexOf(':');
  valB = input.substring(0,x).toInt();
  input = input.substring(x+1,input.length()); 
  valB = valB*60;
  x = input.indexOf(':');
  valB += input.substring(0,x).toInt();
  input = input.substring(x+1,input.length()); 
  timers[nr][1] = valA;
  timers[nr][2] = valB;
}


void timeSwitches() {
  byte aan = 1;
  byte uit = 2;

  for (byte i = 0; i <= 9; i++) { 
    timers[i][0] = 0;            /* uitganspositie = 0 */

    /* indien uitschakeltijd kleiner (vroeger) is dan inschakelijd, is de uitschakeltijd na middernacht.*/
    /* indien utschakeltijd na 24:00 */
    if (timers[i][uit] < timers[i][aan]) {
      if (tijdcode < timers[i][uit]) {
        timers[i][0] = 1;
      }
      if (tijdcode >= timers[i][aan]) {
        timers[i][0] = 1;
      }
    }

    /* Indien actuele tijd >= inschakeltijd && actuele tijd < uitschakeltijd dan status timers[x][0] = 1 */
    /* indien uitschakeltijd voor 24:00 */
    else if ( (tijdcode >= timers[i][aan]) && (tijdcode < timers[i][uit]) ) {
      timers[i][0] = 1;
    }
    if (fadetime == 0){
      if (tijdcode == timers[0][aan]) {fadetime = tijdverloop*60;}
      if (tijdcode == timers[0][uit]) {fadetime = tijdverloop*60;}
    }
  }

  if (sprayActief[0] == 0){ // pomp staat uit
    /* byte sprayActief[2] = {0,0}; // status aan/uit ,secondenteller */
    sprayActief[1] = 0; // spraytijd = 0
    if (tijdcode == spray[0]){sprayActief[1] = sprayduration[0]+1;} // sprayduur per spraytijd
    if (tijdcode == spray[1]){sprayActief[1] = sprayduration[1]+1;}
    if (tijdcode == spray[2]){sprayActief[1] = sprayduration[2]+1;}

    if (sprayActief[1] > 0){
      /* timeswitch schakeld op minuten. Bij spraytest van 20 sec is de minuut nog niet voorbij en zal de spray wederom
         actief worden. Om dit te voorkomen word sprayActief op 62 sec gezet zodat er zeker 1 minuut na start spraytest voorbij is. */
      sprayActief[0] = 62; // de pomp kan niet meer aangezet worden gedurende 1 minuut
    }
  }
}
