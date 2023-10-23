#include <DFMiniMp3.h>
#include <SoftwareSerial.h>
// forward declare the notify class, just the name
//
class Mp3Notify; 

// define a handy type using serial and our notify class
//
typedef DFMiniMp3<HardwareSerial, Mp3Notify> DfMp3; 

// instance a DfMp3 object, 
//
DfMp3 dfmp3(Serial3);

class Mp3Notify
{
  
public:
  static uint32_t LAST_FINISH; 
  static uint32_t tracknum;
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
    // prevent double invocation
    uint32_t now=millis();
        if(now - LAST_FINISH < 200){
        Serial.println("Skip double Invocation ("+String(now - LAST_FINISH)+" millis)");
        LAST_FINISH=now;
        return;
    }
    LAST_FINISH=now;
    Serial.print("Play finished for #");
    Serial.println(tracknum);  

    // start next track
    tracknum += 1;
    // this example will just start back over with 1 after track 3
    if (tracknum > 7) 
    {
      tracknum = 1;
    }
    
    dfmp3.playMp3FolderTrack(tracknum);  // sd:/mp3/0001.mp3, sd:/mp3/0002.mp3, sd:/mp3/0003.mp3
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
uint32_t Mp3Notify::LAST_FINISH=millis();
uint32_t Mp3Notify::tracknum=1;

void setup() 
{
  Serial.begin(115200);

  Serial.println("initializing...");
  
  dfmp3.begin();
  // for boards that support hardware arbitrary pins
  //dfmp3.begin(16, 17, 115200); // RX, TX

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
  
  Serial.println("starting...");

  // start the first track playing
  dfmp3.playMp3FolderTrack(1);  // sd:/mp3/0001.mp3
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
    delay(1);
  }
}

void loop() 
{
  waitMilliseconds(10);
}
