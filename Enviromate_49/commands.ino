// seriele data vanuit serial1 en serial2
void SerialCmd() {
  // als inputComplete is true, data ontvangen
  String task;
  if (inputString.length() > 1) {
    inputString.replace(" ", "");
    byte commaPosition;
    commaPosition = inputString.indexOf('~');                                     // zoek eerste positie van ~ teken in inputString
    task = inputString.substring(0, commaPosition);                               // task is commando vanaf postitie 0 tot commaPosition
    inputString = inputString.substring(commaPosition + 1, inputString.length()); // inputString is waarde vanaf commaPosition tot einde string.
    Serial.print("- ");
    Serial.println(task);

    // kijk naar het begin van de string task
    if (task.indexOf("debug") != -1 ) {
      if (task.indexOf("off") != -1 ) {
        Debug = false;
      }
      else {
        Debug = true;
      }
    }

    else if (task.indexOf("pt") == 0 ) {
      PWMval[0]= 125;
    }

    else if (task.indexOf("reset") == 0 ) { // herstart arduino
      asm volatile ( "jmp 0");
    }

    else if (task.indexOf("coord") == 0 ) { // opslaan coordinaten tbv Dusk2Dawn
      task.replace("coord=", "");
      saveCoord(task);
    }

    else if (task.indexOf("maxtemp") == 0 ) { // tbv alarm bij te hoge temperatuur
      task.replace("maxtemp=", "");
      storeMaxtemp(task);
    }

    else if (task.indexOf("mintemp") == 0 ) { // tbv alarm bij te lage temperatuur
      task.replace("mintemp=", "");
      storeMintemp(task);
    }

    else if (task.indexOf("minUVI") == 0 ) { // tbv alarm bij te lage temperatuur
      task.replace("minUVI=", "");
      storeMinuvIndex(task);
    }

    else if (task.indexOf("maxUVI") == 0 ) { // tbv alarm bij te lage temperatuur
      task.replace("maxUVI=", "");
      storeMaxuvIndex(task);
    }

    else if (task.indexOf("zomertijd") == 0 ) { // wel of geen automatische zomertijd
      task.replace("zomertijd=", "");
      autozomertijd = (task == "1") ? true : false;
      storeZomertijd();
      Serial.println("AZt: "+task);
      setRTCtime();
      ZonOpZonOnder();
    }

    else if (task.indexOf("page") == 0 ) { // bepaal welk nextion pagina is geopend
      NextionPage(task);
    }

    else if (task.indexOf("calUVI") == 0 ) {
      task.replace("calUVI=", "");
      uvicalibrate(task);
    }

    else if (task.indexOf("timeset") == 0 ) { // instellen arduino klok
      SerialRTCtime(task);
    }

    else if (task.indexOf("fanpwm") == 0 ) { // fanpwm2=30 (voorbeeld)
      task.replace("fanpwm", "");            // verwijder fanpwm uit task
      cmd_fanpwm(task);                      // stuur restant van task (2=20) naar functie cmd_fanpwm
      nx_Fan();
    }

    else if (task.indexOf("ledpwm") == 0 ) { // ledpwm1=130
      task.replace("ledpwm", "");
      cmd_ledpwm(task);
      nx_LED();
    }

    else if (task.indexOf("tsw") == 0 ) { // tsw0=12:00-19:10
      task.replace("tsw", "");
      cmd_tsw(task);
    }

    else if (task.indexOf("fog") == 0 ) { // fog1=12:00-13:00
      task.replace("fog", "");
      cmd_fog(task);
    }

    else if (task.indexOf("spray") == 0 ) { // spray_t1=12:00 spray_d1=20
      task.replace("spray", "");
      cmd_spray(task);
    }

    else if (task.indexOf("test") == 0 ) { // test 1 test 0
      cmd_spraytest(task);
    }

    else if (task.indexOf("onoff") == 0 ) { // onoff_led1=1 onoff_led1=0
      cmd_OnOff(task);
      /*
        onoff_led1 1
        onoff_fog1 1
        onoff_fan1 1
        onoff_spray1 1
        onoff_uv 1
        onoff_warmte 1
      */
    }

    else {
      Serial.println(" fout commando");
    }
  }
  else
  {
    // taken zijn verwerkt
    inputComplete = false;
  }
}

/* Seriele input Eind ==================================*/



/* commando's vanuit seriele poort */
void cmd_spraytest(String inputString) {
  if (spraytest == false) {
    spraytest = true;
    sprayActief[0] = 11;
    sprayActief[1] = 10; // pomp aan gedurende 10 sec
  }
  else {
    spraytest = false;
    sprayActief[0] = 1;
    sprayActief[1] = 1;
  }
}

