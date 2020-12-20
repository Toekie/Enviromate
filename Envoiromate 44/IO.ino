/* omdat de 3 led verlichting verschillende PWM waarden kunnen hebben is de tijdspanne tussen elke stap
verhoging of verlaging verschillend. Ele led verlichting heeft daarom zijn eigen interval.
Hierdoor zullen de PWM waarden in alle gevallen even veel tijd nodig hebben om van 0 naar max te gaan.
*/

void setPWMinterval(byte dev){
  word tv = tijdcode + tijdverloop;
  /*interval tussen zon-op + tijdsverloop en zon-onder */
  if ( (tv > timers[0][1]) && (tijdcode < timers[0][2]) ) {
    interval[dev] = 50;
  }

  /*interval indien PWM gewijzigd na zon-onder */
  else if ( (tijdcode > timers[0][2]) && (maxPWMval[dev] < PWMval[dev]) ) {
    interval[dev] = 50;
  }

  /*interval gedurende resterende fade time */
  else
  {
    if (fadetime >0){
      interval[dev] = fadetime * 1000 / (PWMval[dev]+1);
    }
  }

//  if (interval[dev] == 0) {interval[dev] = 1000;} 
}

void intervalcheck() { 
  if (millis() > PWM_timer[0]) {
    setPWMinterval(0);
    PWM_timer[0] = millis() + interval[0]; // instellen volgende interval timer   
    regelPWMA(0);
  }
  if (millis() > PWM_timer[1]) {
    setPWMinterval(1);
    PWM_timer[1] = millis() + interval[1];
    regelPWMA(1);
  }
  if (millis() > PWM_timer[2]) {
    setPWMinterval(2);
    PWM_timer[2] = millis() + interval[2];
    regelPWMA(2);
  }
}

/* iedere intervalstap zal de PWM waarde verhogen of verlagen. */
void regelPWMA(byte n) {
  // verhoog of verlaag PWM waarde van 0 naar maxPWMval in 30 minuten.
  if (timers[0][0] == 1) {
    if (PWMval[n] < maxPWMval[n]) {
      PWMval[n]++; // ophogen actuele PWM waarde
    }
    else if (PWMval[n] > maxPWMval[n]) {
      PWMval[n] --;
    }
  }
  else {
    if (PWMval[n] > 0)  {
      PWMval[n] --; // verlagen actuele PWM waarde
    }
  }

 /* PWM waarde indien knoppen pageLight = "On" */
  byte p_val[3];
  p_val[0] = (LedOnOff[0] == 1) ? PWMval[0] : 0;
  p_val[1] = (LedOnOff[1] == 1) ? PWMval[1] : 0;
  p_val[2] = (LedOnOff[2] == 1) ? PWMval[2] : 0; 
  // aansturen IO pinnen 
  analogWrite(LEDpinA, p_val[0]);
  analogWrite(LEDpinB, p_val[1]);
  analogWrite(LEDpinC, p_val[2]);
}


/* aansturen IO pin foggers*/
void fogger() {
  byte foggerIO = 1;
  
  /* fogger = on indien timer == 1 en pagebutton == 1 */
  if ((timers[3][0] == 1) && (OnOff[3] == 1)) {
    foggerIO = 0;
  }
  if ((timers[4][0] == 1) && (OnOff[4] == 1)) {
    foggerIO = 0;
  }
  if ((timers[5][0] == 1) && (OnOff[5] == 1)) {
    foggerIO = 0;
  }

  digitalWrite(pinFogger, foggerIO);
}


void fanPWM() {
  byte fpwm;
  fpwm = (FanOnOff[0] == 1) ? FanPWM[0] : 0;
  analogWrite(pinFanA, 255-fpwm);

  fpwm = (FanOnOff[1] == 1) ? FanPWM[1] : 0;
  analogWrite(pinFanB, 255-fpwm);

  fpwm = (FanOnOff[2] == 1) ? FanPWM[2] : 0;
  analogWrite(pinFanB, 255-fpwm);
}


void UV_OnOff(){
  byte aan = LOW;
  byte uit = HIGH;

  //UV lamp gekoppeld aan zon-op/onder
  if (OnOff[1] == 1){
    if (PWMval[0] >= maxPWMval[0]) {
      UVlamp = 1;
      digitalWrite(UV_lamp, aan);
    }
    else {
      UVlamp = 0;
      digitalWrite(UV_lamp, uit);
    } 
  }
  // UV lamp op eigen timer 
  else { /* OnOff[1] == 0 */
    if (timers[1][0] == 1){
      UVlamp = 1;
      digitalWrite(UV_lamp, aan);
    }
    else {
      UVlamp = 0;
      digitalWrite(UV_lamp, uit);
    }    
  }
}


void Warmte_OnOff(){
  byte aan = LOW;
  byte uit = HIGH;

  //warmte lamp gekoppeld aan zon-op/onder
  if (OnOff[2] == 1){
    if (PWMval[0] >= maxPWMval[0]) {
      infrarood = 1;
      digitalWrite(heat_lamp, aan);
    }
    else {
      infrarood = 0;
      digitalWrite(heat_lamp, uit);
    } 
  }
  // Warmte lamp op eigen timer 
  else {
    if (timers[2][0] == 1){
      infrarood = 1;
      digitalWrite(heat_lamp, aan);
    }
    else {
      infrarood = 0;
      digitalWrite(heat_lamp, uit);
    }    
  }
}


