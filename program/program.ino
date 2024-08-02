#include <SharpIR.h>
#include <FlowSensor.h>
#include <LiquidCrystal_I2C.h>
#include <ButtonDebouncer.h>

// Pins assignment

//SENSORS
#define WATER_LEVEL_SENSOR_PIN 10 // Infrared sensor pin
#define WATER_FLOW_PIN 2
#define PT100_PIN 15
#define IR A0 // define signal pin


//INPUTS
#define AUTO_PIN 41
#define ADJUST_PIN 42
#define STOP_PIN  3      //43
#define START_PIN 2      //44
#define SELECT_PIN 45
#define PLUS_PIN 46
#define MINUS_PIN 47

//OUTPUTS
#define R1_GALLON_HEATER_PIN 31
#define R2_MILK_PIN 32
#define R3_MIXER_PIN 33
#define R4_VALVE_PIN 34
#define R5_HEATER_PIN 35
#define R6_GAUTO_PIN 36
#define R7_GADJUST_PIN 37
#define R8_RERROR_PIN 38
// Constants
#define WATER_LEVEL_LOW 30 // MAKE sure from water_Level amount
#define WATER_LEVEL_HIGH 10
#define WATER_FLOW_RATE 1 // Liters per minute
#define WATER_TEMP_THRESHOLD_LOW 35
#define WATER_TEMP_THRESHOLD_HIGH 37
#define MIX_INTERVAL 600000 // 10 minutes in milliseconds
#define MIX_DURATION 20000 // 20 seconds in milliseconds
#define TEACH_MODE_DURATION 7000 // Teach mode exit duration in milliseconds
#define TEACH_MODE_TEMP_DURATION 3000 // Teach mode duration for temperature adjustment in milliseconds
#define TEACH_MODE_WATER_DURATION 3000 // Teach mode duration for water amount adjustment in milliseconds
#define TEACH_MODE_MILK_DURATION 3000 // Teach mode duration for milk amount adjustment in milliseconds
#define MODEL 1080 // used 1080 because model GP2Y0A21YK0F is used
#define WATER_FLOW_MODEL 450

const byte statusLed = 26;      // flow sensor led 
const byte sensorInterrupt = 0; // Interrupt 0 is associated with pin 2
const byte startInterrupt = 2;   // start buttoun interrupt
const byte stopInterrupt = 3;   // stop buttoun interrupt

float waterAmount = 0.0; // Total water amount in liters
float water_temp=0.0;
float waterTemp = 0.0; // Water temperature placeholder

int teachModeState = 0; // 0: Normal mode, 1: Teach mode for temperature, 2: Teach mode for water amount, 3: Teach mode for milk amount
int milkAmount = 0; // Milk amount placeholder
int distance; 
int temp,water,milk;
bool goodToGo, endProgram, waterTempOk;
bool firstTimeWaterCheck = true;
bool iswaterOK= false;
void noWater(bool &isWaterPresent);

SharpIR SharpIR(IR, MODEL);//(PIN,MODEL)
FlowSensor flowSensor(WATER_FLOW_PIN, statusLed, sensorInterrupt, 4.5);// (MODEL,PIN)
LiquidCrystal_I2C lcd(0x27, 16, 4); // I2C address 0x27, 16 column and 2 rows

