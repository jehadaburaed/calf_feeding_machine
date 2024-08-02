#include <LiquidCrystal_I2C.h>
#include <ButtonDebouncer.h> 
#include <SharpIR.h>
#include <FlowSensor.h>
#include <Wire.h> //Arduino Mega; SDA = 50, SCL = 51
#include <SPI.h>

// LCD VAR
#define LCD_ADDR 0x27
#define LCD_COLS 16
#define LCD_ROWS 4
//SENSORS
#define IR A0 // define signal pin
#define WATER_LEVEL_SENSOR_PIN A1 // Infrared sensor pin
#define WATER_FLOW_PIN A2
#define PT100_PIN A3
//input
#define AUTO_PIN 41
#define ADJUST_PIN 42
#define STOP_PIN 43
#define START_PIN 44
#define SELECT_PIN 45
#define INC_PIN  46
#define DEC_PIN 47
//sensor models
#define MODEL 1080 // used 1080 because model GP2Y0A21YK0F is used
#define WATER_FLOW_MODEL 450
//water levels
#define WATER_LEVEL_LOW 18 // MAKE sure from water_Level amount
#define WATER_LEVEL_HIGH 8
// mixer
#define START_MIXING 10000
#define MIXING_PERIOD 2000
//milk 
#define MILK_PERIOD 4000
#define VALVE_PERIOD 3000
//OUTPUTS
#define R1_GALLON_HEATER_PIN 31
#define R2_MILK_PIN 32
#define R3_MIXER_PIN 33
#define R4_VALVE_PIN 34
#define R5_HEATER_PIN 35
#define R6_GAUTO_PIN 36
#define R7_GADJUST_PIN 37
#define R8_RERROR_PIN 38

LiquidCrystal_I2C lcd(LCD_ADDR, LCD_COLS, LCD_ROWS);
ButtonDebouncer select(SELECT_PIN, 50);
ButtonDebouncer inc(INC_PIN, 50);
ButtonDebouncer dec(DEC_PIN, 50);
ButtonDebouncer start(START_PIN, 500);
ButtonDebouncer stop(STOP_PIN, 500);
ButtonDebouncer adjust(ADJUST_PIN, 50);
ButtonDebouncer autoB(AUTO_PIN, 50);
SharpIR sharpIR(IR, MODEL);//(PIN,MODEL)
FlowSensor flowSensor(WATER_FLOW_PIN, 2, 4.5);// (MODEL,PIN)

float waterAmount = 0.0; // Total water amount in liters
float water_temp=0.0;
float waterTemp = 0.0; // Water temperature placeholder
float distance1; 
int buttonState = LOW;
int lastButtonState = LOW;
int water = 1000;   // water 1000 ml
int milk  = 180;    // milk  180 gm
int temp  = 37;
int selectStartTime;
int teachModeState = 0; // 0: Normal mode, 1: Teach mode for temperature, 2: Teach mode for water amount, 3: Teach mode for milk amount
int milkAmount = 0; // Milk amount placeholder
int pc =0;
int mc =0;
int mixC= 0;
int mixDuration = 20;


//Variables for the PT100 boards
double resistance;
uint8_t reg1, reg2; //reg1 holds MSB, reg2 holds LSB for RTD
uint16_t fullreg; //fullreg holds the combined reg1 and reg2
double temperature;
//Variables and parameters for the R - T conversion
double Z1, Z2, Z3, Z4, Rt;
double RTDa = 3.9083e-3;
double RTDb = -5.775e-7;
double rpoly = 0;
const int chipSelectPin = 53;

unsigned short counter = 1;
bool save = 1;
bool incStatus=1, decStatus=1, selectStatus=1, adjustStatus=1, autoStatus=1; bool startStatus = 1; bool stopStatus = 1;
bool prevAdjustState = LOW; // Initial state for adjust button
bool prevIncState    = HIGH; // Store the previous state of the inc button
bool prevDecState    = HIGH; // Store the previous state of the dec button
bool prevSelectState = HIGH;
bool prevAutoState   = HIGH; 
bool prevStartState  = LOW; 
bool prevStopState   = LOW;
bool autoModeFlag = 1;
bool goodToGo, endProgram, waterTempOk;
bool mixerState, milkState, valveState, heaterState;  
bool iswaterOK= false;
bool startOp= false;
bool regularMixer = false;

unsigned long mixerStartTime = 0;
unsigned long milkStartTime = 0;
unsigned long valveStartTime = 0;

void noWater(bool &isWaterPresent);

