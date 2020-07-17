// Based upon: https://gammon.com.au/forum/?id=14175

#include <Keyboard.h>

#define KEY_MOD_LEFT(x) x[2][0]
#define KEY_MOD_RIGHT(x) x[2][12]
#define KEY_MOD_FUNC(x) x[3][0]

#define NROWS 4
#define NCOLUMNS 14

const unsigned char rowPins[4] = {16, 15, 14, 23};
const unsigned char columnPins[14] = {22, 21, 20, 19, 18, 13, 8, 7, 6, 5, 4, 3, 2, 17};

//
//
// Arrays for holding keyboard layout
//
//

const unsigned char lettersLayer[4][14] = {
  {KEY_TAB, KEY_ESC, 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '&', KEY_BACKSPACE},
  {KEY_LEFT_CTRL, 'a', 's', 'd', 'f', 'g', 'h','j', 'k', 'l', ';', KEY_RETURN, 0, 0},
  {0, KEY_LEFT_SHIFT, 'z', 'x', 'c', 'v', 'b', 'n', 'm', '.', '/', KEY_RIGHT_SHIFT, 0, 0},
  {0, KEY_LEFT_ALT, KEY_LEFT_GUI, ' ', KEY_RIGHT_GUI, 0, 0, 0, 0, 0, 0, 0, 0, 0}
};

const unsigned char symbolsLayer[4][14] = {
  {'|', 0, '(', ')', '#', '`', 0, 0, 0, 0, 0, 0, 0, '~'},
  {KEY_LEFT_CTRL, '{', '}', '"', '@', 0, 0, 0, 0, 0, 0, 0, 0, 0},
  {0, KEY_LEFT_SHIFT, '[', ']', '\'', '\\', 0, 0, 0, 0, 0, KEY_RIGHT_SHIFT, 0, 0},
  {0, KEY_LEFT_ALT, KEY_LEFT_GUI, '_', KEY_RIGHT_GUI, 0, 0, 0, 0, 0, 0, 0, 0, 0}
};

const unsigned char numbersLayer[4][14] = {
  {0, 0, '7', '8', '9', 0, 0, 0, 0, 0, '*', '%', '?', '!'},
  {KEY_LEFT_CTRL, '4', '5', '6', 0, 0, 0, 0, 0, '-', '+', '=', 0, 0},
  {0, 0, '1', '2', '3', '.', 0, '<', '>', '^', '$', 0, 0, 0},
  {0, KEY_LEFT_ALT, '0', ',', KEY_RIGHT_GUI, 0, 0, 0, 0, 0, 0, 0, 0, 0}
};

const unsigned char arrowsLayer[4][14] = {
  {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, KEY_UP_ARROW, 0, 0},
  {KEY_LEFT_CTRL, 0, 0, 0, 0, 0, 0, 0, 0, KEY_LEFT_ARROW, KEY_RIGHT_ARROW, 0, 0, 0},
  {0, KEY_LEFT_SHIFT, 0, 0, 0, 0, 0, 0, 0, 0, KEY_DOWN_ARROW, KEY_RIGHT_SHIFT, 0, 0},
  {0, KEY_LEFT_ALT, KEY_LEFT_GUI, 0, KEY_RIGHT_GUI, 0, 0, 0, 0, 0, 0, 0, 0, 0}
};

//
//
// Arrays for holding keyboard state
//
//

// 2D array to hold raw state of switch array
unsigned char prevArrayState[NROWS][NCOLUMNS];
unsigned char currArrayState[NROWS][NCOLUMNS];

// Variables to hold cleaned up (debounced) state of MOD keys
unsigned char leftModCleanState;
unsigned char rightModCleanState;

// Array to hold state of USB HID keys
unsigned char prevHidState[256];
unsigned char currHidState[256];

// Last time a particular USB HID was sent/pressed
unsigned long lastPress[NROWS][NCOLUMNS];

// Time for heartbeat key release
unsigned long lastHeartBeat;

