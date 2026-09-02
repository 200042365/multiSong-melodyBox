#include <Arduino.h>

const int buttonPins[3] = {17, 18, 33};

const int RGB[3] = {1, 2, 4};

const int buzzer = 37;

const int NUM_SONGS = 3;
const int NUM_NOTES = 16;

const int NUM_RGB_PINS = 3;
const int NUM_BUTTONS = 3;

// -------------------- MUSICAL NOTES --------------------

const int REST    = 0;
const int NOTE_C4 = 262;
const int NOTE_D4 = 294;
const int NOTE_E4 = 330;
const int NOTE_F4 = 349;
const int NOTE_G4 = 392;
const int NOTE_A4 = 440;
const int NOTE_B4 = 494;
const int NOTE_C5 = 523;

// -------------------- SONG NAMES --------------------

const char* songNames[NUM_SONGS] ={
  " funky town",
  " smoke on the water",
  " addams family "
};

int melodies[NUM_SONGS][NUM_NOTES] = {

  // button 1 -- funkytown
  {
  NOTE_C5, NOTE_C5, NOTE_A4, NOTE_C5, 
  REST, NOTE_G4, REST, NOTE_G4, 
  NOTE_C5, NOTE_F4, NOTE_E4, NOTE_C5, 
  REST, REST, REST, REST
  },

    // button 2 -- smoke on the water
  {
    NOTE_E4, NOTE_G4, NOTE_A4, REST,    
    NOTE_E4, NOTE_G4, NOTE_B4, NOTE_A4,
    NOTE_E4, NOTE_G4, NOTE_A4, REST,    
    NOTE_G4, NOTE_E4, REST,
    REST
  },

      // button 3 -- addams family
  {
    NOTE_C4, NOTE_D4, NOTE_E4, NOTE_F4, 
    REST,    REST,    NOTE_D4, NOTE_E4, 
    NOTE_F4, NOTE_G4, REST,    REST,    
    NOTE_D4, NOTE_E4, NOTE_F4, NOTE_G4
  }
};

int noteDurations[NUM_SONGS][NUM_NOTES] = {
   {
    8, 8, 8, 8,    
    8, 4, 8, 4,
    4, 4, 4, 2,    
    4, 4, 4, 4
  },

   {
    4, 4, 4, 8,    
    4, 4, 8, 2,
    4, 4, 4, 8,    
    4, 2, 4, 4
  },
  
  {
    8, 8, 8, 8,    
    4, 4,    8, 8, 
    8, 8, 4, 4,    
    8, 8, 8, 8

  }
};

// -------------------- RGB COLOUR ARRAY --------------------

int colours[][3] = {
  {255,   0,   0},  // Red
  {  0, 255,   0},  // Green
  {  0,   0, 255},  // Blue
  {255, 255,   0},  // Yellow
  {255,   0, 255},  // Magenta
  {  0, 255, 255},  // Cyan
  {255, 100,   0},  // Orange
  {255, 255, 255}   // White
};

const int NUM_COLOURS = 8;

// -------------------- BUTTON VARIABLES --------------------

int buttonState[NUM_BUTTONS] = {
  HIGH, HIGH, HIGH
};

int lastButtonReading[NUM_BUTTONS] = {
  HIGH, HIGH, HIGH
};

unsigned long debounceStart[NUM_BUTTONS] = {
  0, 0, 0
};

const unsigned long DEBOUNCE_TIME = 50;

// -------------------- MELODY VARIABLES --------------------

int currentSong = -1;
int currentNote = 0;

bool songPlaying = false;
bool notePlaying = false;
bool gapPlaying = false;

unsigned long noteStartTime = 0;
unsigned long noteLength = 0;

unsigned long gapStartTime = 0;
unsigned long gapLength = 0;

// -------------------- RGB FUNCTIONS --------------------

void setColor(int colourNumber) {
  
  for (int channel = 0;
  channel < NUM_RGB_PINS;
  channel++) {

    analogWrite(
      RGB[channel],
      colours[colourNumber][channel]
    );
  }
}

void RGBoff() {

  for (int channel = 0; channel < NUM_RGB_PINS; channel++) {
    
    analogWrite(RGB[channel], 0);
  }
}

void startCurrentNote() {

  if(currentNote >= NUM_NOTES) {
    songPlaying = false;
    notePlaying = false;
    gapPlaying = false;

    noTone(buzzer);
    RGBoff();

    Serial.println("Song finished");
    return;
  }

  int frequency = melodies[currentSong][currentNote];

  noteLength = 1000 / noteDurations[currentSong][currentNote];

  int colourNumber = currentNote % NUM_COLOURS;

  if (frequency != REST) {
    tone(buzzer, frequency);
    setColor(colourNumber);
  }
  else {
    noTone(buzzer);
    RGBoff();
  }

  noteStartTime = millis();

  notePlaying = true;
  gapPlaying = false;
}

void startSong(int songNumber) {

  noTone(buzzer);
  RGBoff();

  currentSong = songNumber;
  currentNote = 0;
  songPlaying = true;

  Serial.print("Playing: ");
  Serial.println(songNames[currentSong]);

  // Start the first note immediately
  startCurrentNote();
}

void updateSong() {
  
  if (!songPlaying) {
    return;
  }

  unsigned long currentMillis = millis();

  if (notePlaying && currentMillis - noteStartTime >= noteLength) {
    noTone(buzzer);
    RGBoff();

    notePlaying = false;
    gapPlaying = true;

    // Start timing the gap between notes
    gapStartTime = currentMillis;

    // Gap is 30% of the note duration
    gapLength = noteLength * 30 / 100;
  }

  if (gapPlaying && currentMillis - gapStartTime >= gapLength) {

    gapPlaying = false;

    // Move to the next note
    currentNote++;

    // Start the next note
    startCurrentNote();
  }
}

// -------------------- CHECK BUTTONS --------------------

void checkButtons() {
  unsigned long currentMillis = millis();

  for (int button = 0; button < NUM_BUTTONS; button++) {

    int reading = digitalRead(buttonPins[button]);

    if (reading != lastButtonReading[button]) {
      debounceStart[button] = currentMillis;
    }

    if (currentMillis - debounceStart[button] >= DEBOUNCE_TIME) {

      if(reading != buttonState[button]) {
        buttonState[button] = reading;

        if (buttonState[button] == LOW) {
          Serial.print("Button selected: ");
          Serial.println(button + 1);

          startSong(button);
        }
      }
    }

    lastButtonReading[button] = reading;
  }
}

void setup() {
  Serial.begin(115200);

  for (int x = 0; x < NUM_BUTTONS; x++) {
    pinMode(buttonPins[x], INPUT_PULLUP);
  }

  for (int x = 0; x < NUM_RGB_PINS; x++){
    pinMode(RGB[x], OUTPUT);
  }

  pinMode(buzzer, OUTPUT);

  RGBoff();
  noTone(buzzer);

  Serial.println("Melody player ready");
  Serial.println("Button 1: funky town");
  Serial.println("Button 2: smoke on the water");
  Serial.println("Button 3: addams family");
}

void loop() {
  checkButtons();
  updateSong();
}