void cmd_spraytest_(String inputString) {
  if (inputString == "test=1") {
    spraytest = true;
    sprayActief[0] = 11;
    sprayActief[1] = 10; // pomp aan gedurende 10 sec
  }
  else {
    spraytest = false;
    sprayActief[1] = 0;
    sprayActief[0] = 0;
  }
}

/*instellen tijdschakelaars*/
void cmd_tsw(String inputString) {
  // eerst heel eenvoudige controle op invoer
  if (inputString.indexOf('-') == -1) { // "-" teken moet voorkomen
    inputString = "";
    Serial.println("foutieve invoer");
    return;
  }
  
  byte dev = inputString.substring(0, 1).toInt();

/*
  if ((zonUpDown == false) && (dev == 0)) {
    LED_OnA = inputString.substring(2, inputString.length());
  }
*/
  //convertTxt(0, LED_OnA);
  convertTxt(dev, inputString.substring(2, inputString.length()));
  storeTimers(); // opslaan in EEPROM
}

/*instellen tijdschakelaar foggers*/
void cmd_fog(String inputString) {
  if (inputString.indexOf('-') == -1) {
    inputString = "";
    Serial.println(F("foutieve invoer"));
    return;
  }

  byte dev = inputString.substring(0, 1).toInt() + 2;
  convertTxt(dev, inputString.substring(2, inputString.length()));
  storeTimers(); // opslaan in EEPROM
  
  // check op overlap van tijden en kleur velden
  String kleur;
  kleur = (timers[4][1] <= timers[3][2]) ? geel : wit;
  Serial1.print(F("Fogger.t1.bco="));
  Serial1.print(kleur);
  endString();
  Serial1.print(F("Fogger.t2.bco="));
  Serial1.print(kleur);
  endString();
  
  kleur = (timers[5][1] <= timers[4][2]) ? geel : wit;
  Serial1.print(F("Fogger.t3.bco="));
  Serial1.print(kleur);
  endString();
  Serial1.print(F("Fogger.t4.bco="));
  Serial1.print(kleur);
 endString(); 
}

/*instellen spray*/
void cmd_spray(String inputString) {
  byte x;
  byte nr;
  String tmp;

  //voorbeeld: _t1=12:00
  // 3e char van input is spray nr
  nr = inputString.substring(2, 3).toInt() - 1;
  // 4e en verder is tijd
  tmp = inputString.substring(4, inputString.length());

  if (inputString.indexOf("_t") != -1) { // tijd
    spray[nr] = TimeToMin(tmp);
    storeSprayTimers(nr);
  }

  //voorbeeld: _d1=20
  if (inputString.indexOf("_d") != -1) { // duration
    sprayduration[nr] = tmp.toInt();
    storeSprayDuration(nr);
  }
}

/* instellen fan PWM */
void cmd_fanpwm(String inputString) {
  // voorbeeld 1=123
  byte dev = inputString.substring(0, 1).toInt();                     // pos 0 is fan nr
  byte pval = inputString.substring(2, inputString.length()).toInt(); // 2e tot laatste is PWM waarde
  FanPWM[dev - 1] = pval;                                             // array telt vanaf 0
  storeFanPWM();                                                      // opslaan in EEPROM
}

/* instellen LED PWM */
void cmd_ledpwm(String inputString) {
  // 2=123
  byte dev = inputString.substring(0, 1).toInt(); // bepaal led nr
  dev -=1;
  byte pval = inputString.substring(2, inputString.length()).toInt(); // max PWM
  maxPWMval[dev] = pval; // array telt vanaf 0
  storeMaxPWM(); // opslaan in EEPROM
}

/* On/Off buttons */
void cmd_OnOff(String inputString) {
  //onoff_led1=1
  byte x;
  byte nr;
  byte val;

  x = inputString.indexOf('=');                      // pos van "=" in inputString
  nr = inputString.substring(x - 1, x).toInt();      // getal voor = teken
  val = inputString.substring(x + 1, x + 2).toInt(); // string na = teken
  int indx;
  if (inputString.indexOf("led") != -1 ) {
    nr -= 1;
    LedOnOff[nr] = val;
  }

  if (inputString.indexOf("fog") != -1 ) {
    nr += 2;
    OnOff[nr] = val;
  }

  if (inputString.indexOf("fan") != -1 ) {
    nr -= 1;
    FanOnOff[nr] = val;
  }

  if (inputString.indexOf("spray") != -1 ) {
    nr -= 1;
    sprayOnOff[nr] = val;
  }
  if (inputString.indexOf("uv") != -1 ) {
    OnOff[1] = val;
  }

  if (inputString.indexOf("warmte") != -1 ) {
    OnOff[2] = val;
  }
}