void setup() {  
  Serial.begin(9600);
  SPI.begin();
  flowSensor.begin();
  lcd.init();
  lcd.backlight();


  noInterrupts();           // Disable all interrupts
  TCCR1A = 0;               // Set entire TCCR1A register to 0
  TCCR1B = 0;               // Same for TCCR1B
  TCNT1  = 0;               // Initialize counter value to 0
  OCR1A = 15624;            // 1 second interval at 16MHz with prescaler of 1024
  TCCR1B |= (1 << WGM12);   // Turn on CTC mode
  TCCR1B |= (1 << CS12) | (1 << CS10);  // Set CS12 and CS10 bits for 1024 prescaler
  TIMSK1 |= (1 << OCIE1A);  // Enable timer compare interrupt
  interrupts();

  noInterrupts();           // Disable all interrupts
  TCCR3A = 0;               // Set entire TCCR1A register to 0
  TCCR3B = 0;               // Same for TCCR1B
  TCNT3  = 0;               // Initialize counter value to 0
  OCR3A = 62496;            // 4 second interval at 16MHz with prescaler of 1024
  TCCR3B |= (1 << WGM12);   // Turn on CTC mode
  TCCR3B |= (1 << CS12) | (1 << CS10);  // Set CS12 and CS10 bits for 1024 prescaler
  TIMSK3 |= (1 << OCIE3A);  // Enable timer compare interrupt
  interrupts();

  pinMode(SELECT_PIN,INPUT_PULLUP);
  pinMode(INC_PIN,   INPUT_PULLUP);
  pinMode(DEC_PIN,   INPUT_PULLUP);
  pinMode(START_PIN, INPUT_PULLUP);
  pinMode(ADJUST_PIN,INPUT_PULLUP);
  pinMode(AUTO_PIN,  INPUT_PULLUP);
  pinMode(STOP_PIN,  INPUT_PULLUP);


  pinMode(R1_GALLON_HEATER_PIN, OUTPUT);  
  pinMode(R2_MILK_PIN,  OUTPUT);
  pinMode(R3_MIXER_PIN, OUTPUT);
  pinMode(R4_VALVE_PIN, OUTPUT);
  pinMode(R5_HEATER_PIN,OUTPUT);
  pinMode(R6_GAUTO_PIN, OUTPUT);
  pinMode(R7_GADJUST_PIN,OUTPUT);  
  pinMode(R8_RERROR_PIN, OUTPUT);
  pinMode(chipSelectPin, OUTPUT); //because CS is manually switched 
  
  digitalWrite(R1_GALLON_HEATER_PIN, HIGH);
  digitalWrite(R2_MILK_PIN, HIGH);
  digitalWrite(R3_MIXER_PIN, HIGH);
  digitalWrite(R4_VALVE_PIN, HIGH);
  digitalWrite(R5_HEATER_PIN, HIGH);
  digitalWrite(R6_GAUTO_PIN, HIGH);
  digitalWrite(R7_GADJUST_PIN, HIGH);
  digitalWrite(R8_RERROR_PIN, HIGH);

  mixerState = milkState = valveState = heaterState = 0;
  //lcd.print("hi , omar");

}

ISR(TIMER1_COMPA_vect) {
  
  if(startOp){
      pc++;
      mc++;          // Increment the seconds counter
      if (mc >= 20) { //mix counter
        mc = 0;      // Reset counter after 10 seconds
        mixerState = 0;
        digitalWrite(R3_MIXER_PIN, HIGH);
      }
      if(pc >= 10){   //powder counter (milk)
        Serial.println("hi from pc");
        pc = 0;
        valveState = 0;
        digitalWrite(R4_VALVE_PIN, HIGH);
      }
      
    }
}

ISR(TIMER3_COMPA_vect) {
  if(regularMixer){
    mixC++;
      if(mixC >= 5 && mixDuration >= 15){
        mixDuration--;
        Serial.print("mix Duration is: "); Serial.println(mixDuration);
        digitalWrite(R3_MIXER_PIN,LOW);
        mixerState = 1;
      }
      else if(mixDuration <=16){
          mixDuration = 20;
          mixC = 0;
          digitalWrite(R3_MIXER_PIN,HIGH);
          mixerState = 0;
          }
  }
  if(startOp){
   digitalWrite(R2_MILK_PIN, HIGH);
   milkState = 0;
   Serial.println("hi from vc");
   
   }

}

