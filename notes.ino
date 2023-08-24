#include <DFMiniMp3.h>
#include <LiquidCrystal.h>

// lcd stuff
const int rs = 12, en = 11, d4 = 5, d5 = 4, d6 = 3, d7 = 2;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

// keypad stuff
const int BASE_PIN = 40;  // the number of the pushbutton pin
const int NUM_PINS = 14; // 7 physical keys, skiping every other pin
int buttonState = 0;  // variable for reading the pushbutton status

// sound stuff
bool isNotePlaying;
class Mp3Notify; 
typedef DFMiniMp3<HardwareSerial, Mp3Notify> DfMp3; 
DfMp3 dfmp3(Serial2);

// game logic
const int GAME_MODE_PRACTICE = 0;
const int GAME_MODE_PLAY = 1;
const int GAME_MODE_UNDEF = -1;
int gameMode = GAME_MODE_UNDEF;

int simonNotesLength;
int simonNotes[100];  // the sequence of notes to guess.

const int TOTAL_NUM_TRIES = 1;  // this is hard, give user a few tries.
int numTries;

const int SOUND_CORRECT = 8;
const int SOUND_WRONG = 9;
const int SOUND_GAME_OVER = 10;
const int SOUND_YOUR_TURN = 11;

void getCardStats()  
/* Makes sure we have good conncetion to sound card. */
{
  uint16_t version = dfmp3.getSoftwareVersion();
  Serial.print("version ");
  Serial.println(version);

  uint16_t volume = dfmp3.getVolume();
  Serial.print("volume ");
  Serial.println(volume);
  //dfmp3.setVolume(1);
  
  uint16_t count = dfmp3.getTotalTrackCount(DfMp3_PlaySource_Sd);
  Serial.print("files ");
  Serial.println(count);
}

/* Runs every time we lose or turn on. */
void start_over() { 
  Serial.println("Starting Over");
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Welcome to Notes!");
  lcd.setCursor(0, 1);
  lcd.print("I'll play notes.");
  lcd.setCursor(0, 2);
  lcd.print("You play them back.");
  lcd.setCursor(0, 3);
  lcd.print("Ready?");

  // initialize simon notes
  randomSeed(millis());
  for (int i=0; i<100; i++) {
    simonNotes[i] = random(1, 8);
  }
  // wait for card to init so it wont break up playing first note
  waitMilliseconds(7000); 

  simonNotesLength = 0;
  numTries = 0;
  simonNotesLength = 0;
  isNotePlaying = false;
}

void setup() {
  Serial.begin(115200);
  Serial.println("Setting up...");
  lcd.begin(20,4);
  // initialize the input pins
  for (int i=0; i<NUM_PINS; i+=2)
  {
    int pin = BASE_PIN + i;
    pinMode(pin, INPUT);
  }
  dfmp3.begin();
  getCardStats();

  start_over();
}

void waitMilliseconds(uint16_t msWait)
{
  uint32_t start = millis();
  
  while ((millis() - start) < msWait)
  {
    // if you have loops with delays, its important to 
    // call dfmp3.loop() periodically so it allows for notifications 
    // to be handled without interrupts
    dfmp3.loop(); 
    delay(10);
  }
}

void play_piano() { 
  uint32_t pressedKeysStart[NUM_PINS];
  for (int i=0; i<NUM_PINS; i+=2)
  {
    pressedKeysStart[i] = 0;
  }
  while (true)
  {
    // wait for user input
    for (int i=0; i<NUM_PINS; i+=2)
    {
      uint32_t now = millis();
 
      int pin = BASE_PIN + i;
      buttonState = digitalRead(pin);
      if (buttonState == HIGH) {
          if (now -  pressedKeysStart[i] > 3*1000) { 
            pressedKeysStart[i] = millis();
            Serial.println("First press.");
          }
          else if (now - pressedKeysStart[i] > 100) { // held for half second, good key press.
            Serial.println("Registered a keypress.");
            int note = (pin-BASE_PIN)/2 + 1;
            String noteS = "";
            if (note == 1) { 
              noteS = "C";
            }
            else if (note == 2) {
              noteS = "D";
            }
            else if (note == 3) {
              noteS = "E";
            }
            else if (note == 4) {
              noteS = "F";
            }
            else if (note == 5) {
              noteS = "G";
            }
            else if (note == 7) {
              noteS = "A";
            }
            else if (note == 8) {
              noteS = "B";
            }

            lcd.clear();
            lcd.setCursor(0, 0);
            lcd.print("You played the note:");
            lcd.setCursor(0, 1);
            lcd.print("         " + noteS);
            Serial.println(String(note));
            play_sound_and_wait(note);
        }
      }
    }
  }
}

void play_simon() { 
  simonNotesLength++;
  Serial.println("My Turn...");
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("My Turn. Listen...");
  print_status();

  Serial.println("");
  for (int i=0; i<simonNotesLength; i++) { 
    Serial.print(simonNotes[i]);
    Serial.print(",");
    play_sound_and_wait(simonNotes[i]);
  }
}

void play_sound_and_wait(int sound) {
  isNotePlaying = true;
  dfmp3.playMp3FolderTrack(sound); 
  while (isNotePlaying) {  // wait for note to finish
    waitMilliseconds(50); 
  } 
}