void setup() {
  // put your setup code here, to run once:
  pinMode(WATER_LEVEL_SENSOR_PIN, INPUT);
  pinMode(AUTO_PIN  , INPUT);
  pinMode(ADJUST_PIN, INPUT);
  pinMode(STOP_PIN  , INPUT);
  pinMode(START_PIN , INPUT);
  pinMode(SELECT_PIN, INPUT);
  pinMode(PLUS_PIN  , INPUT);
  pinMode(MINUS_PIN , INPUT);

  pinMode(R1_GALLON_HEATER_PIN, OUTPUT);  
  pinMode(R2_MILK_PIN, OUTPUT);
  pinMode(R3_MIXER_PIN, OUTPUT);
  pinMode(R4_VALVE_PIN, OUTPUT);
  pinMode(R5_HEATER_PIN, OUTPUT);
  pinMode(R6_GAUTO_PIN, OUTPUT);
  pinMode(R7_GADJUST_PIN, OUTPUT);  
  pinMode(R8_RERROR_PIN, OUTPUT);  

  pinMode(ADJUST_PIN, INPUT_PULLUP);
  pinMode(PLUS_PIN, INPUT_PULLUP);
  pinMode(MINUS_PIN, INPUT_PULLUP);
  Serial.begin(9600);
  flowSensor.begin();
  attachInterrupt(digitalPinToInterrupt(startInterrupt), start, LOW);
  attachInterrupt(digitalPinToInterrupt(stopInterrupt), stop, LOW);
  lcd.init(); // initialize the lcd
  lcd.backlight();
  lcd.clear();// maybe we will not need this line

  digitalWrite(R1_GALLON_HEATER_PIN, HIGH);
  digitalWrite(R2_MILK_PIN, HIGH);
  digitalWrite(R3_MIXER_PIN, HIGH);
  digitalWrite(R4_VALVE_PIN, HIGH);
  digitalWrite(R5_HEATER_PIN, HIGH);
  digitalWrite(R6_GAUTO_PIN, HIGH);
  digitalWrite(R7_GADJUST_PIN, HIGH);
  digitalWrite(R8_RERROR_PIN, HIGH);

}

void loop() {
  Serial.println("Wellcome to loop");
  Serial.print("adjust button status:");
  bool adjust_button = debounceButton(ADJUST_PIN);
  Serial.println(adjust_button);
  if(adjust_button)
    handleTeachMode();
  bool waterDetected = false;
  lcd.clear();
  delay(20);
  lcd.setCursor(0,0);
  lcd.print("Ready: ");
  delay(5);
  lcd.print(goodToGo);
  delay(5);
  lcd.setCursor(1, 0);
  lcd.print("End Program: ");
  delay(5);
  lcd.print(endProgram);
  delay(5);
  /*if(goodToGo){// if button start is pressed enter 
  // read water level
  distance   = SharpIR.distance();
  //read temp value
  water_temp = analogRead(PT100_PIN);
  //display water level and temp on lcd
  if(water_temp < WATER_TEMP_THRESHOLD_LOW){ //LOW WATER TEMP 
    //Print on lcd water temp is low, trun on heater 
    waterTempOk = false;
    digitalWrite(R5_HEATER_PIN,HIGH);
  }
  else if(water_temp > WATER_TEMP_THRESHOLD_HIGH){
    //priint on lcd High water temp, turn off heater
    waterTempOk = false;
    digitalWrite(R5_HEATER_PIN,LOW); 
  }
  else{
  waterTempOk = true;  //temp is good
  }
  if(firstTimeWaterCheck || distance >= WATER_LEVEL_LOW){ 
    //no water and it is the first time to run the program
    //open water valv if flwo sensor not reading any thing so water tank is empty
    digitalWrite(R4_VALVE_PIN,HIGH);
    flowSensor.update();
    delay(3000);
    //check flow_water_sensor  
    unsigned long totalMilliLitres = flowSensor.getTotalMilliLitres();
    Serial.print("from Water check the Output water quantity: ");
    Serial.print(totalMilliLitres);
    Serial.println("mL");
    digitalWrite(R4_VALVE_PIN,LOW);
    if(totalMilliLitres <= 100 )        // no water-> output water is less than 100ml
    noWater(iswaterOK);
    firstTimeWaterCheck = false;   
  }
  else{//( dis < 10 ) 
    // Water level is high, check water temperature and act accordingly
    waterTemp = getWaterTemperature();
    if (waterTemp >= WATER_TEMP_THRESHOLD_HIGH -2 && waterTemp < WATER_TEMP_THRESHOLD_HIGH) {
      // Water temperature is within acceptable range, turn on valve
      digitalWrite(R4_VALVE_PIN, HIGH);
      flowSensor.update ();
      waterAmount = flowSensor.getTotalMilliLitres(); // Increment water amount
      if (waterAmount >= 1000.0) {
        // If 1 liter water reached, turn off valve
        digitalWrite(R4_VALVE_PIN, LOW);
      }
      Serial.print("heated Water amount is : ");
      Serial.print(waterAmount);
      Serial.println(" ml");
    } else if (waterTemp >= WATER_TEMP_THRESHOLD_HIGH) {
      // Water temperature is high, turn off valve and heater
      digitalWrite(R4_VALVE_PIN, LOW);
      //digitalWrite(R5_HEATER_PIN, LOW);
      Serial.println("Temperature is high");
    } else if (waterTemp < WATER_TEMP_THRESHOLD_HIGH - 2) {
      // Water temperature is low, turn off valve and turn on heater
      digitalWrite(R4_VALVE_PIN, LOW);
      //digitalWrite(R5_HEATER_PIN, HIGH);
      Serial.println("Temperature is low");
    }
  }

  // Powder Tank
  digitalWrite(R2_MILK_PIN, HIGH); // Turn on motor to push powder

  // Mixer
  digitalWrite(R3_MIXER_PIN, HIGH); // Heater is always on
  mixMixture(); // Function to mix the mixture every 10 minutes

  // Teach Mode Button Handling
  // ADJUST BUTTON HAS BEEN PRESEED (high)
  if(digitalRead(ADJUST_PIN))
    handleTeachMode();
  }*/
 }
 
