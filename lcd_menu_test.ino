#include <LiquidCrystal.h>
#include <EEPROM.h>

//--LCD dispay
LiquidCrystal lcd(8, 9, 4, 5, 6, 7);

//--Input & Button Logic
const int numOfInputs = 6;
//const int inputPins[numOfInputs] = {8,9,10,11,11};
int inputState[numOfInputs];
int lastInputState[numOfInputs] = { LOW,LOW,LOW,LOW,LOW };
bool inputFlags[numOfInputs] = { LOW,LOW,LOW,LOW,LOW };
long lastDebounceTime[numOfInputs] = { 0,0,0,0,0 };
long debounceDelay = 5;

//--LCD Menu Logic
const int numOfScreens = 6;
int currentScreen = 0;
String screens[numOfScreens][2] = 
  {{"End Voltage:", "Volts"  }, 
   {"Dis. Current:", "Amps"  }, 
   {"Timer:", "Mins"         },
   {"Overload Temp:", "degC" },
   {"Start Discharge:", ""   },
   {"Save Paramaters:", ""   }};

//--Device parameters
float parameters[numOfScreens];
float parameterCheck[numOfScreens][3] = 
  {{ 3.7, 4.2, 0.01  },  // end discharge 
   { 1.0, 10.0, 0.5  },  // discharge amps 
   { 0.0, 120.0, 1.0 },  // Timer
   { 0.0, 65.0, 1.0  },  // Temperature 
   { 0, 1, 1         },  // Start stop 
   { 0, 1, 1         }}; // Save parameters
   
//--EEPROM
int eeAddress[numOfScreens];// = { 1, 32, 63, 95 };
bool eepromFlag = 1;
//int eeFlagAddress = 0;
bool debugFlag = 1;


void setup() {
  //for(int i = 0; i < numOfInputs; i++) {
  //  pinMode(inputPins[i], INPUT);
  //  digitalWrite(inputPins[i], HIGH); // pull-up 20k
  //}
  Serial.begin(9600);
  
  while (!Serial) {
    ; // wait for serial port to connect
  }
  
  for (int i = 0; i < numOfScreens; i++) {
    int n = 32;
    if (i == 0) { eeAddress[i] = 1; }
    else { eeAddress[i] = i * n; }
  } 

  //EEPROM.put(eeAddress, eeprom);
  if (!eepromFlag) {
    Serial.println("Writing EEPROM Parameters:");
    parameters[0] = 3.80;
    parameters[1] = 5.0;
    parameters[2] = 10.0;
    parameters[3] = 50.0;
    parameters[4] = +0.0; // 0.0 is different to +0.0 and -0.0

    for (int i = 0; i < numOfScreens; i++) {
      EEPROM.put(eeAddress[i], parameters[i]);
      Serial.println(i);
    }
  }

  for (int i =0; i < numOfScreens; i++) {
    float f;
    Serial.print("EEPROM Address ");
    Serial.print(i);
    Serial.print(": ");
    Serial.println(EEPROM.get(eeAddress[i], f));
  }

  //--get parameters from EEPROM
  for (int i =0; i < numOfScreens; i++) {
    float f;
    parameters[i] = EEPROM.get(eeAddress[i], f);
  }

  lcd.begin(16, 2);
  printScreen();
}

void loop() {
  setInputFlags();
  resolveInputFlags();
}

int keystate() {     //read key value
  int val = analogRead(A0);
  if (val > 1000) { return 100;    } //no key
  else if (val > 800)  { return 4; } //select
  else if (val > 600)  { return 0; } //left
  else if (val > 400)  { return 3; } //down
  else if (val > 200)  { return 2; } //up
  else { return 1; }                 //right
}

void setInputFlags() {
  int key = keystate();
  //if (debugFlag) { Serial.println(key); }

  for(int i = 0; i < numOfInputs; i++) {
    int reading;
    if (key != 100 && key == i) { reading = HIGH; } 
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
  if (debugFlag) { Serial.println(input); }
  if(input == 0) {
    if (currentScreen == 0) { currentScreen = 0; } 
    else { currentScreen--; }
  } 
  else if(input == 1) {
    if (currentScreen == numOfScreens-1) { currentScreen = numOfScreens-1; } 
    else { currentScreen++; }
  } 
  else if(input == 2) { parameterChange(0); } 
  else if(input == 3) { parameterChange(1); }
  else if(input == 4) {
    if (currentScreen == numOfScreens-1) {
      saveToEeprom();
    }
    else if (currentScreen == numOfScreens-2) {
      discharge();
    }
  }
}


void parameterChange(int key) {
  if(key == 0) {
    if (parameters[currentScreen] < parameterCheck[currentScreen][1]) { 
      parameters[currentScreen] = parameters[currentScreen] + parameterCheck[currentScreen][2]; 
    }
  }
  else if(key == 1) {
    if (parameters[currentScreen] > parameterCheck[currentScreen][0]) { 
      parameters[currentScreen] = parameters[currentScreen] - parameterCheck[currentScreen][2];
    }
  }
}

void saveToEeprom() { 
  if (parameters[currentScreen] == 1.0)  {
    for (int i = 0; i < numOfScreens-2; i++) {
      EEPROM.put(eeAddress[i], parameters[i]);
    }
  }
}


void discharge() {
  if (parameters[currentScreen] == 1.0)  {
    Serial.println("Discharging");
  }
}

void printScreen() {
  //Serial.println(parameters[currentScreen]);
  lcd.clear();
  lcd.print(screens[currentScreen][0]);
  if (currentScreen < numOfScreens-2) {
    lcd.setCursor(0,1);
    lcd.print(parameters[currentScreen]);
    lcd.print(" ");
    lcd.print(screens[currentScreen][1]);
  }
  else if (parameters[currentScreen] == 0.0) {
    lcd.setCursor(0,1);
    lcd.print("No ");
  }
  else if (parameters[currentScreen] == 1.0) {
    lcd.setCursor(0,1);
    lcd.print("Yes ");
  }
}
