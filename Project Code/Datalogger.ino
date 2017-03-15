#include <AddicoreRFID.h>
#include <Wire.h>
#include "RTClib.h"
#include <SPI.h>
#include <SD.h>

AddicoreRFID myRFID;
RTC_DS1307 rtc;

const int chipSelectSD = 10;
const int chipSelectRFID = 7;
const int resetRFID = 3;
const char* saveFile = "dl.txt";
#define MAX_LEN 16

void setup()
{
  // Open serial communications
  Serial.begin(9600);

  //initialize SPI communications used for SD and RFID 
  SPI.begin();

  //initialize real time clock pins 4 and 5
  rtc.begin();


  Serial.print("Initializing SD card...");

  // see if the card is present and can be initialized and wait 1 second and try again if not present
  while(!SD.begin(chipSelectSD)) {
    Serial.println("Card failed, or not present");
    delay(1000);
  }

  //set the values for RFID SPI communication and initialize
  myRFID.Advanced_Setup_AddicoreRFID(chipSelectRFID, resetRFID);
  myRFID.AddicoreRFID_Init();

  Serial.println("card initialized.");
}


void loop()
{
  unsigned char i, tmp, checksum, stat;
  unsigned char str[MAX_LEN];
  char *out = (char*)malloc(20 * sizeof(char));;
  str[1] = 0x4400;
  stat = myRFID.AddicoreRFID_Request(PICC_REQIDL , str);//search area for cards
  stat = myRFID.AddicoreRFID_Anticoll(str);//read selected card with anticollision detection

  if (stat == MI_OK)//if stat = MI_OK then a card has been succesfully read
  {
    checksum = str[0] ^ str[1] ^ str[2] ^ str[3];//calculate checksum
    
    //print RFID UNID
    Serial.print(str[0]);
    Serial.print(" , ");
    Serial.print(str[1]);
    Serial.print(" , ");
    Serial.print(str[2]);
    Serial.print(" , ");
    Serial.println(str[3]);

    //print out calculated checksum (checksum) and recieved checksum (str[4])
    Serial.println(str[4]);
    Serial.println(checksum);

    //if checksums do not match cancel SD card save and return to start of loop
    Serial.println();
    if(str[4]!= checksum){
      Serial.println("checksums do not match");
      free(out);
      myRFID.AddicoreRFID_Halt();//puts card into hibernation to avoid filling up chip's buffer
      return;
    }

    sprintf(out, "%d,%d,%d,%d", str[0], str[1], str[2], str[3]);//compile char* to save the UNID to SD card
    
    saveMessage(out, (char*)saveFile);//save out to SD card with date and time to file savefile
    
    delay(2000);//wait 2 second before re-reading to avoid reading the same card twice accidentally
  }
  free(out);
  myRFID.AddicoreRFID_Halt();//puts card into hibernation to avoid filling up chip's buffer
}


//
//
//
//
//
//
//save str and current date and time to and SD card file specified by fileName
void saveMessage(char* str, char* fileName) {
  // make a string for assembling the data to log:
  char *dataString = (char*)malloc(50 * sizeof(char));
  char *dateTime = date();
  sprintf(dataString, "%s\t%s", str, dateTime);//16 + 11 + 8
  Serial.println(dataString);




  // open the file. note that only one file can be open at a time,
  // so you have to close this one before opening another.
  File dataFile = SD.open("dl.txt", FILE_WRITE);
  // if the file is available, write to it:
  if (dataFile) {
    dataFile.println(dataString);
    dataFile.close();
  }
  // if the file isn't open, pop up an error:
  // some times an error will not come up even if there actually is an error
  else {
    Serial.println("error");
  }
  
  free(dataString);
  free(dateTime);
}


//return the current date and time
char* date() {
  DateTime t_now = rtc.now();//retrieves the current date and time from the real time clock
  char* date = (char*)malloc(30 * sizeof(char));
  //compile the date and time into a usable string
  sprintf(date, "%d/%d/%d %d:%d:%d\n", t_now.month(), t_now.day(), t_now.year(), t_now.hour(), t_now.minute(), t_now.second());
  return date;
}