void start(){
  Serial.println("wellcome to start");
  goodToGo = true;
  lcd.clear(); delay(20);
  lcd.setCursor(0,0);
  lcd.print("Starting Program"); delay(5);
  Serial.println("Wellcome, Let's Get Started");
  Serial.println("Water check");
  lcd.setCursor(1,4);
  lcd.print("Water Check");  delay(5);
  digitalWrite(R4_VALVE_PIN,HIGH);
  flowSensor.update();
  delay(1500);// wait for 1.5 sec for water to get out
  //check flow_water_sensor  
  unsigned long totalMilliLitres = flowSensor.getTotalMilliLitres();
  Serial.print("from Water check the Output water quantity: ");
  Serial.print(totalMilliLitres);
  Serial.println("mL");
  digitalWrite(R4_VALVE_PIN,LOW);  
  lcd.setCursor(2,0);
  lcd.print("S-Water amount: "); delay(5);
  //lcd.setCursor(2,8);
  lcd.print(totalMilliLitres); delay(5);

  if(totalMilliLitres <= 100 )        // no water-> output water is less than 100ml
    noWater(iswaterOK);

 }
void stop(){
  Serial.println("wellcome to stop");
  lcd.clear();  delay(20);
  lcd.setCursor(0,0);
  lcd.print("Go To Sleep"); delay(5);
  digitalWrite(R2_MILK_PIN,  LOW);
  digitalWrite(R3_MIXER_PIN, LOW);  
  digitalWrite(R4_VALVE_PIN, LOW);
  digitalWrite(R5_HEATER_PIN,LOW);
  //delay(500);
  endProgram = true;
 }

void noWater(bool &isWaterPresent){
  Serial.println("wellcome to No water");

  float waterAmount= 0.0;
  lcd.clear(); delay(20);
  lcd.setCursor(0,0);
  lcd.print("noWater Check"); delay(5);
  lcd.setCursor(3, 0);
  lcd.print("L:"); delay(5);
  bool findWater = false;  
  isWaterPresent = findWater;
  //Open water valv until it see water 
  digitalWrite(R4_VALVE_PIN,HIGH);
  //enter infinit loop until see water 
  
  while(!findWater){
  Serial.println("Waiting for water");
  flowSensor.update();
  waterAmount = flowSensor.getTotalMilliLitres();
  if(waterAmount >= 100.0 ){// if water reed less than 100 ml then no water
    findWater= true;
      }
    lcd.setCursor(3,2);  
    lcd.print(waterAmount);
    delay(10);
    }
  //turn off water valve  
  digitalWrite(R4_VALVE_PIN,LOW);
  }

