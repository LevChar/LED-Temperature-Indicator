/**
 * Project: LED Temperature Indicator.
 * purpose: This Project is used for collection of the current environmental 
			temperature and displaying it on a 8x8 LED display. 
			The temperature is calculated according to the Steinhart–Hart equation,
			based on samples of the resistance, taken from the thermistor.
			Update rate is 0.1 seconds.
 */

/* ******************************************************************************* */

/**
  *  Basic settings & pin Configuration  *
										 */

typedef unsigned int Uint;

// Matrix pins
const Uint DIN = 3;
const Uint CLK = 2;
const Uint CS = 4;

// Led pins
const Uint GREEN = 5;
const Uint RED = 6;

// Switch button pin
const Uint SWITCH_TEMP_BUTTON = 10; 

// Thermistor pin
const Uint THERMISTOR_PIN = A0;

//Starting columns of the digits on the matrix
const Uint LEFT_DIGIT = 0;
const Uint RIGHT_DIGIT = 5;

// Steinhart-Hart coeficients for thermistor & thermistor resistor resistance
const float R1 = 10000; 
const float c1 = 0.001129148;
const float c2 = 0.000234125;
const float c3 = 0.0000000876741;

// Variables used in calculations and in-display methods
const Uint maxLedStrength = 255;
const int decreaseInTempSensitivity = -1.8;
const float ledLightChangeSensitivity = 150;
float prevTempearature;
bool isFarenheit = false;

/* ******************************************************************************* */

/**
 * Bit encodings of led patterns (Basic scheme, digits, temp symbols) *
																	  */

The number of each column on the matrix (the first 8 bits)
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

// LEDs pattern to turn on the matrix (used like a place holder for display updates)
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

/* ******************************************************************************* */

/**
 * Arduino Basic functions - Setup & Loop *
										  */

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

  if (switch_button_state == LOW)
	switchTemperatureUnit();

  float T = getCelciusTemperature();
  
  // Convert Celcius to Farenheit if needed
  T = isFarenheit ? ((T * 9.0) / 5.0 + 32.0) : T ;

  calcChangeByTempDiffAndTurnOnLEDs(T - prevTempearature);

  // Serial.println(T); // Print the temperature (for debug purposes)

  // Negative temperature cannot be shown on the matrix
  if (T >= 0) { 
	//Temp calculation for fraction status bar
    int intTemp = T; 				   // The integer part
    double tempFraction = T - intTemp; // The fractional part
    
	// Keep only the 2 list significant digits of the temperature 
	// (used for displaing farenheit temperatures above 100 degrees).
    intTemp -= (intTemp / 100) * 100; 

	// Update scheme matrix with new temperature
    setDigit(LEFT_DIGIT, intTemp / 10);  // Left digit
    setDigit(RIGHT_DIGIT, intTemp % 10); // Right digit

    // update fraction status bar in scheme matrix
	setQuarters(tempFraction * 4);
   
	// In farenheit mode, update the second status bar (one line before the last 
	// in scheme matrix) to fully loaded state to indicate temperature above 100
    if (T >= 100) {
      setSecondRow(true);
    }
    else {
      setSecondRow(false);
    }

	// Show the updated temperature on the matrix
    showScheme(); 
	
	// Save current temperature for comparison with next measured temperature
    prevTempearature = T; 
  }

  delay(100);
}

/* ******************************************************************************* */

/**
 * Low level functions, implementing cummunication with MAX7219 LED Display Driver *
																				   */

/**
 * Write 1 bit to the buffer.
 */
void writeBit(bool b) {
  digitalWrite(DIN, b);
  digitalWrite(CLK, LOW);
  digitalWrite(CLK, HIGH);
}

/**
 * Latch the entire buffer.
 */
void latchBuf() {
  digitalWrite(CS, LOW);
  digitalWrite(CS, HIGH);
}

/* ******************************************************************************* */

/**
 * Matrix initialization and display functions *
											   */

/**
 * Initialize matrix configuration.
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
 * Write the scheme to the matrix.
 */
void showScheme() {
  // Repeat for each of the 8 columns of the matrix
  for (int i = 0; i < 8; i++) { 
	// Write column # to the buffer (the first 8 bits)
    for (int j = 0; j < 8; j++) { 
      writeBit(cols[i][j]);
    }
	
	// Write which LEDs to turn on the column (the last 8 bits)
    for (int k = 0; k < 8; k++) { 
      writeBit(scheme[i][k]);
    }

    latchBuf();
  }
}

/* ******************************************************************************* */

/**
 *  Manipulation of Patterns for displaying on the matrix *
													      */

/**
 * Copy digit pattern from the digits array to the appropriate digit location 
 * on the scheme.
 */