void loop() { 
  save = 0;
  // lcd.clear();
  // lcd.setCursor(0,0);
  // lcd.print("Welcome to auto");
  autoPrinting();
  adjust.update();
  autoB.update();
  select.update();
  stop.update();
  start.update();
  startStatus= start.getState();
  stopStatus = stop.getState();
  // adjust mode 
  // Serial.println("START STATUS!");
  // Serial.println(startStatus);
  // Serial.println("STOP STATUS!");
  // Serial.println(stopStatus);  
  if(prevStartState == HIGH && startStatus == LOW){
    Serial.println("Interrupt occurred!");
    Serial.print("button status: ");
    Serial.println(save);
    //lcd.setCursor(15,0);
    //save =  1;
    save = !save;
    //lcd.print(save);
    starting();
     }
  if(prevStopState == HIGH && stopStatus == LOW){
    stoping();
   }  
  
  if(goodToGo){// if button start is pressed enter 
    distance1 = 0;
  
     readRegister();
     convertToTemperature();
     printDisplay();
    
    // read water level
    digitalWrite(R6_GAUTO_PIN,LOW);
    digitalWrite(R7_GADJUST_PIN,HIGH);

    distance1 = mesureDistance();
  
    Serial.print("IR see : ");
    Serial.println(distance1);
    //read temp value
    //water_temp = analogRead(PT100_PIN); comenit it unti fix transmiter problem
    //update flow sensor reading
    flowSensor.update();    


    //display water level and temp on lcd
    Serial.print("is water ok: ");
    Serial.println(iswaterOK);
    Serial.print("is Temp  ok: ");
    Serial.println(waterTempOk);

    
    if(temperature < temp - 2 && iswaterOK ){         //LOW WATER TEMP 
      //Print on lcd water temp is low, trun on heater 
      waterTempOk = false;
      digitalWrite(  R5_HEATER_PIN, LOW );
    }
    else if(temperature > temp ){//priint on lcd High water temp, turn off heater
                                      
      waterTempOk = false;
      digitalWrite( R5_HEATER_PIN , HIGH );
    }
    else{ //temp is good
      waterTempOk = true; 
    }
    if( distance1 >= WATER_LEVEL_LOW && waterTempOk && iswaterOK){    //
      startOp = true;
      //open water valv if flwo sensor is not reading any thing so water tank is empty->>>> open Milk, mixer on
      digitalWrite( R2_MILK_PIN, LOW); 
      digitalWrite( R3_MIXER_PIN, LOW);
      digitalWrite( R4_VALVE_PIN , LOW );
      valveState = 1;
      milkState  = 1;
      mixerState = 1;
      //flowSensor.update();
      //check flow_water_sensor  
      //unsigned long totalMilliLitres = flowSensor.getTotalMilliLitres();
    }
    else if(distance1 <= WATER_LEVEL_HIGH && waterTempOk && iswaterOK){
      Serial.println("Fuck you");
      regularMixer = true;
    }
    // Adjust Mode Button Handling
    // ADJUST BUTTON HAS BEEN PRESEED (high)
  selectStatus = select.getState();
  adjustStatus = adjust.getState();

  if (prevAdjustState == HIGH && adjustStatus == LOW) {
        digitalWrite(R7_GADJUST_PIN,LOW);
        digitalWrite(R6_GAUTO_PIN,HIGH);
        save = 1;
        //if(  1){//selectStatus ==LOW && prevSelectState == HIGH
        start.update();   
        select.update();
        autoB.update();
        lcd.clear();
        //Serial.print("Select_status is:");
        //Serial.println(select.getState());
        inc.update();
        dec.update();
        while(save != 0){
            autoB.update();
            autoStatus   = autoB.getState();
            if(counter > 3) counter = 1;
            incStatus= inc.getState();
            decStatus= dec.getState();
            selectStatus= select.getState();
            start.update();
            select.update();
            inc.update();
            dec.update();

            if(prevIncState == LOW && incStatus == HIGH  && counter %3 == 1 ){
              temp++;
              }
            else if(prevDecState == LOW && decStatus == HIGH && counter %3 == 1 ){
              temp--;
              }        
            if(prevIncState == LOW && incStatus == HIGH && counter %3 == 2 ){
              milk+=20;
              }
            else if(prevDecState == LOW && decStatus == HIGH && counter %3 == 2 ){
              milk-=20;
              }
            if(prevIncState == LOW && incStatus == HIGH && counter %3 == 0 ){
              water+=100;
              }
            else if(prevDecState == LOW && decStatus == HIGH && counter %3 == 0){
              water-=100;
              }                                        
            prevIncState = incStatus; // Update the previous state
            prevDecState = decStatus; // Update the previous state        
            
            lcd.setCursor(0,0);
            lcd.print("VAR values");
            lcd.setCursor(0, 1);
            lcd.print("T:");
            lcd.print(temp);
            lcd.print("C");
            lcd.setCursor(0, 2);
            lcd.print("M:");
            lcd.print(milk);
            lcd.print("mg");
            lcd.setCursor(0, 3);
            lcd.print("Wat:");
            lcd.print(water);
            lcd.print("ml");
            lcd.setCursor(12, 3);
            lcd.print("c=");
            lcd.print(counter);
            /*Serial.print("Temp: ");Serial.println(temp);
            Serial.print("Water: "); Serial.println(water);
            Serial.print("Milk: ");Serial.println(milk);*/
            if( selectStatus ==LOW && prevSelectState == HIGH){
            counter++;
            selectStartTime= millis();
            }
            Serial.print("AUTO_Status is: ");
            Serial.print(autoStatus);
            Serial.println(prevAutoState);
            if( autoStatus ==LOW && prevAutoState == HIGH ) {
              Serial.println("exit adjust mode");
              save = 0;
              lcd.clear();
              autoModeFlag = 0 ;
            }
            prevSelectState = selectStatus;
            prevAutoState   = autoStatus;
            
            // Wait a bit before returning to normal state
            delay(50);
          }
      }
    prevAdjustState = adjustStatus;  // Update the previous state of adjust button

  }
  prevStartState = startStatus;
  prevStopState = stopStatus;
}