void mixMixture() {
  static unsigned long lastMixTime = 0;
  static bool mixState = false;

  unsigned long currentTime = millis();

  if (currentTime - lastMixTime >= MIX_INTERVAL) {
    if (!mixState) {
      // First mix: Mix water and milk powder
      delay(MIX_DURATION); // Mix for 20 seconds
    } else {
      // Second mix: Mix the mixture every 10 minutes
      delay(MIX_DURATION); // Mix for 20 seconds
    }
    mixState = !mixState;
    lastMixTime = currentTime;
  }
 }

float getWaterTemperature() {
  // Function to read temperature sensor and return water temperature
  // This function should be implemented according to your hardware setup
  // For demonstration purposes, we'll return a random value between 30 and 45
  return random(30, 45);
 }

void handleTeachMode() {
  Serial.println("wellcome to handlee teach mode");

  bool select_status = false;
  bool adjust_status = digitalRead(ADJUST_PIN);
  byte counter = 0;
  byte state;
  static unsigned long teachModeStartTime = 0;
  static bool teachButtonState = false;
  lcd.clear();
  lcd.setCursor(0,2);
  lcd.print("Adjust Mode");
  lcd.setCursor(1,2);
  lcd.print("TEMP: ");
  lcd.print(temp);
  lcd.setCursor(2,2);
  lcd.print("WATER: ");
  lcd.print(water);
  lcd.setCursor(3,2);
  lcd.print("Milk: ");
  lcd.print(milk);
  delay(500);
  int teachButtonStateNew = digitalRead(ADJUST_PIN);

  // Check if teach mode button is pressed
  if (teachButtonStateNew == LOW && teachButtonState == HIGH) {
    teachModeStartTime = millis();
  }
  while(!select_status){
  
  //check if select has been pressed for 5 sec then exit ADJUST MODE  
  int selectButtonStartTime = millis();
  bool selectButtonPressed  = debounceButton(SELECT_PIN);
  bool plusButtonPressed  = debounceButton(PLUS_PIN);
  bool minusButtonPressed  = debounceButton(MINUS_PIN);
  Serial.println("from adjust mode print the stateus of buttons");
  Serial.print("select status: ");
  Serial.println(selectButtonPressed);
  Serial.print("plus status: ");
  Serial.println(plusButtonPressed);
  Serial.print("minus status: ");
  Serial.println(minusButtonPressed);


  if( !selectButtonPressed && millis() - selectButtonStartTime >= 5000)
    select_status= true;
  // Check if teach mode button is held for certain durations
  teachButtonState = teachButtonStateNew;
  if(!selectButtonPressed){
    if(counter > 2)
    counter = 0;
    else
    counter++;
  }

  // Adjust parameters based on teach mode state
  
  if (counter == 0) {
    if (!plusButtonPressed)
      temp++;
    else if(!minusButtonPressed)
      temp--;  
    lcd.setCursor(1,7);
    lcd.print(temp);  
  } else if (counter == 1) {
    if (plusButtonPressed)
      water += 50;
    else if(!minusButtonPressed)
      water -= 50; 
    lcd.setCursor(2,7);
    lcd.print(water);
  } else if (counter == 2) {
    if (!plusButtonPressed)
      milk += 20;
    else if(!minusButtonPressed)
      milk -= 20; 
    lcd.setCursor(3,7);
    lcd.print(milk); 
    delay(100); 
    // Teach mode for milk amount
    // Implement code to adjust milk amount
      }
    }   
 }

const int debounceDelay = 100; // Adjust debounce delay as needed

bool debounceButton(int buttonPin) {
  static unsigned long lastDebounceTime = 0;
  static bool lastButtonState = LOW;
  bool buttonState;
  
  // Read the state of the button
  bool reading = digitalRead(buttonPin);

  // If the button state has changed, reset the debounce timer
  if (reading != lastButtonState) {
    lastDebounceTime = millis();
  }
  
  // Check if the button state has remained stable for the debounce delay
  if ((millis() - lastDebounceTime) > debounceDelay) {
    // Update the button state if it's been stable for long enough
    buttonState = reading;
  } else {
    // Otherwise, keep the previous button state
    buttonState = lastButtonState;
  }

  // Save the current button state for the next iteration
  lastButtonState = reading;

  // Return the debounced button state
  return buttonState;
}