void wrong_answer() { 
  Serial.println("Oops. you are wrong!>");
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Wrong Answer!");
  play_sound_and_wait(SOUND_WRONG);
  waitMilliseconds(3000); 
  numTries ++;
  simonNotesLength--;
  if (numTries > TOTAL_NUM_TRIES)
  {
    play_sound_and_wait(SOUND_GAME_OVER);  
    Serial.println("You lose!!!!");
    lcd.clear();    
    lcd.setCursor(0, 0);
    lcd.print("Game Over :(");
    lcd.setCursor(0, 1);
    lcd.print("xxxxxxxxxxxxxxxxxxxx");
    lcd.setCursor(0, 2);
    lcd.print("xxxxxxxxxxxxxxxxxxxx");
    lcd.setCursor(0, 3);
    lcd.print("xxxxxxxxxxxxxxxxxxxx");
    waitMilliseconds(5000); 
    start_over();
  }
}

void print_status() { 
  lcd.setCursor(0, 1);
  lcd.print("Level: " + String(simonNotesLength)) ;
  lcd.setCursor(0, 2);
  lcd.print("Tries Left: " + String(TOTAL_NUM_TRIES - numTries));
}
 
void play_user() { 
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Your Turn.");
  print_status();
  int numUserKeys = 0;  // number index of key to read from the user
  uint32_t pressedKeysStart[NUM_PINS]; // holds when the last key press was for each key
  for (int i=0; i<NUM_PINS; i+=2)
  {
    pressedKeysStart[i] = 0;
  }
  int lastKeyPressed = -1;
  play_sound_and_wait(SOUND_YOUR_TURN);
  Serial.println("Waiting for user input...");
  bool is_correct = true;

  while (numUserKeys < simonNotesLength)  // keep reading keys from the user
  {
    // wait for user input
    for (int i=0; i<NUM_PINS; i+=2) // only using even pins
    {
      uint32_t now = millis();
 
      int pin = BASE_PIN + i;
      buttonState = digitalRead(pin);
      if (buttonState == HIGH) {
          if (now -  pressedKeysStart[i] > 2*1000) {   // if key wasnt already down (2 sec) register first press.
            pressedKeysStart[i] = millis();
            lastKeyPressed = -1;
            Serial.println("First press.");
          }
          else if (now - pressedKeysStart[i] > 300 && lastKeyPressed != i) { // key held for x ms, good key press.
            numUserKeys ++;  
            Serial.println("Registered a keypress.");
            lastKeyPressed = i;
            int note = (pin-BASE_PIN)/2 + 1;
            Serial.println(String(note));
            play_sound_and_wait(note);

            /// now check if it's right .! 
            if (simonNotes[numUserKeys-1] != note)
            {
              numUserKeys=100; // break out of the loop
              is_correct = false;
              wrong_answer();
            }
          }
        }
    }
  }
  if (is_correct) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Right. Great Answer!");
    Serial.println("Correct Answer!!");
    play_sound_and_wait(SOUND_CORRECT);
  }

}

void set_game_mode() { 
  if (gameMode == GAME_MODE_UNDEF) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Select Game Mode:");
    lcd.setCursor(0, 1);
    lcd.print("  1 for Practice");
    lcd.setCursor(0, 2);
    lcd.print("  2 for Game Play");

    // wait for user input
    for (int i=0; i<NUM_PINS; i+=2) // only using even pins
    {
      uint32_t now = millis();
 
      int pin = BASE_PIN + i;
      buttonState = digitalRead(pin);
      if (buttonState == HIGH) {
        int note = (pin-BASE_PIN)/2 + 1;

        if (note == 1) { 
          gameMode = GAME_MODE_PRACTICE;
          Serial.println("Your game mode is Practice.");
        }
        else {
          gameMode = GAME_MODE_PLAY;
          Serial.println("Your game mode is Play.");
        }
      }
    }
  }
}

void loop() {
  set_game_mode();
  gameMode = GAME_MODE_PRACTICE;
  if (gameMode == GAME_MODE_PLAY) {
    play_simon();  
    play_user();
  }
  else {
    play_piano();
  }
}

/*  ******************************* End program **********************************/
/*  only other modification is onPlayFinish */
/*  ******************************************************************************/

class Mp3Notify
{
  
public:
  static uint32_t tracknum;
  static uint32_t LAST_FINISH;
  static void PrintlnSourceAction(DfMp3_PlaySources source, const char* action)
  {
    if (source & DfMp3_PlaySources_Sd) 
    {
        Serial.print("SD Card, ");
    }
    if (source & DfMp3_PlaySources_Usb) 
    {
        Serial.print("USB Disk, ");
    }
    if (source & DfMp3_PlaySources_Flash) 
    {
        Serial.print("Flash, ");
    }
    Serial.println(action);
  }
  static void OnError([[maybe_unused]] DfMp3& mp3, uint16_t errorCode)
  {
    // see DfMp3_Error for code meaning
    Serial.println();
    Serial.print("Com Error ");
    Serial.println(errorCode);
  }
  
  static void OnPlayFinished([[maybe_unused]] DfMp3& mp3, [[maybe_unused]] DfMp3_PlaySources source, uint16_t track)
  {
    uint32_t now=millis();
    // prevent double invocation
    if(now - LAST_FINISH < 200){
        Serial.println("Skip double Invocation ("+String(now - LAST_FINISH)+" millis)");
        LAST_FINISH=now;
        return;
    }
    isNotePlaying = false;
    LAST_FINISH=now;
  }
  static void OnPlaySourceOnline([[maybe_unused]] DfMp3& mp3, DfMp3_PlaySources source)
  {
    PrintlnSourceAction(source, "online");
  }
  static void OnPlaySourceInserted([[maybe_unused]] DfMp3& mp3, DfMp3_PlaySources source)
  {
    PrintlnSourceAction(source, "inserted");
  }
  static void OnPlaySourceRemoved([[maybe_unused]] DfMp3& mp3, DfMp3_PlaySources source)
  {
    PrintlnSourceAction(source, "removed");
  }
};
uint32_t Mp3Notify::tracknum=1;
uint32_t Mp3Notify::LAST_FINISH=millis();