void relais() {
  byte aanuit;
  // extra relais aan timer
  aanuit = (timers[6][0] == 1) ? LOW : HIGH; // OnOff
  digitalWrite(pinRelaisA, aanuit);
  aanuit = (timers[7][0] == 1) ? LOW : HIGH; // OnOff
  digitalWrite(pinRelaisB, aanuit);
  aanuit = (timers[8][0] == 1) ? LOW : HIGH; // OnOff
  digitalWrite(pinRelaisC, aanuit);
  aanuit = (timers[9][0] == 1) ? LOW : HIGH; // OnOff
  digitalWrite(pinRelaisD, aanuit);
}


void spraytime() {
 if ((sprayActief[1] > 0) || (spraytest)) {
    digitalWrite(sprayPomp, LOW);
    sprayActief[1]--;
    if (sprayActief[1] == 0) {
      spraytest = false;
      Serial1.print("Mister.b0.txt=\"Test\"");
      endString();
      }
    else {
      digitalWrite(sprayPomp, HIGH);
      Serial1.print("Mister.b0.txt=\""+String(sprayActief[1])+"\"");
      endString();
    }
  }

// blokkeerteller, voorkomt dat binnen 1 minuut de spray weer actief word
  if (sprayActief[0] > 0) {
    sprayActief[0]--;
  }

}

void alarmBeep(){
  if (DHTalarm + UVIalarm > 0){
    beep = !beep;
    }
  else {
    beep = 0;
  }
  digitalWrite(pinBeeper, beep);
}


//-------------------------------------Humidity and Temperature------------------------------------------------
void readDHT(){
  DHTcount++;
  if (DHTcount < 10){return;} // 1x per 10 sec.

  DHTcount = 0;
  humidity = dht.getHumidity();     //Read  humidity , Reading humidity takes about 250 milliseconds!
  temperature = dht.getTemperature(); //Read temperature, Reading temperature takes about 250 milliseconds!

  if (temperature < mintemp){
    DHTalarm = 1;
  }
  else if (temperature > maxtemp){
    DHTalarm = 2;
  }
  else {DHTalarm = 0;}

  if (Debug){
 // Serial.print(dht.getStatusString());
    Serial.print("humidity: ");
    Serial.print(humidity, 1);
    Serial.print(", temperature: ");
    Serial.print(temperature, 1);
    Serial.print(", max temp: ");
    Serial.print(maxtemp);
    Serial.print(", mintemp: ");
    Serial.print(mintemp);
    Serial.print(", DHTalarm: ");
    Serial.print(DHTalarm);
    Serial.print(", UVIalarm: ");
    Serial.println(UVIalarm);
  }
}
//-----------------------------------------------Day temperature control---------------------------------------


//UVI begin --------------------------------------------------------------------------------------

float UVI()
{
  int uvLevel = averageAnalogRead(UVOUT);
  int refLevel = averageAnalogRead(REF_3V3);

  //Use the 3.3V power pin as a reference to get a very accurate output value from sensor
  float outputVoltage = 3.3 / refLevel * uvLevel;

  float uvIntensity = mapfloat(outputVoltage, UVIcal, 2.8, 0.0, 15.0); //Convert the voltage to a UV intensity level
  uvIndex = ((outputVoltage - UVIcal) / 0.08) - 1; // uvIntensity converted to UV Index value
  
  //Serial.print(refLevel);
  //Serial.print(uvLevel);
  //Serial.print(outputVoltage);
  //Serial.print(uvIntensity);
  //Serial.print(uvIndex);

  char uvIndexlcdresult[10]; // Buffer big enough for 9-character float
  dtostrf(uvIndex, 8, 3, uvIndexlcdresult); // Leave room for too large numbers!

//  Serial.print(uvIndexlcdresult);
//  Serial.print(uvIndexlcdresult);

  UVIalarm = 0;
  if (uvIndex < minUVindx){ UVIalarm = 1;}
  if (uvIndex > maxUVindx){ UVIalarm = 2;}
   
 return uvIndex;
}


//Takes an average of readings on a given pin
//Returns the average
int averageAnalogRead(int pinToRead)
{
  byte numberOfReadings = 8;
  unsigned int runningValue = 0;

  for (int x = 0 ; x < numberOfReadings ; x++)
    runningValue += analogRead(pinToRead);
  runningValue /= numberOfReadings;

  return (runningValue);
}

float mapfloat(float x, float in_min, float in_max, float out_min, float out_max)
{
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}
//UVI einde --------------------------------------------------------------------------------------


void backlight(){
  word LDRValue = analogRead(A7);
  word valLDR;
  if (LDRValue < LDRdrempel) { valLDR = map(LDRValue, 0, LDRdrempel, 30, 70);}
  else { valLDR = map(LDRValue, LDRdrempel, 1024, 70, 100); }

  if (valLDR > displayLight) {displayLight+=1;}
  if (valLDR < displayLight) {displayLight-=1;}
  if (displayLight > 100) {displayLight = 100;}
  if (lastdisplayLight != displayLight){ // stuur dimwaarde indien veranderd
    lastdisplayLight = displayLight;
    Serial1.print("dim=");
    Serial1.print(displayLight);
    endString();
    Serial.print("dim=");
    Serial.print(displayLight);
    Serial.print(", ");

  }
}
