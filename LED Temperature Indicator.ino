// Matrix pins
int DIN = 3;
int CS = 4;
int CLK = 2;

int GREEN = 5; // Green LED pin
int RED = 6; // Red LED pin
int SWITCH_TEMP_BUTTON = 10; // Switch button pin

// Thermistor pin
int THERMISTOR_PIN = A0;

float LED_LIGHT_CHANGE_SENSITIVITY = 150;

float R1 = 10000; // Thermistor resistor resistance
float c1 = 0.001129148, c2 = 0.000234125, c3 = 0.0000000876741; // Steinhart-Hart coeficients for thermistor

bool isFarenheit = false;
float prevTempearature;

// The number of each column on the matrix (the first 8 bits)
bool cols[][8] = {
  {0, 0, 0, 0, 0, 0, 0, 1}, // 1
  {0, 0, 0, 0, 0, 0, 1, 0}, // 2
  {0, 0, 0, 0, 0, 0, 1, 1}, // 3
  {0, 0, 0, 0, 0, 1, 0, 0}, // 4
  {0, 0, 0, 0, 0, 1, 0, 1}, // 5
  {0, 0, 0, 0, 0, 1, 1, 0}, // 6
  {0, 0, 0, 0, 0, 1, 1, 1}, // 7
  {0, 0, 0, 0, 1, 0, 0, 0}, // 8
};

// The LEDs to turn on the matrix
bool scheme[][8] = {
  {0, 0, 0, 1, 1, 1, 1, 1}, // Left-most column
  {0, 0, 0, 1, 0, 0, 0, 1},
  {0, 0, 0, 1, 1, 1, 1, 1},
  {0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 1, 1, 1, 1, 1},
  {0, 0, 0, 1, 0, 0, 0, 1},
  {0, 0, 0, 1, 1, 1, 1, 1} // Right-most column
};