void noWater(bool &isWaterPresent){
  Serial.println("wellcome to No water");
  bool findWater = false;  
  //Open water valv until it see water 
  digitalWrite( R4_VALVE_PIN , LOW );
  valveState = 1;
  Serial.println("Open valve");
  float waterRate= 0.0;
  lcd.clear();
  delay(20);
  lcd.setCursor(0,0);
  lcd.print("No Water Check"); delay(5);
  //enter infinit loop until see water 
  while(!findWater){

    for(int i = 0; i<70; i++){
    Serial.print("Waiting For Water From noWater: ");   
    flowSensor.update();
   // waterAmount = flowSensor.getTotalMilliLitres();
    waterRate = flowSensor.getFlowRate();
    Serial.println(waterRate);
    }
      if(waterRate <= 4.0 ){// if water reed less than 100 ml then no water
        findWater= true;
        lcd.setCursor(7,1);  
        lcd.print(waterRate);
        lcd.print("ml/sec");          
          }
        delay(100);  

    }
  //turn off water valve  
  isWaterPresent = findWater;
  digitalWrite( R4_VALVE_PIN , HIGH );
  valveState = 0;
  lcd.clear();
  }



void starting(){
  float totalMilliLitres = 0.0;
  Serial.println("wellcome to start");
  goodToGo = true;
  lcd.clear(); delay(20);
  lcd.setCursor(0,0);
  lcd.print("Starting Program");
  Serial.println("Wellcome, Let's Get Started");
  Serial.println("Water check");
  lcd.setCursor(4,1);
  lcd.print("Water Check");
  digitalWrite(R4_VALVE_PIN,LOW);// TURN VALV ON
  valveState = 1;
  flowSensor.update();
  delay(1500);// wait for 1.5 sec for water to get out
  //check flow_water_sensor  
  Serial.print("from Water check the Output water quantity: ");
  digitalWrite(R4_VALVE_PIN,HIGH); // TURN VALV OFF
  valveState = 0;  
  lcd.setCursor(0,2);
  lcd.print("S-Water:"); delay(50);
  //lcd.setCursor(2,8);
  flowSensor.update();
  totalMilliLitres = flowSensor.getFlowMilliLitres();
  //totalMilliLitres = flowSensor.getTotalMilliLitres();

  Serial.print(totalMilliLitres);
  Serial.println("mL");
  lcd.print(totalMilliLitres);

  if(totalMilliLitres <= 100 ){        // no water-> output water is less than 100ml
    noWater(iswaterOK);
    }

 }
