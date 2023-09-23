#include <LiquidCrystal.h>
LiquidCrystal lcd(8, 9, 4, 5, 6, 7);

//Input & Button Logic
const int numOfInputs = 5;
//const int inputPins[numOfInputs] = {8,9,10,11,11};
int inputState[numOfInputs];
int lastInputState[numOfInputs] = {LOW,LOW,LOW,LOW,LOW};
bool inputFlags[numOfInputs] = {LOW,LOW,LOW,LOW,LOW};
long lastDebounceTime[numOfInputs] = {0,0,0,0,0};
long debounceDelay = 5;

//LCD Menu Logic
const int numOfScreens = 4;
int currentScreen = 0;
String screens[numOfScreens][2] = {{"End Voltage:","Volts"}, {"Dis. Current:", "Amps"}, 
{"Timer:","Mins"},{"Overload Temp:","degC"}};
float parameters[numOfScreens];
float parameterCheck[numOfScreens][3] = {{3.7, 4.2, 0.01}, {1.0, 10.0, 0.5}, {0.0, 120.0, 1.0}, {0.0, 65.0, 1.0}};

void setup() {
  //for(int i = 0; i < numOfInputs; i++) {
  //  pinMode(inputPins[i], INPUT);
  //  digitalWrite(inputPins[i], HIGH); // pull-up 20k
  //}
  Serial.begin(9600);
  lcd.begin(16, 2);
  parameters[0] = 3.80;
  parameters[1] = 5.0;
  parameters[2] = 10.0;
  parameters[3] = 50.0;
  printScreen();
}

void loop() {
  setInputFlags();
  resolveInputFlags();
}

int keystate() {     //read key value
  int a=analogRead(0);
  if (a > 1000) { return 100; } //no key
  if (a > 800)  { return 5; } //select
  if (a > 600)  { return 0; } //left
  if (a > 400)  { return 3; } //down
  if (a > 200)  { return 2; } //up
  return 1;                   //right
}

void setInputFlags() {
  int key = keystate();
  Serial.println(key);

  for(int i = 0; i < numOfInputs; i++) {
    //int key = keystate();
    int reading;
    if (key != 100 && key == i) { reading = HIGH; } //Serial.println("here");}
    else { reading = LOW; }
    if (reading != lastInputState[i]) {
      lastDebounceTime[i] = millis();
    }
    if ((millis() - lastDebounceTime[i]) > debounceDelay) {
      if (reading != inputState[i]) {
        inputState[i] = reading;
        if (inputState[i] == HIGH) {
          inputFlags[i] = HIGH;
        }
      }
    }
    lastInputState[i] = reading;
  }
}

void resolveInputFlags() {
  for(int i = 0; i < numOfInputs; i++) {
    if(inputFlags[i] == HIGH) {
      inputAction(i);
      inputFlags[i] = LOW;
      printScreen();
    }
  }
}

void inputAction(int input) {
  if(input == 0) {
    if (currentScreen == 0) {
      currentScreen = numOfScreens-1;
    }else{
      currentScreen--;
    }
  }else if(input == 1) {
    if (currentScreen == numOfScreens-1) {
      currentScreen = 0;
    }else{
      currentScreen++;
    }
  }else if(input == 2) {
    parameterChange(0);
  }else if(input == 3) {
    parameterChange(1);
  }
}

void parameterChange(int key) {
  if(key == 0) {
      if (parameters[currentScreen] < parameterCheck[currentScreen][1]) { 
        parameters[currentScreen] = parameters[currentScreen] + parameterCheck[currentScreen][2]; 
      }
  }else if(key == 1) {
      if (parameters[currentScreen] > parameterCheck[currentScreen][0]) { 
        parameters[currentScreen] = parameters[currentScreen]- parameterCheck[currentScreen][2];
      }
  }
}

void printScreen() {
  lcd.clear();
  lcd.print(screens[currentScreen][0]);
  lcd.setCursor(0,1);
  lcd.print(parameters[currentScreen]);
  lcd.print(" ");
  lcd.print(screens[currentScreen][1]);
}