// Patterns of digits to show on the matrix
bool digits[][3][5] = {
  {{1, 1, 1, 1, 1}, {1, 0, 0, 0, 1}, {1, 1, 1, 1, 1}}, // 0
  {{0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {1, 1, 1, 1, 1}}, // 1
  {{1, 1, 1, 0, 1}, {1, 0, 1, 0, 1}, {1, 0, 1, 1, 1}}, // 2
  {{1, 0, 1, 0, 1}, {1, 0, 1, 0, 1}, {1, 1, 1, 1, 1}}, // 3
  {{0, 0, 1, 1, 1}, {0, 0, 1, 0, 0}, {1, 1, 1, 1, 1}}, // 4
  {{1, 0, 1, 1, 1}, {1, 0, 1, 0, 1}, {1, 1, 1, 0, 1}}, // 5
  {{1, 1, 1, 1, 1}, {1, 0, 1, 0, 1}, {1, 1, 1, 0, 1}}, // 6
  {{0, 0, 0, 0, 1}, {0, 0, 0, 0, 1}, {1, 1, 1, 1, 1}}, // 7
  {{1, 1, 1, 1, 1}, {1, 0, 1, 0, 1}, {1, 1, 1, 1, 1}}, // 8
  {{1, 0, 1, 1, 1}, {1, 0, 1, 0, 1}, {1, 1, 1, 1, 1}}, // 9
};

bool farenheitPattern[][8] = {
  {0, 0, 0, 0, 1, 1, 1, 0}, // Left-most column
  {0, 0, 0, 0, 1, 0, 1, 0},
  {0, 0, 0, 0, 1, 1, 1, 0},
  {0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 1, 1, 1, 1, 1, 0},
  {0, 0, 0, 0, 1, 0, 1, 0},
  {0, 0, 0, 0, 1, 0, 1, 0},
  {0, 0, 0, 0, 1, 0, 1, 0} // Right-most column
};

bool celsiusPattern[][8] = {
  {0, 0, 0, 0, 1, 1, 1, 0}, // Left-most column
  {0, 0, 0, 0, 1, 0, 1, 0},
  {0, 0, 0, 0, 1, 1, 1, 0},
  {0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 1, 1, 1, 1, 1, 0},
  {0, 0, 1, 0, 0, 0, 1, 0},
  {0, 0, 1, 0, 0, 0, 1, 0},
  {0, 0, 1, 0, 0, 0, 1, 0} // Right-most column
};

void setup() {
  pinMode(DIN, OUTPUT);
  pinMode(CS, OUTPUT);
  pinMode(CLK, OUTPUT);
  pinMode(GREEN, OUTPUT);
  pinMode(RED, OUTPUT);
  pinMode(SWITCH_TEMP_BUTTON, INPUT_PULLUP);
  Serial.begin(115200);

  initMatrix();
  showScheme();

  prevTempearature = getCelciusTemperature();
}

void loop() {
  int switch_button_state = digitalRead(SWITCH_TEMP_BUTTON);

  if (switch_button_state == LOW) {
    switchTemperatureUnit();

    if (isFarenheit) {
      prevTempearature = (prevTempearature * 9.0) / 5.0 + 32.0;
    }
    else {
      prevTempearature = (prevTempearature - 32.0) * 5.0 / 9.0;
    }
  }

  float T = getCelciusTemperature();

  if (isFarenheit) {
    T = (T * 9.0) / 5.0 + 32.0; // Convert Celcius to Farenheit
  }

  turnLedsByTemperatureDifference(T - prevTempearature);

  Serial.println(T); // Print the temperature (for debug purposes)

  if (T >= 0) { // Because negative temperature cannot be shown on the matrix
    int intTemp = T; // The integer part of the temperature
    double tempFraction = T - intTemp; // The fractional part of the temperature
    
    intTemp -= (intTemp / 100) * 100; // Keep only the 2 right digits of the temperature

    setLeftDigit(intTemp / 10); // Set the left digit on the matrix
    setRightDigit(intTemp % 10); // Set the right digit on the matrix

    // Set the size of the line at the bottom of the matrix
    if (tempFraction <= 0.25) {
      setQuarters(1);
    }
    else if (tempFraction <= 0.5) {
      setQuarters(2);
    }
    else if (tempFraction <= 0.75) {
      setQuarters(3);
    }
    else {
      setQuarters(4);
    }

    // Check whether the second row from the bottom should be turned on
    if (T >= 100) {
      setSecondRow(true);
    }
    else {
      setSecondRow(false);
    }

    showScheme(); // Show the new temperature on the matrix
    
    prevTempearature = T; // Save the current temperature to compare with the next measured temperature
  }

  delay(100);
}

/**
 * Initialize matrix configurations
 */
void initMatrix() {
  // Set registers: decode mode, scan limit, and shutdown (0x900, 0xB07, 0xC01)
  for (int i = 0; i < 4; i++) writeBit(LOW);
  writeBit(HIGH);
  for (int i = 0; i < 2; i++) writeBit(LOW);
  writeBit(HIGH);
  for (int i = 0; i < 8; i++) writeBit(LOW);
  latchBuf();
  for (int i = 0; i < 4; i++) writeBit(LOW);
  writeBit(HIGH);
  writeBit(LOW);
  writeBit(HIGH);
  writeBit(HIGH);
  for (int i = 0; i < 5; i++) writeBit(LOW);
  for (int i = 0; i < 3; i++) writeBit(HIGH);
  latchBuf();
  for (int i = 0; i < 4; i++) writeBit(LOW);
  for (int i = 0; i < 2; i++) writeBit(HIGH);
  for (int i = 0; i < 2; i++) writeBit(LOW);
  for (int i = 0; i < 7; i++) writeBit(LOW);
  writeBit(HIGH);
  latchBuf();
}

/**
 * Write 1 bit to the buffer
 */
void writeBit(bool b) {
  digitalWrite(DIN, b);
  digitalWrite(CLK, LOW);
  digitalWrite(CLK, HIGH);
}

/**
 * Latch the entire buffer
 */
void latchBuf() {
  digitalWrite(CS, LOW);
  digitalWrite(CS, HIGH);
}

/**
 * Write the scheme to the matrix
 */
void showScheme() {
  for (int i = 0; i < 8; i++) { // For each column of the matrix ////////////////////////////////put num of lines/cols in constant
    for (int j = 0; j < 8; j++) { // Write the number of the column (the first 8 bits)
      writeBit(cols[i][j]);
    }

    for (int k = 0; k < 8; k++) { // Write which LEDs to turn on in each column (the last 8 bits)
      writeBit(scheme[i][k]);
    }

    latchBuf();
  }
}

/**
 * Copy the digit pattern from the digits array to the left digit on the scheme
 */
void setLeftDigit(int digit) {
  for (int i = 0; i < 5; i++) {
    scheme[0][i + 3] = digits[digit][0][i];
    scheme[1][i + 3] = digits[digit][1][i];
    scheme[2][i + 3] = digits[digit][2][i];
  }
}

/**
 * Copy the digit pattern from the digits array to the right digit on the scheme
 */
void setRightDigit(int digit) {
  for (int i = 0; i < 5; i++) {
    scheme[5][i + 3] = digits[digit][0][i];
    scheme[6][i + 3] = digits[digit][1][i];
    scheme[7][i + 3] = digits[digit][2][i];
  }
}

/**
 * Turn on the line at the bottom of the matrix.
 * quarters=the amount of quarters of the line to show
 */
void setQuarters(int quarters) {
  for (int i = 0; i < quarters; i++) {
    scheme[i * 2][0] = 1;
    scheme[i * 2 + 1][0] = 1;
  }

  for (int i = 0; i < 4 - quarters; i++) {
    scheme[7 - i * 2][0] = 0;
    scheme[6 - i * 2][0] = 0;
  }
}

/**
 * Turn on or off the second line from the bottom of the matrix.
 * show=true to turn on, show=false to turn off.
 */
void setSecondRow(bool show) {
  for (int i = 0; i < 8; i++) {
    scheme[i][1] = show;
  }
}

/**
 * Switch between the temperature units (Celsius or Farenheit)
 */
void switchTemperatureUnit()
{
  isFarenheit = !isFarenheit;

  if (isFarenheit)
  {
    showTempSymbol(farenheitPattern);
  }
  else
  {
    showTempSymbol(celsiusPattern);
  }

  digitalWrite(GREEN, LOW);
  digitalWrite(RED, LOW);

  delay(500);
  resetMiddleCols(); // Turning off some cells that are not overridden by the temperature number
}

/**
 * Show temperature unit symbol
 */
void showTempSymbol(bool symbolMatrix[8][8]) {
  for (int i = 0; i < 8; i++) { // For each column of the matrix
    for (int k = 0; k < 8; k++) { // Write which LEDs to turn on in each column (the last 8 bits)
      scheme[i][k] = symbolMatrix[i][k];
    }
  }

  showScheme();
}

/**
 * Turning off the 5th column from the left and the 3rd line from the bottom
 * (which might be turned on after showing the temperature unit symbol
 * and not overridden by the temperature number)
 */
void resetMiddleCols() {
  for (int i = 0; i < 8; i++) {
    scheme[4][i] = 0;
  }
  
  for (int i = 0; i < 8; i++) {
    scheme[i][2] = 0;
  }
}

/**
 * Turn the green or the red LED in accordance with the difference between
 * the current temperature and the previous measured temperature
 */
void turnLedsByTemperatureDifference(float tempDiff) {
  float ledStrength = tempDiff * LED_LIGHT_CHANGE_SENSITIVITY;

  if (ledStrength < 0) {
  ledStrength*=-1.8;
  }
  
  if (ledStrength > 255) {
    ledStrength = 255;
  }

  if (tempDiff > 0) {
    analogWrite(RED, ledStrength);
    digitalWrite(GREEN, LOW);
  }
  else if (tempDiff < 0) 
  {
    analogWrite(GREEN, ledStrength);
    digitalWrite(RED, LOW);
  }
  else 
  {
    digitalWrite(GREEN, LOW);
    digitalWrite(RED, LOW);
  }
}

/**
 * Measure the current temperature
 */
float getCelciusTemperature() {
  int Vo = analogRead(THERMISTOR_PIN);
  float R2 = R1 * (1023.0 / (float)Vo - 1.0); // Calculate resistance on thermistor
  float logR2 = log(R2);
  float T = (1.0 / (c1 + c2 * logR2 + c3 * logR2 * logR2 * logR2)); // Temperature in Kelvin
  T = T - 273.15; // convert Kelvin to Celsius

  return T;
}
