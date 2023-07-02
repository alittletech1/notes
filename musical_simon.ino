const int BASE_PIN = 40;  // the number of the pushbutton pin
const int NUM_PINS = 14; // 7 physical keys, skiping every other pin

int buttonState = 0;  // variable for reading the pushbutton status
bool isNotePlaying;
int simonNotesLength;
int simonNotes[100];  // the sequence of notes to guess.

const int TOTAL_NUM_TRIES = 1;  // this is hard, give user a few tries.
int numTries;

const int SOUND_CORRECT = 8;
const int SOUND_WRONG = 9;
const int SOUND_GAME_OVER = 10;
const int SOUND_YOUR_TURN = 11;

#include <DFMiniMp3.h>

// forward declare the notify class, just the name
//
class Mp3Notify; 

// define a handy type using serial and our notify class
//
typedef DFMiniMp3<HardwareSerial, Mp3Notify> DfMp3; 

DfMp3 dfmp3(Serial2);

void getCardStats()
{
  uint16_t version = dfmp3.getSoftwareVersion();
  Serial.print("version ");
  Serial.println(version);

  uint16_t volume = dfmp3.getVolume();
  Serial.print("volume ");
  Serial.println(volume);
  dfmp3.setVolume(30);
  
  uint16_t count = dfmp3.getTotalTrackCount(DfMp3_PlaySource_Sd);
  Serial.print("files ");
  Serial.println(count);
}

void setup() {
  // initialize the input pins
  for (int i=0; i<NUM_PINS; i+=2)
  {
    int pin = BASE_PIN + i;
    pinMode(pin, INPUT);
  }
  Serial.begin(115200);
  dfmp3.begin();
  getCardStats();
  Serial.println("starting...");
  
  // initialize simon notes
  randomSeed(millis());
  for (int i=0; i<100; i++) {
    simonNotes[i] = random(1, 8);
  }
  // wait for card to init so it wont break up playing first note
  waitMilliseconds(3000); 

  simonNotesLength = 0;
  numTries = 0;
  simonNotesLength = 0;
  isNotePlaying = false;
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
            Serial.println(String(note));
            play_sound_and_wait(note);
        }
      }
    }
  }
}

void play_simon() { 
  Serial.println("Simons Turn...");
  simonNotesLength++;
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
  play_sound_and_wait(SOUND_WRONG);
  numTries ++;
  simonNotesLength--;
  if (numTries > TOTAL_NUM_TRIES)
  {
    play_sound_and_wait(SOUND_GAME_OVER);
    Serial.println("You lose!!!!");
    waitMilliseconds(1000); 
    setup();
  }
}

void play_user() { 
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
//        else {
//          pressedKeysStart[i] = 0;  // for next round, rest the key start.
//       }
    }
  }
  if (is_correct) {
    Serial.println("Correct Answer!!");
    play_sound_and_wait(SOUND_CORRECT);
  }

}

void loop() {

  play_simon();  
  play_user();
}
void loopx(){
  play_piano();
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