//
//
// Initialize and scan array
//
//

void setup() {
  // put your setup code here, to run once:
  pinMode(11, OUTPUT);
  digitalWrite(11, LOW);
  delay(1000);
  Keyboard.begin();
  //Serial.begin(19200);
  delay(1000);

  unsigned long currMillis = millis();
  for(byte col = 0; col < NCOLUMNS; col++) {
    for(byte row = 0; row < NROWS; row++) {
        lastPress[row][col] = currMillis;
    }
  }

  lastHeartBeat = millis();
}

void loop() {
  // put your main code here, to run repeatedly:

   // Scan array
  for(byte col = 0; col < NCOLUMNS; col++) {
    //Serial.println(col);
    pinMode(columnPins[col], OUTPUT);
    digitalWrite(columnPins[col], LOW);

    for(byte row = 0; row < NROWS; row++) {
      //Serial.println(row);
      pinMode(rowPins[row], INPUT_PULLUP);
      currArrayState[row][col] = !digitalRead(rowPins[row]);
      //Serial.println(currArrayState[row][col]);
      //delay(100);
    }
    pinMode(columnPins[col], INPUT);
  }

  // Record state change time for debounce
  unsigned long currMillis = millis();
  
  for(byte col = 0; col < NCOLUMNS; col++) {
    for(byte row = 0; row < NROWS; row++) {
      if(currArrayState[row][col] != prevArrayState[row][col]) {
        lastPress[row][col] = currMillis;
      }
    }
  }

  /*
  for(byte col = 0; col < NCOLUMNS; col++) {
    for(byte row = 0; row < NROWS; row++) {
      currHidState[lettersLayer[row][col]] = currArrayState[row][col];
    }
  }
  */

  if((currMillis - KEY_MOD_LEFT(lastPress)) > 30) {
    leftModCleanState = KEY_MOD_LEFT(currArrayState);
  }
  if((currMillis - KEY_MOD_RIGHT(lastPress)) > 30) {
    rightModCleanState = KEY_MOD_RIGHT(currArrayState);
  }
    
  for(byte col = 0; col < NCOLUMNS; col++) {
    for(byte row = 0; row < NROWS; row++) {
      if((currMillis - lastPress[row][col]) > 30) {

        // Select layer
        if((leftModCleanState == 0) && (rightModCleanState == 0)) {
          currHidState[symbolsLayer[row][col]] = 0;
          currHidState[numbersLayer[row][col]] = 0;
          currHidState[arrowsLayer[row][col]] = 0;
          currHidState[lettersLayer[row][col]] = currArrayState[row][col];
        }
        else if((leftModCleanState == 1) && (rightModCleanState == 0)) {
          currHidState[symbolsLayer[row][col]] = currArrayState[row][col];
        }
        else if((leftModCleanState == 0) && (rightModCleanState == 1)) {
          currHidState[numbersLayer[row][col]] = currArrayState[row][col];
        }
        else if((leftModCleanState == 1) && (rightModCleanState == 1)) {
          currHidState[arrowsLayer[row][col]] = currArrayState[row][col];
        }
        
      }
    }
  }

  for(int i = 0; i < 256; i++) {
    if((prevHidState[i] == 0) && (currHidState[i] == 1)) {
       /*byte x = i;
       Serial.write(&x, 1);
       Serial.println();*/
       Keyboard.press(i);
    }
    if((prevHidState[i] == 1) && (currHidState[i] == 0)) {
       Keyboard.release(i);
    }
  }

  if((currMillis - lastHeartBeat) > 200) {
    uint8_t sum = 0;
    for(int i = 0; i < 256; i++) {
      sum += currHidState[i];
    }
    if(sum == 0) {
      Keyboard.releaseAll();
    }
    lastHeartBeat = currMillis;
  }

  memcpy(prevArrayState, currArrayState, sizeof(currArrayState[0][0])*NROWS*NCOLUMNS);
  memcpy(prevHidState, currHidState, sizeof(currHidState[0])*256);
}