void stoping(){
  goodToGo = false;
  Serial.println("wellcome to stop");
  lcd.clear();  delay(20);
  lcd.setCursor(0,0);
  lcd.print("Go To Sleep"); delay(700);
  digitalWrite(R1_GALLON_HEATER_PIN, HIGH);
  digitalWrite(R2_MILK_PIN,  HIGH);
  digitalWrite(R3_MIXER_PIN, HIGH);  
  digitalWrite(R4_VALVE_PIN, HIGH);
  digitalWrite(R5_HEATER_PIN,HIGH);
  digitalWrite(R6_GAUTO_PIN,HIGH);
  valveState = 0;
  milkState  = 0;
  mixerState = 0;
  heaterState= 0;
  delay(500);
  endProgram = true;
}

 void autoPrinting(){
  //lcd.clear();
  lcd.setCursor(4, 0);
  lcd.print("Auto Mode");
  lcd.setCursor(0, 1);
  lcd.print("Temp:");
  lcd.print(temp);
  lcd.print("c   H V M PM");
  lcd.setCursor(0, 2);
  lcd.print("Milk:");
  lcd.print(milk);
  lcd.print("mg ");
  lcd.print(heaterState);
  lcd.print(" ");
  lcd.print(valveState);
  lcd.print(" ");
  lcd.print(mixerState);
  lcd.print(" ");
  lcd.print(milkState);
  lcd.print(" ");
  lcd.setCursor(0, 3);
  lcd.print("Water:");
  lcd.print(water);
  lcd.print("ml");  


  }


float mesureDistance(){

  int analogPin = A1;
  float sensorVal = 0;
  float sensorVolt = 0;
  float Vr=5.0;
  float sum=0;
  float k1=16.7647563;
  float k2=-0.85803107;
  float distance=0;

  sum=0;
  for (int i=0; i<150; i++) {
    sum=sum+float(analogRead(analogPin));  
  }
  sensorVal=sum/150;
  sensorVolt=sensorVal*Vr/1024;

  distance = pow(sensorVolt*(1/k1), 1/k2);
  delay(250);
  return distance;
}


void printDisplay()
{
    // Start at top-left corner
  Serial.print("PT100 TEMP IS: ");
  Serial.println(temperature);
  
}

void convertToTemperature()
{
  Rt = resistance;
  Rt /= 32768;
  Rt *= 430; //This is now the real resistance in Ohms

  Z1 = -RTDa;
  Z2 = RTDa * RTDa - (4 * RTDb);
  Z3 = (4 * RTDb) / 100;
  Z4 = 2 * RTDb;

  temperature = Z2 + (Z3 * Rt);
  temperature = (sqrt(temperature) + Z1) / Z4;

  if (temperature >= 0)
  {
    Serial.print("Temperature: ");
    Serial.println(temperature); //Temperature in Celsius degrees
    return; //exit
  }
  else
  {
    Rt /= 100;
    Rt *= 100; // normalize to 100 ohm

    rpoly = Rt;

    temperature = -242.02;
    temperature += 2.2228 * rpoly;
    rpoly *= Rt; // square
    temperature += 2.5859e-3 * rpoly;
    rpoly *= Rt; // ^3
    temperature -= 4.8260e-6 * rpoly;
    rpoly *= Rt; // ^4
    temperature -= 2.8183e-8 * rpoly;
    rpoly *= Rt; // ^5
    temperature += 1.5243e-10 * rpoly;

    Serial.print("Temperature: ");
    Serial.println(temperature); //Temperature in Celsius degrees

  }
  //Note: all formulas can be found in the AN-709 application note from Analog Devices
}


void readRegister()
{
  SPI.beginTransaction(SPISettings(500000, MSBFIRST, SPI_MODE1));
  digitalWrite(chipSelectPin, LOW);

  SPI.transfer(0x80); //80h = 128 - config register
  SPI.transfer(0xB0); //B0h = 176 - 10110000: bias ON, 1-shot, start 1-shot, 3-wire, rest are 0
  digitalWrite(chipSelectPin, HIGH);

  digitalWrite(chipSelectPin, LOW);
  SPI.transfer(1);
  reg1 = SPI.transfer(0xFF);
  reg2 = SPI.transfer(0xFF);
  digitalWrite(chipSelectPin, HIGH);

  fullreg = reg1; //read MSB
  fullreg <<= 8;  //Shift to the MSB part
  fullreg |= reg2; //read LSB and combine it with MSB
  fullreg >>= 1; //Shift D0 out.
  resistance = fullreg; //pass the value to the resistance variable
  //note: this is not yet the resistance of the RTD!

  digitalWrite(chipSelectPin, LOW);

  SPI.transfer(0x80); //80h = 128
  SPI.transfer(144); //144 = 10010000
  SPI.endTransaction();
  digitalWrite(chipSelectPin, HIGH);

  //Serial.print("Resistance: ");
  //Serial.println(resistance);
}