void setDigit(Uint digitLocation, int digit) {
  for (int i = 0; i < 5; i++) {
    scheme[digitLocation + 0][i + 3] = digits[digit][0][i];
    scheme[digitLocation + 1][i + 3] = digits[digit][1][i];
    scheme[digitLocation + 2][i + 3] = digits[digit][2][i];
  }
}

/**
 * Turn on the status bar at the bottom of the matrix
 * according to the quarters value (0.00 - 0.25 - 0.50 - 1.00).
 */
void setQuarters(int quarters) {
  
  //Clear previous status bar data
  for (int i = 0; i < 8 ; i++) {
    scheme[i][0] = 0;
  }
  
  //Set current status bar data
  for (int i = 0; i < quarters; i++) {
    scheme[i * 2][0] = 1;
    scheme[i * 2 + 1][0] = 1;
  }

  
}

/**
 * Turn on/off the second line from the bottom of the matrix.
 * show=true to turn on, show=false to turn off.
 */
void setSecondRow(bool show) {
  for (int i = 0; i < 8; i++) {
    scheme[i][1] = show;
  }
}

/**
 * Show temperature unit symbol
 */
void showTempSymbol(bool symbolMatrix[8][8]) {
  // Repeat for each of the 8 columns of the matrix
  for (int i = 0; i < 8; i++) {
	// Write the LEDs to turn on in each column (the last 8 bits)
    for (int k = 0; k < 8; k++) { 
      scheme[i][k] = symbolMatrix[i][k];
    }
  }
  
  showScheme();
}

/**
 * Turn off 4th & 5th column from the left and the 3rd line from the bottom
 * (which might be turned on after showing the symbol of the temperature unit
 * and not overridden by the temperature number)
 */
void resetMiddleColsAndRows() {
  for (int i = 0; i < 8; i++) {
    scheme[3][i] = 0;
	scheme[4][i] = 0;
  }
  
  for (int i = 0; i < 8; i++) {
    scheme[i][2] = 0;
  }
}

/* ******************************************************************************* */

/**
 *  Temperature calculation & switching between unints(Celsius & Farenheit)*
																		   */
													   
/**
 * Measure the current temperature
 */
float getCelciusTemperature() {
  int Vo = analogRead(THERMISTOR_PIN);
  
  // Calculate resistance on thermistor
  float R2 = R1 * (1023.0 / (float)Vo - 1.0); 
  float logR2 = log(R2);
  
  // Temperature in Kelvin
  float T = (1.0 / (c1 + c2 * logR2 + c3 * logR2 * logR2 * logR2)); 
  
  // Convert Kelvin to Celsius
  T = T - 273.15; 

  return T;
}

/**
 * Switch between the temperature units (Celsius or Farenheit)
 */
void switchTemperatureUnit()
{
  isFarenheit = !isFarenheit;

  if (isFarenheit)
  {
	prevTempearature = (prevTempearature * 9.0) / 5.0 + 32.0;
    showTempSymbol(farenheitPattern);
  }
  else
  {
	prevTempearature = (prevTempearature - 32.0) * 5.0 / 9.0;
    showTempSymbol(celsiusPattern);
  }

  digitalWrite(GREEN, LOW);
  digitalWrite(RED, LOW);

  delay(500);
  
  // Turn off columns and rows which are not used for displaying the temperature
  resetMiddleColsAndRows(); 
}

/* ******************************************************************************* */

/**
 *  Manipulation of LEDs according changes in temperature  *
														   */
/**
 * Calculate the intensity of the change between currently measured
 * temperature and previously measured temperature.
 * ledLightChangeSensitivity & decreaseInTempSensitivity are coeficients 
 * which can be adjusted for specific LED brand and the desire effects.
 */
void calcChangeByTempDiffAndTurnOnLEDs(float tempDiff) {
  float ledStrength = tempDiff * ledLightChangeSensitivity;
  ledStrength = < 0 ? (ledStrength * decreaseInTempSensitivity) : ledStrength;
  ledStrength = ledStrength > maxLedStrength ? : maxLedStrength : ledStrength;
  turnLedsWithDiffStrength(tempDiff, ledStrength);
}

/**
 * Turn green or red LED in accordance with the intensity of the change
 * of the current temperature with respect to the previously measured temperature.
 */
void turnLedsWithDiffStrength(float difference, float strength) {
	// Increase in temperature
	if (tempDiff > 0) {
		analogWrite(RED, ledStrength);
		digitalWrite(GREEN, LOW);
	}
	
	// Decrease in temperature
	else if (tempDiff < 0) 
	{
		analogWrite(GREEN, ledStrength);
		digitalWrite(RED, LOW);
	}
	
	// No change in temperature
	else 
	{
		digitalWrite(GREEN, LOW);
		digitalWrite(RED, LOW);
	}
}
