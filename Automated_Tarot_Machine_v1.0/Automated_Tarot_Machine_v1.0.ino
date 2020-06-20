#include "Adafruit_Thermal.h"
#include <SD.h>
#include <SPI.h>
#include "SoftwareSerial.h"
#include <Adafruit_NeoPixel.h>
#include <Entropy.h>

//Declare pin functions
#define stp 2
#define dir 3
#define MS1 4
#define MS2 5
#define EN  6 // stp, dir, MS1, MS2, EN are all for the Easy Driver
#define LED_PIN 7 // Optional, be sure to comment this outt if you aren't going to use LEDS
#define LED_COUNT 24
#define HOME_SENSOR 8 //This is the pin for the hall effect sensor
#define TX_PIN 9 // Arduino transmit YELLOW WIRE labeled RX on printer
#define RX_PIN 10 // Arduino receive GREEN WIRE labeled TX on printer
#define BUTTON_PIN 11 // Only relavent if you wish to operate the machine without the computer attached
#define SD_Pin 53 // Pin for the SD card module


Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_GRBW + NEO_KHZ800); // declare LEDS
SoftwareSerial mySerial(RX_PIN, TX_PIN); // Declare SoftwareSerial obj first
Adafruit_Thermal printer(&mySerial, 48);     // Pass addr to printer constructor
// Then see setup() function regarding serial & printer begin() calls.

//Declare variables for functions
char user_input;
int x;

inline void initSD() {
  pinMode(SD_Pin, OUTPUT);
  if (!SD.begin(SD_Pin)) {
    Serial.println("SD Error");
  } else {
    Serial.println("SD Ok");
  }
}

//****************************************************************************
//****************************************************************************

boolean lastButtonState = HIGH;
unsigned long currentButtonTime = 0, lastButtonTime = 0, ButtonCheckTime = 20;

//****************************************************************************
//****************************************************************************

void setup() {
  initSD(); //Initialize the SD card
  Entropy.initialize(); //Initialize the Entropy library
  pinMode(stp, OUTPUT);
  pinMode(dir, OUTPUT);
  pinMode(MS1, OUTPUT);
  pinMode(MS2, OUTPUT);
  pinMode(EN, OUTPUT); //Set the pinmodes for the easydriver
  pinMode(HOME_SENSOR, INPUT); // Set the pinmode for the hall effect sensor
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  Serial.begin(9600); //Open Serial connection for debugging
  Serial.println("Begin motor control");
  strip.begin();           // INITIALIZE NeoPixel strip object (REQUIRED)
  strip.show();            // Turn OFF all pixels ASAP
  strip.setBrightness(50); // Set BRIGHTNESS to about 1/5 (max = 255)
  mySerial.begin(19200);  // Initialize SoftwareSerial
  printer.begin();        // Init printer (same regardless of serial type)

  FindHome(); // Go to the "Top of the deck" at startup
}

//****************************************************************************
//****************************************************************************

void loop(){
  
  currentButtonTime = millis();
  digitalWrite(EN, LOW); //Pull enable pin low to allow motor control

  if ( currentButtonTime - lastButtonTime > ButtonCheckTime ) {
    
    boolean buttonState = digitalRead(BUTTON_PIN);
    
    if (buttonState == LOW && lastButtonState == HIGH) {
      
      PickACard();
      CardOfTheDay();
      colorWipe(strip.Color(0, 0, 0, 0), 0); // OFF
      FindHome();
      
  } 
  lastButtonState = buttonState;
 }
  
  while(Serial.available()){
      user_input = Serial.read(); //Read user input and trigger appropriate function
      digitalWrite(EN, LOW); //Pull enable pin low to allow motor control
      if (user_input == '1'){
          Serial.println();
          Serial.print("Your card of the day is... ");
          PickACard();
          CardOfTheDay();
          colorWipe(strip.Color(0, 0, 0, 0), 0); // OFF
          FindHome();
          }
      else if (user_input == '2'){
          Serial.println();
          Serial.print("Your past was... ");
          PickACard();
          Past();
          colorWipe(strip.Color(0, 0, 0, 0), 0); // OFF
          Reset();
          Serial.print("Your present is... ");
          PickACard();
          Present();
          colorWipe(strip.Color(0, 0, 0, 0), 0); // OFF
          Reset();
          Serial.print("Your future will be... ");
          PickACard();
          Future();
          colorWipe(strip.Color(0, 0, 0, 0), 0); // OFF
          FindHome();
          }
      else if (user_input == '3'){
          Serial.println();
          Serial.print("Your relationship card is... ");
          PickACard();
          Relationship();
          colorWipe(strip.Color(0, 0, 0, 0), 0); // OFF
          FindHome();
          }
      else if (user_input == '4'){
          Serial.println();
          Serial.print("Your vocational card is... ");
          PickACard();
          Vocation();
          colorWipe(strip.Color(0, 0, 0, 0), 0); // OFF
          FindHome();
          }          
         }
        }

//****************************************************************************
//****************************************************************************

void FindHome(){
  digitalWrite(dir, LOW); //Pull direction pin low to move "forward"
  digitalWrite(MS1, HIGH); 
  digitalWrite(MS2, HIGH);  //Pull MS1, and MS2 high to set logic to 1/8th microstep resolution
  Serial.println("Searching for home...");
    for(x= 0; x< 500 ; x++)  //Loop forward enough times to stop a false home
  {
    digitalWrite(stp,HIGH); //Trigger one step forward
    delayMicroseconds(1275);
    digitalWrite(stp,LOW); //Pull step pin low so it can be triggered again
    delayMicroseconds(1275);
  }
  while(digitalRead(HOME_SENSOR)){
    digitalWrite(stp,HIGH); //Trigger one step forward
    delayMicroseconds(1275);
    digitalWrite(stp,LOW); //Pull step pin low so it can be triggered again
    delayMicroseconds(1275);
  }
  Serial.println("Home Found");
  Serial.println();
  Serial.println("What type of reading would you like?");
  Serial.println();
  Serial.println("1: Card of the day");
  Serial.println("2: Past, Present, Future");      
  Serial.println("3: Love and relationships");
  Serial.println("4: Career / Vocation");
 }

//****************************************************************************

void Reset(){
  digitalWrite(dir, LOW); //Pull direction pin low to move "forward"
  digitalWrite(MS1, HIGH); //Pull MS1, and MS2 high to set logic to 1/8th microstep resolution
  digitalWrite(MS2, HIGH);
    for(x= 0; x< 500 ; x++)  //Loop forward enough times to stop a false home
  {
    digitalWrite(stp,HIGH); //Trigger one step forward
    delayMicroseconds(1275);
    digitalWrite(stp,LOW); //Pull step pin low so it can be triggered again
    delayMicroseconds(1275);
  }
  while(digitalRead(HOME_SENSOR)){
    digitalWrite(stp,HIGH); //Trigger one step forward
    delayMicroseconds(1275);
    digitalWrite(stp,LOW); //Pull step pin low so it can be triggered again
    delayMicroseconds(1275);
  }
} 

//****************************************************************************

void CardOfTheDay(){
  printer.feed(1);
  printer.boldOn();
  printer.justify('C');
  printer.setSize('L');
  printer.upsideDownOn();
  printer.println(F("CARD OF THE DAY"));
  printer.feed(4);
  printer.upsideDownOff();
  printer.sleep();      // Tell printer to sleep
  delay(3000L);         // Sleep for 3 seconds
  printer.wake();       // MUST wake() before printing again, even if reset
  printer.setDefault(); // Restore printer to defaults
}

//****************************************************************************

void Past(){
  printer.feed(1);
  printer.boldOn();
  printer.justify('C');
  printer.setSize('L');
  printer.upsideDownOn();
  printer.println(F("PAST"));
  printer.feed(4);
  printer.upsideDownOff();
  printer.sleep();      // Tell printer to sleep
  delay(3000L);         // Sleep for 3 seconds
  printer.wake();       // MUST wake() before printing again, even if reset
  printer.setDefault(); // Restore printer to defaults
}

//****************************************************************************

void Present(){
  printer.feed(1);
  printer.boldOn();
  printer.justify('C');
  printer.setSize('L');
  printer.upsideDownOn();
  printer.println(F("PRESENT"));
  printer.feed(4);
  printer.upsideDownOff();
  printer.sleep();      // Tell printer to sleep
  delay(3000L);         // Sleep for 3 seconds
  printer.wake();       // MUST wake() before printing again, even if reset
  printer.setDefault(); // Restore printer to defaults
}

//****************************************************************************

void Future(){
  printer.feed(1);
  printer.boldOn();
  printer.justify('C');
  printer.setSize('L');
  printer.upsideDownOn();
  printer.println(F("FUTURE"));
  printer.feed(4);
  printer.upsideDownOff();
  printer.sleep();      // Tell printer to sleep
  delay(3000L);         // Sleep for 3 seconds
  printer.wake();       // MUST wake() before printing again, even if reset
  printer.setDefault(); // Restore printer to defaults
}

//****************************************************************************

void Relationship(){
  printer.feed(1);
  printer.boldOn();
  printer.justify('C');
  printer.setSize('M');
  printer.upsideDownOn();
  printer.println(F("RELATIONSHIP CARD"));
  printer.feed(4);
  printer.upsideDownOff();
  printer.sleep();      // Tell printer to sleep
  delay(3000L);         // Sleep for 3 seconds
  printer.wake();       // MUST wake() before printing again, even if reset
  printer.setDefault(); // Restore printer to defaults
}

//****************************************************************************

void Vocation(){
  printer.feed(1);
  printer.boldOn();
  printer.justify('C');
  printer.setSize('L');
  printer.upsideDownOn();
  printer.println(F("VOCATION CARD"));
  printer.feed(4);
  printer.upsideDownOff();
  printer.sleep();      // Tell printer to sleep
  delay(3000L);         // Sleep for 3 seconds
  printer.wake();       // MUST wake() before printing again, even if reset
  printer.setDefault(); // Restore printer to defaults
}

//****************************************************************************

void PickACard()
{
  
  int val = 
  
  Entropy.random(40, 1581); // Full range of cards excluding "blanks" #0 & #20
  int rem;
 
  rem = val % 20 ;
  if (rem < 10)
    val -= rem;
  else
    val += 20 - rem; //ensure value is a multiple of 20
 
  digitalWrite(dir, LOW); //Pull direction pin low to move "forward"
  digitalWrite(MS1, HIGH); //Pull MS1, and MS2 high to set logic to 1/8th microstep resolution
  digitalWrite(MS2, HIGH);
  for(x= 0; x< val + 1600 ; x++)  // Move spindle one full rotation before going to the choosen card
  {
    digitalWrite(stp,HIGH); 
    delayMicroseconds(1275); // delay dictates how fast the spindle rotates.
    digitalWrite(stp,LOW); 
    delayMicroseconds(1275);
  }
  
//****************************************************************************
//****************************************************************************

// LED CODE

    if (val > 20 && val < 470) { // Major Arcana
      
      Yellow(1);
}
    else if (val > 471 && val < 750) { // Wands element: Fire
      
      Red(1);
}
    else if (val > 751 && val < 1030) { // Cups element: Water
      
      Blue(1);
}
    else if (val > 1031 && val < 1310) { // Swords element: Air
      
      White(1);
}
    else if (val > 1311 && val < 1600) { // Pentacles element: Earth
      
      Green(1);
}
//****************************************************************************
//****************************************************************************

// PRINTER CODE

  if (val == 40) // The Fool
  { 
  int readingType = Entropy.random(0, 31);
  Serial.print("The Fool");
  Serial.println("");
    if (readingType <= 9)
    {
      Serial.print("Reading type is: Upright");
      Serial.println("");
      File data = SD.open("0_p", FILE_READ);
      printer.printBitmap(384, 660, dynamic_cast<Stream*>(&data));
      data.close();
    }
    else if (readingType >= 10 && readingType <= 19)
    {
      Serial.print("Reading type is: Open");
      Serial.println("");
      File data = SD.open("0_o", FILE_READ);
      printer.printBitmap(384, 660, dynamic_cast<Stream*>(&data));
      data.close();
    }
    else if (readingType >= 20)
    {
      Serial.print("Reading type is: Reversed");
      Serial.println("");
      File data = SD.open("0_n", FILE_READ);
      printer.printBitmap(384, 660, dynamic_cast<Stream*>(&data));
      data.close();
    }
  }

  else if (val == 60) // The Magician
  {
//  int readingType = 5;
//  int readingType = 15;
//  int readingType = 25;  
  int readingType = Entropy.random(0, 31); 
  Serial.print("The Magician");
  Serial.println("");
  Serial.println("");
    if (readingType <= 9)
    {
      Serial.print("Reading type is: Upright");
      Serial.println("");
      File data = SD.open("1_p", FILE_READ);
      printer.printBitmap(384, 661, dynamic_cast<Stream*>(&data));
      data.close();
    }
    else if (readingType >= 10 && readingType <= 19)
    {
      Serial.print("Reading type is: Open");
      Serial.println("");
      File data = SD.open("1_o", FILE_READ);
      printer.printBitmap(384, 661, dynamic_cast<Stream*>(&data));
      data.close();
    }
    else if (readingType >= 20)
    {
      Serial.print("Reading type is: Reversed");
      Serial.println("");
      File data = SD.open("1_n", FILE_READ);
      printer.printBitmap(384, 661, dynamic_cast<Stream*>(&data));
      data.close();
    }
  }
  
  else if (val == 80) // The High Priestess
  {
//  int readingType = 5;
//  int readingType = 15;
//  int readingType = 25;  
  int readingType = Entropy.random(0, 31);  
  Serial.print("The High Priestess");
  Serial.println("");
  Serial.println("");
    if (readingType <= 9)
    {
      Serial.print("Reading type is: Upright");
      Serial.println("");
      File data = SD.open("2_p", FILE_READ);
      printer.printBitmap(384, 661, dynamic_cast<Stream*>(&data));
      data.close();
    }
    else if (readingType >= 10 && readingType <= 19)
    {
      Serial.print("Reading type is: Open");
      Serial.println("");
      File data = SD.open("2_o", FILE_READ);
      printer.printBitmap(384, 661, dynamic_cast<Stream*>(&data));
      data.close();
    }
    else if (readingType >= 20)
    {
      Serial.print("Reading type is: Reversed");
      Serial.println("");
      File data = SD.open("2_n", FILE_READ);
      printer.printBitmap(384, 661, dynamic_cast<Stream*>(&data));
      data.close();
    }
  }
  
  else if (val == 100) // The Empress
  {
//  int readingType = 5;
//  int readingType = 15;
//  int readingType = 25;  
  int readingType = Entropy.random(0, 31);  
  Serial.print("The Empress");
  Serial.println("");
  Serial.println("");
    if (readingType <= 9)
    {
      Serial.print("Reading type is: Upright");
      Serial.println("");
      File data = SD.open("3_p", FILE_READ);
      printer.printBitmap(384, 661, dynamic_cast<Stream*>(&data));
      data.close();
    }
    else if (readingType >= 10 && readingType <= 19)
    {
      Serial.print("Reading type is: Open");
      Serial.println("");
      File data = SD.open("3_o", FILE_READ);
      printer.printBitmap(384, 661, dynamic_cast<Stream*>(&data));
      data.close();
    }
    else if (readingType >= 20)
    {
      Serial.print("Reading type is: Reversed");
      Serial.println("");
      File data = SD.open("3_n", FILE_READ);
      printer.printBitmap(384, 661, dynamic_cast<Stream*>(&data));
      data.close();
    }
  }

  else if (val == 120) // The Emperor
  {
//  int readingType = 5;
//  int readingType = 15;
//  int readingType = 25;  
  int readingType = Entropy.random(0, 31);   
  Serial.print("The Emperor");
  Serial.println("");
  Serial.println("");
    if (readingType <= 9)
    {
      Serial.print("Reading type is: Upright");
      Serial.println("");
      File data = SD.open("4_p", FILE_READ);
      printer.printBitmap(384, 661, dynamic_cast<Stream*>(&data));
      data.close();
    }
    else if (readingType >= 10 && readingType <= 19)
    {
      Serial.print("Reading type is: Open");
      Serial.println("");
      File data = SD.open("4_o", FILE_READ);
      printer.printBitmap(384, 661, dynamic_cast<Stream*>(&data));
      data.close();
    }
    else if (readingType >= 20)
    {
      Serial.print("Reading type is: Reversed");
      Serial.println("");
      File data = SD.open("4_n", FILE_READ);
      printer.printBitmap(384, 661, dynamic_cast<Stream*>(&data));
      data.close();
    }
  }

  else if (val == 140) // The Hierophant
  {
//  int readingType = 5;
//  int readingType = 15;
//  int readingType = 25;  
  int readingType = Entropy.random(0, 31);  
  Serial.print("The Hierophant");
  Serial.println("");
  Serial.println("");
    if (readingType <= 9)
    {
      Serial.print("Reading type is: Upright");
      Serial.println("");
      File data = SD.open("5_p", FILE_READ);
      printer.printBitmap(384, 661, dynamic_cast<Stream*>(&data));
      data.close();
    }
    else if (readingType >= 10 && readingType <= 19)
    {
      Serial.print("Reading type is: Open");
      Serial.println("");
      File data = SD.open("5_o", FILE_READ);
      printer.printBitmap(384, 661, dynamic_cast<Stream*>(&data));
      data.close();
    }
    else if (readingType >= 20)
    {
      Serial.print("Reading type is: Reversed");
      Serial.println("");
      File data = SD.open("5_n", FILE_READ);
      printer.printBitmap(384, 661, dynamic_cast<Stream*>(&data));
      data.close();
    }
  }

  else if (val == 160) // The Lovers
  {
//   int readingType = 5;
//  int readingType = 15;
//  int readingType = 25;  
  int readingType = Entropy.random(0, 31);   
  Serial.print("The Lovers");
  Serial.println("");
  Serial.println("");
    if (readingType <= 9)
    {
      Serial.print("Reading type is: Upright");
      Serial.println("");
      File data = SD.open("6_p", FILE_READ);
      printer.printBitmap(384, 661, dynamic_cast<Stream*>(&data));
      data.close();
    }
    else if (readingType >= 10 && readingType <= 19)
    {
      Serial.print("Reading type is: Open");
      Serial.println("");
      File data = SD.open("6_o", FILE_READ);
      printer.printBitmap(384, 661, dynamic_cast<Stream*>(&data));
      data.close();
    }
    else if (readingType >= 20)
    {
      Serial.print("Reading type is: Reversed");
      Serial.println("");
      File data = SD.open("6_n", FILE_READ);
      printer.printBitmap(384, 661, dynamic_cast<Stream*>(&data));
      data.close();
    }
  }

  else if (val == 180) // The Chariot
  {
//  int readingType = 5;
//  int readingType = 15;
//  int readingType = 25;  
  int readingType = Entropy.random(0, 31);   
  Serial.print("The Chariot");
  Serial.println("");
  Serial.println("");
    if (readingType <= 9)
    {
      Serial.print("Reading type is: Upright");
      Serial.println("");
      File data = SD.open("7_p", FILE_READ);
      printer.printBitmap(384, 661, dynamic_cast<Stream*>(&data));
      data.close();
    }
    else if (readingType >= 10 && readingType <= 19)
    {
      Serial.print("Reading type is: Open");
      Serial.println("");
      File data = SD.open("7_o", FILE_READ);
      printer.printBitmap(384, 661, dynamic_cast<Stream*>(&data));
      data.close();
    }
    else if (readingType >= 20)
    {
      Serial.print("Reading type is: Reversed");
      Serial.println("");
      File data = SD.open("7_n", FILE_READ);
      printer.printBitmap(384, 661, dynamic_cast<Stream*>(&data));
      data.close();
    }
  }

  else if (val == 200) // Strength
  {
//  int readingType = 5;
//  int readingType = 15;
//  int readingType = 25;  
  int readingType = Entropy.random(0, 31);
  Serial.println("Strength");
  Serial.println("");
  Serial.println("");
    if (readingType <= 9)
    {
      Serial.print("Reading type is: Upright");
      Serial.println("");
      File data = SD.open("8_p", FILE_READ);
      printer.printBitmap(384, 661, dynamic_cast<Stream*>(&data));
      data.close();
    }
    else if (readingType >= 10 && readingType <= 19)
    {
      Serial.print("Reading type is: Open");
      Serial.println("");
      File data = SD.open("8_o", FILE_READ);
      printer.printBitmap(384, 661, dynamic_cast<Stream*>(&data));
      data.close();
    }
    else if (readingType >= 20)
    {
      Serial.print("Reading type is: Reversed");
      Serial.println("");
      File data = SD.open("8_n", FILE_READ);
      printer.printBitmap(384, 661, dynamic_cast<Stream*>(&data));
      data.close();
    }
  }

  else if (val == 220) // The Hermit
  {
//  int readingType = 5;
//  int readingType = 15;
//  int readingType = 25;  
  int readingType = Entropy.random(0, 31);  
  Serial.print("The Hermit");
  Serial.println("");
  Serial.println("");
    if (readingType <= 9)
    {
      Serial.print("Reading type is: Upright");
      Serial.println("");
      File data = SD.open("9_p", FILE_READ);
      printer.printBitmap(384, 661, dynamic_cast<Stream*>(&data));
      data.close();
    }
    else if (readingType >= 10 && readingType <= 19)
    {
      Serial.print("Reading type is: Open");
      Serial.println("");
      File data = SD.open("9_o", FILE_READ);
      printer.printBitmap(384, 661, dynamic_cast<Stream*>(&data));
      data.close();
    }
    else if (readingType >= 20)
    {
      Serial.print("Reading type is: Reversed");
      Serial.println("");
      File data = SD.open("9_n", FILE_READ);
      printer.printBitmap(384, 661, dynamic_cast<Stream*>(&data));
      data.close();
    }
  }

  else if (val == 240) // Wheel of Fortune
  {
//  int readingType = 5;
//  int readingType = 15;
//  int readingType = 25;  
  int readingType = Entropy.random(0, 31);  
  Serial.print("Wheel of Fortune");
  Serial.println("");
  Serial.println("");
    if (readingType <= 9)
    {
      Serial.print("Reading type is: Upright");
      Serial.println("");
      File data = SD.open("10_p", FILE_READ);
      printer.printBitmap(384, 661, dynamic_cast<Stream*>(&data));
      data.close();
    }
    else if (readingType >= 10 && readingType <= 19)
    {
      Serial.print("Reading type is: Open");
      Serial.println("");
      File data = SD.open("10_o", FILE_READ);
      printer.printBitmap(384, 661, dynamic_cast<Stream*>(&data));
      data.close();
    }
    else if (readingType >= 20)
    {
      Serial.print("Reading type is: Reversed");
      Serial.println("");
      File data = SD.open("10_n", FILE_READ);
      printer.printBitmap(384, 661, dynamic_cast<Stream*>(&data));
      data.close();
    }
  }

  else if (val == 260) // Justice
  {
//  int readingType = 5;
//  int readingType = 15;
//  int readingType = 25;  
  int readingType = Entropy.random(0, 31);  
  Serial.print("Justice");
  Serial.println("");
  Serial.println("");
    if (readingType <= 9)
    {
      Serial.print("Reading type is: Upright");
      Serial.println("");
      File data = SD.open("11_p", FILE_READ);
      printer.printBitmap(384, 661, dynamic_cast<Stream*>(&data));
      data.close();
    }
    else if (readingType >= 10 && readingType <= 19)
    {
      Serial.print("Reading type is: Open");
      Serial.println("");
      File data = SD.open("11_o", FILE_READ);
      printer.printBitmap(384, 661, dynamic_cast<Stream*>(&data));
      data.close();
    }
    else if (readingType >= 20)
    {
      Serial.print("Reading type is: Reversed");
      Serial.println("");
      File data = SD.open("11_n", FILE_READ);
      printer.printBitmap(384, 661, dynamic_cast<Stream*>(&data));
      data.close();
    }
  }

  else if (val == 280) // The Hanged Man
  {
//  int readingType = 5;
//  int readingType = 15;
//  int readingType = 25;  
  int readingType = Entropy.random(0, 31);  
  Serial.print("The Hanged Man");
  Serial.println("");
  Serial.println("");
    if (readingType <= 9)
    {
      Serial.print("Reading type is: Upright");
      Serial.println("");
      File data = SD.open("12_p", FILE_READ);
      printer.printBitmap(384, 660, dynamic_cast<Stream*>(&data));
      data.close();
    }
    else if (readingType >= 10 && readingType <= 19)
    {
      Serial.print("Reading type is: Open");
      Serial.println("");
      File data = SD.open("12_o", FILE_READ);
      printer.printBitmap(384, 660, dynamic_cast<Stream*>(&data));
      data.close();
    }
    else if (readingType >= 20)
    {
      Serial.print("Reading type is: Reversed");
      Serial.println("");
      File data = SD.open("12_n", FILE_READ);
      printer.printBitmap(384, 660, dynamic_cast<Stream*>(&data));
      data.close();
    }
  }

  else if (val == 300) // Death
  {
//  int readingType = 5;
//  int readingType = 15;
//  int readingType = 25;  
  int readingType = Entropy.random(0, 31);  
  Serial.print("Death");
  Serial.println("");
  Serial.println("");
    if (readingType <= 9)
    {
      Serial.print("Reading type is: Upright");
      Serial.println("");
      File data = SD.open("13_p", FILE_READ);
      printer.printBitmap(384, 661, dynamic_cast<Stream*>(&data));
      data.close();
    }
    else if (readingType >= 10 && readingType <= 19)
    {
      Serial.print("Reading type is: Open");
      Serial.println("");
      File data = SD.open("13_o", FILE_READ);
      printer.printBitmap(384, 661, dynamic_cast<Stream*>(&data));
      data.close();
    }
    else if (readingType >= 20)
    {
      Serial.print("Reading type is: Reversed");
      Serial.println("");
      File data = SD.open("13_n", FILE_READ);
      printer.printBitmap(384, 661, dynamic_cast<Stream*>(&data));
      data.close();
    }
  }

  else if (val == 320) // Temperance
  {
//  int readingType = 5;
//  int readingType = 15;
//  int readingType = 25;  
  int readingType = Entropy.random(0, 31);  
  Serial.print("Temperance");
  Serial.println("");
    if (readingType <= 9)
    {
      Serial.print("Reading type is: Upright");
      Serial.println("");
      File data = SD.open("14_p", FILE_READ);
      printer.printBitmap(384, 661, dynamic_cast<Stream*>(&data));
      data.close();
    }
    else if (readingType >= 10 && readingType <= 19)
    {
      Serial.print("Reading type is: Open");
      Serial.println("");
      File data = SD.open("14_o", FILE_READ);
      printer.printBitmap(384, 661, dynamic_cast<Stream*>(&data));
      data.close();
    }
    else if (readingType >= 20)
    {
      Serial.print("Reading type is: Reversed");
      Serial.println("");
      File data = SD.open("14_n", FILE_READ);
      printer.printBitmap(384, 661, dynamic_cast<Stream*>(&data));
      data.close();
    }
  }

  else if (val == 340) // The Devil
  {
//  int readingType = 5;
//  int readingType = 15;
//  int readingType = 25;  
  int readingType = Entropy.random(0, 31);  
  Serial.print("The Devil");
  Serial.println("");
    if (readingType <= 9)
    {
      Serial.print("Reading type is: Upright");
      Serial.println("");
      File data = SD.open("15_p", FILE_READ);
      printer.printBitmap(384, 661, dynamic_cast<Stream*>(&data));
      data.close();
    }
    else if (readingType >= 10 && readingType <= 19)
    {
      Serial.print("Reading type is: Open");
      Serial.println("");
      File data = SD.open("15_o", FILE_READ);
      printer.printBitmap(384, 661, dynamic_cast<Stream*>(&data));
      data.close();
    }
    else if (readingType >= 20)
    {
      Serial.print("Reading type is: Reversed");
      Serial.println("");
      File data = SD.open("15_n", FILE_READ);
      printer.printBitmap(384, 661, dynamic_cast<Stream*>(&data));
      data.close();
    }
  }  

  else if (val == 360) // The Tower
  {
//  int readingType = 5;
//  int readingType = 15;
//  int readingType = 25;  
  int readingType = Entropy.random(0, 31);  
  Serial.print("The Tower");
  Serial.println("");
    if (readingType <= 9)
    {
      Serial.print("Reading type is: Upright");
      Serial.println("");
      File data = SD.open("16_p", FILE_READ);
      printer.printBitmap(384, 661, dynamic_cast<Stream*>(&data));
      data.close();
    }
    else if (readingType >= 10 && readingType <= 19)
    {
      Serial.print("Reading type is: Open");
      Serial.println("");
      File data = SD.open("16_o", FILE_READ);
      printer.printBitmap(384, 661, dynamic_cast<Stream*>(&data));
      data.close();
    }
    else if (readingType >= 20)
    {
      Serial.print("Reading type is: Reversed");
      Serial.println("");
      File data = SD.open("16_n", FILE_READ);
      printer.printBitmap(384, 661, dynamic_cast<Stream*>(&data));
      data.close();
    }
  }

  else if (val == 380) //The Star
  {
//  int readingType = 5;
//  int readingType = 15;
//  int readingType = 25;  
  int readingType = Entropy.random(0, 31);  
  Serial.print("The Star");
  Serial.println("");
  Serial.println("");
    if (readingType <= 9)
    {
      Serial.print("Reading type is: Upright");
      Serial.println("");
      File data = SD.open("17_p", FILE_READ);
      printer.printBitmap(384, 661, dynamic_cast<Stream*>(&data));
      data.close();
    }
    else if (readingType >= 10 && readingType <= 19)
    {
      Serial.print("Reading type is: Open");
      Serial.println("");
      File data = SD.open("17_o", FILE_READ);
      printer.printBitmap(384, 661, dynamic_cast<Stream*>(&data));
      data.close();
    }
    else if (readingType >= 20)
    {
      Serial.print("Reading type is: Reversed");
      Serial.println("");
      File data = SD.open("17_n", FILE_READ);
      printer.printBitmap(384, 661, dynamic_cast<Stream*>(&data));
      data.close();
    }
  }

    else if (val == 400) // The Moon
  {
//  int readingType = 5;
//  int readingType = 15;
//  int readingType = 25;  
  int readingType = Entropy.random(0, 31); 
  Serial.print("The Moon");
  Serial.println("");
  Serial.println("");
    if (readingType <= 9)
    {
      Serial.print("Reading type is: Upright");
      Serial.println("");
      File data = SD.open("18_p", FILE_READ);
      printer.printBitmap(384, 661, dynamic_cast<Stream*>(&data));
      data.close();
    }
    else if (readingType >= 10 && readingType <= 19)
    {
      Serial.print("Reading type is: Open");
      Serial.println("");
      File data = SD.open("18_o", FILE_READ);
      printer.printBitmap(384, 661, dynamic_cast<Stream*>(&data));
      data.close();
    }
    else if (readingType >= 20)
    {
      Serial.print("Reading type is: Reversed");
      Serial.println("");
      File data = SD.open("18_n", FILE_READ);
      printer.printBitmap(384, 661, dynamic_cast<Stream*>(&data));
      data.close();
    }
  } 

    else if (val == 420) // The Sun
  {
//  int readingType = 5;
//  int readingType = 15;
//  int readingType = 25;  
  int readingType = Entropy.random(0, 31);  
  Serial.print("The Sun");
  Serial.println("");
  Serial.println("");
    if (readingType <= 9)
    {
      Serial.print("Reading type is: Upright");
      Serial.println("");
      File data = SD.open("19_p", FILE_READ);
      printer.printBitmap(384, 661, dynamic_cast<Stream*>(&data));
      data.close();
    }
    else if (readingType >= 10 && readingType <= 19)
    {
      Serial.print("Reading type is: Open");
      Serial.println("");
      File data = SD.open("19_o", FILE_READ);
      printer.printBitmap(384, 661, dynamic_cast<Stream*>(&data));
      data.close();
    }
    else if (readingType >= 20)
    {
      Serial.print("Reading type is: Reversed");
      Serial.println("");
      File data = SD.open("19_n", FILE_READ);
      printer.printBitmap(384, 661, dynamic_cast<Stream*>(&data));
      data.close();
    }
  }
  
    else if (val == 440) // Judgement
  {
//  int readingType = 5;
//  int readingType = 15;
//  int readingType = 25;  
  int readingType = Entropy.random(0, 31);  
  Serial.print("Judgement");
  Serial.println("");
  Serial.println("");
    if (readingType <= 9)
    {
      Serial.print("Reading type is: Upright");
      Serial.println("");
      File data = SD.open("20_p", FILE_READ);
      printer.printBitmap(384, 661, dynamic_cast<Stream*>(&data));
      data.close();
    }
    else if (readingType >= 10 && readingType <= 19)
    {
      Serial.print("Reading type is: Open");
      Serial.println("");
      File data = SD.open("20_o", FILE_READ);
      printer.printBitmap(384, 661, dynamic_cast<Stream*>(&data));
      data.close();
    }
    else if (readingType >= 20)
    {
      Serial.print("Reading type is: Reversed");
      Serial.println("");
      File data = SD.open("20_n", FILE_READ);
      printer.printBitmap(384, 661, dynamic_cast<Stream*>(&data));
      data.close();
    }
  } 

    else if (val == 460) // The World
  {
//  int readingType = 5;
//  int readingType = 15;
//  int readingType = 25;  
  int readingType = Entropy.random(0, 31);  
  Serial.print("The World");
  Serial.println("");
  Serial.println("");
    if (readingType <= 9)
    {
      Serial.print("Reading type is: Upright");
      Serial.println("");
      File data = SD.open("21_p", FILE_READ);
      printer.printBitmap(384, 661, dynamic_cast<Stream*>(&data));
      data.close();
    }
    else if (readingType >= 10 && readingType <= 19)
    {
      Serial.print("Reading type is: Open");
      Serial.println("");
      File data = SD.open("21_o", FILE_READ);
      printer.printBitmap(384, 661, dynamic_cast<Stream*>(&data));
      data.close();
    }
    else if (readingType >= 20)
    {
      Serial.print("Reading type is: Reversed");
      Serial.println("");
      File data = SD.open("21_n", FILE_READ);
      printer.printBitmap(384, 661, dynamic_cast<Stream*>(&data));
      data.close();
    }
  }
 
    else if (val == 480) // Ace of Wands
  {
//  int readingType = 5;
//  int readingType = 15;
//  int readingType = 25;  
  int readingType = Entropy.random(0, 31);  
  Serial.print("Ace of Wands");
  Serial.println("");
  Serial.println("");
    if (readingType <= 9)
    {
      Serial.print("Reading type is: Upright");
      Serial.println("");
      File data = SD.open("a_w_u", FILE_READ);
      printer.printBitmap(384, 661, dynamic_cast<Stream*>(&data));
      data.close();
    }
    else if (readingType >= 10 && readingType <= 19)
    {
      Serial.print("Reading type is: Open");
      Serial.println("");
      File data = SD.open("a_w_o", FILE_READ);
      printer.printBitmap(384, 661, dynamic_cast<Stream*>(&data));
      data.close();
    }
    else if (readingType >= 20)
    {
      Serial.print("Reading type is: Reversed");
      Serial.println("");
      File data = SD.open("a_w_r", FILE_READ);
      printer.printBitmap(384, 661, dynamic_cast<Stream*>(&data));
      data.close();
    }
  } 

    else if (val == 500) // Two of Wands
  {
//  int readingType = 5;
//  int readingType = 15;
//  int readingType = 25;  
  int readingType = Entropy.random(0, 31);  
  Serial.print("Two of Wands");
  Serial.println("");
  Serial.println("");
    if (readingType <= 9)
    {
      Serial.print("Reading type is: Upright");
      Serial.println("");
      File data = SD.open("2_w_u", FILE_READ);
      printer.printBitmap(384, 661, dynamic_cast<Stream*>(&data));
      data.close();
    }
    else if (readingType >= 10 && readingType <= 19)
    {
      Serial.print("Reading type is: Open");
      Serial.println("");
      File data = SD.open("2_w_o", FILE_READ);
      printer.printBitmap(384, 661, dynamic_cast<Stream*>(&data));
      data.close();
    }
    else if (readingType >= 20)
    {
      Serial.print("Reading type is: Reversed");
      Serial.println("");
      File data = SD.open("2_w_r", FILE_READ);
      printer.printBitmap(384, 661, dynamic_cast<Stream*>(&data));
      data.close();
    }
  printer.feed(1);
  printer.justify('C');
  printer.setSize('S');
  printer.upsideDownOn();
  printer.println(F("Two of Wands"));
  } 

    else if (val == 520) // Three of Wands
  {
//  int readingType = 5;
//  int readingType = 15;
//  int readingType = 25;  
  int readingType = Entropy.random(0, 31);  
  Serial.print("Three of Wands");
  Serial.println("");
  Serial.println("");
    if (readingType <= 9)
    {
      Serial.print("Reading type is: Upright");
      Serial.println("");
      File data = SD.open("3_w_u", FILE_READ);
      printer.printBitmap(384, 661, dynamic_cast<Stream*>(&data));
      data.close();
    }
    else if (readingType >= 10 && readingType <= 19)
    {
      Serial.print("Reading type is: Open");
      Serial.println("");
      File data = SD.open("3_w_o", FILE_READ);
      printer.printBitmap(384, 661, dynamic_cast<Stream*>(&data));
      data.close();
    }
    else if (readingType >= 20)
    {
      Serial.print("Reading type is: Reversed");
      Serial.println("");
      File data = SD.open("3_w_r", FILE_READ);
      printer.printBitmap(384, 661, dynamic_cast<Stream*>(&data));
      data.close();
    }
  printer.feed(1);
  printer.justify('C');
  printer.setSize('S');
  printer.upsideDownOn();
  printer.println(F("Three of Wands"));
  } 

    else if (val == 540) // Four of Wands
  {
//  int readingType = 5;
//  int readingType = 15;
//  int readingType = 25;  
  int readingType = Entropy.random(0, 31);  
  Serial.print("Four of Wands");
  Serial.println("");
  Serial.println("");
    if (readingType <= 9)
    {
      Serial.print("Reading type is: Upright");
      Serial.println("");
      File data = SD.open("4_w_u", FILE_READ);
      printer.printBitmap(384, 661, dynamic_cast<Stream*>(&data));
      data.close();
    }
    else if (readingType >= 10 && readingType <= 19)
    {
      Serial.print("Reading type is: Open");
      Serial.println("");
      File data = SD.open("4_w_o", FILE_READ);
      printer.printBitmap(384, 661, dynamic_cast<Stream*>(&data));
      data.close();
    }
    else if (readingType >= 20)
    {
      Serial.print("Reading type is: Reversed");
      Serial.println("");
      File data = SD.open("4_w_r", FILE_READ);
      printer.printBitmap(384, 661, dynamic_cast<Stream*>(&data));
      data.close();
    }
  printer.feed(1);
  printer.justify('C');
  printer.setSize('S');
  printer.upsideDownOn();
  printer.println(F("Four of Wands"));    
  } 

    else if (val == 560) // Five of Wands
  {
//  int readingType = 5;
//  int readingType = 15;
//  int readingType = 25;  
  int readingType = Entropy.random(0, 31);  
  Serial.print("Five of Wands");
  Serial.println("");
  Serial.println("");
    if (readingType <= 9)
    {
      Serial.print("Reading type is: Upright");
      Serial.println("");
      File data = SD.open("5_w_u", FILE_READ);
      printer.printBitmap(384, 661, dynamic_cast<Stream*>(&data));
      data.close();
    }
    else if (readingType >= 10 && readingType <= 19)
    {
      Serial.print("Reading type is: Open");
      Serial.println("");
      File data = SD.open("5_w_o", FILE_READ);
      printer.printBitmap(384, 661, dynamic_cast<Stream*>(&data));
      data.close();
    }
    else if (readingType >= 20)
    {
      Serial.print("Reading type is: Reversed");
      Serial.println("");
      File data = SD.open("5_w_r", FILE_READ);
      printer.printBitmap(384, 661, dynamic_cast<Stream*>(&data));
      data.close();
    }
  printer.feed(1);
  printer.justify('C');
  printer.setSize('S');
  printer.upsideDownOn();
  printer.println(F("Five of Wands"));    
  } 

    else if (val == 580) // Six of Wands
  {
//  int readingType = 5;
//  int readingType = 15;
//  int readingType = 25;  
  int readingType = Entropy.random(0, 31);  
  Serial.print("Six of Wands");
  Serial.println("");
  Serial.println("");
    if (readingType <= 9)
    {
      Serial.print("Reading type is: Upright");
      Serial.println("");
      File data = SD.open("6_w_u", FILE_READ);
      printer.printBitmap(384, 661, dynamic_cast<Stream*>(&data));
      data.close();
    }
    else if (readingType >= 10 && readingType <= 19)
    {
      Serial.print("Reading type is: Open");
      Serial.println("");
      File data = SD.open("6_w_o", FILE_READ);
      printer.printBitmap(384, 661, dynamic_cast<Stream*>(&data));
      data.close();
    }
    else if (readingType >= 20)
    {
      Serial.print("Reading type is: Reversed");
      Serial.println("");
      File data = SD.open("6_w_r", FILE_READ);
      printer.printBitmap(384, 661, dynamic_cast<Stream*>(&data));
      data.close();
    }
  printer.feed(1);
  printer.justify('C');
  printer.setSize('S');
  printer.upsideDownOn();
  printer.println(F("Six of Wands"));    
  } 

    else if (val == 600) // Seven of Wands
  {
//  int readingType = 5;
//  int readingType = 15;
//  int readingType = 25;  
  int readingType = Entropy.random(0, 31);  
  Serial.print("Seven of Wands");
  Serial.println("");
  Serial.println("");
    if (readingType <= 9)
    {
      Serial.print("Reading type is: Upright");
      Serial.println("");
      File data = SD.open("7_w_u", FILE_READ);
      printer.printBitmap(384, 661, dynamic_cast<Stream*>(&data));
      data.close();
    }
    else if (readingType >= 10 && readingType <= 19)
    {
      Serial.print("Reading type is: Open");
      Serial.println("");
      File data = SD.open("7_w_o", FILE_READ);
      printer.printBitmap(384, 661, dynamic_cast<Stream*>(&data));
      data.close();
    }
    else if (readingType >= 20)
    {
      Serial.print("Reading type is: Reversed");
      Serial.println("");
      File data = SD.open("7_w_r", FILE_READ);
      printer.printBitmap(384, 661, dynamic_cast<Stream*>(&data));
      data.close();
    }
  printer.feed(1);
  printer.justify('C');
  printer.setSize('S');
  printer.upsideDownOn();
  printer.println(F("Seven of Wands"));    
  } 

    else if (val == 620) // Eight of Wands
  {
//  int readingType = 5;
//  int readingType = 15;
//  int readingType = 25;  
  int readingType = Entropy.random(0, 31);  
  Serial.print("Eight of Wands");
  Serial.println("");
  Serial.println("");
    if (readingType <= 9)
    {
      Serial.print("Reading type is: Upright");
      Serial.println("");
      File data = SD.open("8_w_u", FILE_READ);
      printer.printBitmap(384, 661, dynamic_cast<Stream*>(&data));
      data.close();
    }
    else if (readingType >= 10 && readingType <= 19)
    {
      Serial.print("Reading type is: Open");
      Serial.println("");
      File data = SD.open("8_w_o", FILE_READ);
      printer.printBitmap(384, 661, dynamic_cast<Stream*>(&data));
      data.close();
    }
    else if (readingType >= 20)
    {
      Serial.print("Reading type is: Reversed");
      Serial.println("");
      File data = SD.open("8_w_r", FILE_READ);
      printer.printBitmap(384, 661, dynamic_cast<Stream*>(&data));
      data.close();
    }
  printer.feed(1);
  printer.justify('C');
  printer.setSize('S');
  printer.upsideDownOn();
  printer.println(F("Eight of Wands"));    
  } 

    else if (val == 640) // Nine of Wands
  {
//  int readingType = 5;
//  int readingType = 15;
//  int readingType = 25;  
  int readingType = Entropy.random(0, 31);  
  Serial.print("Nine of Wands");
  Serial.println("");
  Serial.println("");
    if (readingType <= 9)
    {
      Serial.print("Reading type is: Upright");
      Serial.println("");
      File data = SD.open("9_w_u", FILE_READ);
      printer.printBitmap(384, 661, dynamic_cast<Stream*>(&data));
      data.close();
    }
    else if (readingType >= 10 && readingType <= 19)
    {
      Serial.print("Reading type is: Open");
      Serial.println("");
      File data = SD.open("9_w_o", FILE_READ);
      printer.printBitmap(384, 661, dynamic_cast<Stream*>(&data));
      data.close();
    }
    else if (readingType >= 20)
    {
      Serial.print("Reading type is: Reversed");
      Serial.println("");
      File data = SD.open("9_w_r", FILE_READ);
      printer.printBitmap(384, 661, dynamic_cast<Stream*>(&data));
      data.close();
    }
  printer.feed(1);
  printer.justify('C');
  printer.setSize('S');
  printer.upsideDownOn();
  printer.println(F("Nine of Wands"));    
  } 

    else if (val == 660) // Ten of Wands
  {
//  int readingType = 5;
//  int readingType = 15;
//  int readingType = 25;  
  int readingType = Entropy.random(0, 31);  
  Serial.print("Ten of Wands");
  Serial.println("");
  Serial.println("");
    if (readingType <= 9)
    {
      Serial.print("Reading type is: Upright");
      Serial.println("");
      File data = SD.open("10_w_u", FILE_READ);
      printer.printBitmap(384, 661, dynamic_cast<Stream*>(&data));
      data.close();
    }
    else if (readingType >= 10 && readingType <= 19)
    {
      Serial.print("Reading type is: Open");
      Serial.println("");
      File data = SD.open("10_w_o", FILE_READ);
      printer.printBitmap(384, 661, dynamic_cast<Stream*>(&data));
      data.close();
    }
    else if (readingType >= 20)
    {
      Serial.print("Reading type is: Reversed");
      Serial.println("");
      File data = SD.open("10_w_r", FILE_READ);
      printer.printBitmap(384, 661, dynamic_cast<Stream*>(&data));
      data.close();
    }
  printer.feed(1);
  printer.justify('C');
  printer.setSize('S');
  printer.upsideDownOn();
  printer.println(F("Ten of Wands"));    
  }

    else if (val == 680) // Page of Wands
  {
//  int readingType = 5;
//  int readingType = 15;
//  int readingType = 25;  
  int readingType = Entropy.random(0, 31);  
  Serial.print("Page of Wands");
  Serial.println("");
  Serial.println("");
    if (readingType <= 9)
    {
      Serial.print("Reading type is: Upright");
      Serial.println("");
      File data = SD.open("p_w_u", FILE_READ);
      printer.printBitmap(384, 661, dynamic_cast<Stream*>(&data));
      data.close();
    }
    else if (readingType >= 10 && readingType <= 19)
    {
      Serial.print("Reading type is: Open");
      Serial.println("");
      File data = SD.open("p_w_o", FILE_READ);
      printer.printBitmap(384, 661, dynamic_cast<Stream*>(&data));
      data.close();
    }
    else if (readingType >= 20)
    {
      Serial.print("Reading type is: Reversed");
      Serial.println("");
      File data = SD.open("p_w_r", FILE_READ);
      printer.printBitmap(384, 661, dynamic_cast<Stream*>(&data));
      data.close();
    }
  }

    else if (val == 700) // Knight of Wands
  {
//  int readingType = 5;
//  int readingType = 15;
//  int readingType = 25;  
  int readingType = Entropy.random(0, 31);  
  Serial.print("Knight of Wands");
  Serial.println("");
  Serial.println("");
    if (readingType <= 9)
    {
      Serial.print("Reading type is: Upright");
      Serial.println("");
      File data = SD.open("kn_w_u", FILE_READ);
      printer.printBitmap(384, 661, dynamic_cast<Stream*>(&data));
      data.close();
    }
    else if (readingType >= 10 && readingType <= 19)
    {
      Serial.print("Reading type is: Open");
      Serial.println("");
      File data = SD.open("kn_w_o", FILE_READ);
      printer.printBitmap(384, 661, dynamic_cast<Stream*>(&data));
      data.close();
    }
    else if (readingType >= 20)
    {
      Serial.print("Reading type is: Reversed");
      Serial.println("");
      File data = SD.open("kn_w_r", FILE_READ);
      printer.printBitmap(384, 661, dynamic_cast<Stream*>(&data));
      data.close();
    }
  }

    else if (val == 720) // Queen of Wands
  {
//  int readingType = 5;
//  int readingType = 15;
//  int readingType = 25;  
  int readingType = Entropy.random(0, 31);  
  Serial.print("Queen of Wands");
  Serial.println("");
  Serial.println("");
    if (readingType <= 9)
    {
      Serial.print("Reading type is: Upright");
      Serial.println("");
      File data = SD.open("q_w_u", FILE_READ);
      printer.printBitmap(384, 661, dynamic_cast<Stream*>(&data));
      data.close();
    }
    else if (readingType >= 10 && readingType <= 19)
    {
      Serial.print("Reading type is: Open");
      Serial.println("");
      File data = SD.open("q_w_o", FILE_READ);
      printer.printBitmap(384, 661, dynamic_cast<Stream*>(&data));
      data.close();
    }
    else if (readingType >= 20)
    {
      Serial.print("Reading type is: Reversed");
      Serial.println("");
      File data = SD.open("q_w_r", FILE_READ);
      printer.printBitmap(384, 661, dynamic_cast<Stream*>(&data));
      data.close();
    }
  }

    else if (val == 740) // King of Wands
  {
//  int readingType = 5;
//  int readingType = 15;
//  int readingType = 25;  
  int readingType = Entropy.random(0, 31);  
  Serial.print("King of Wands");
  Serial.println("");
  Serial.println("");
    if (readingType <= 9)
    {
      Serial.print("Reading type is: Upright");
      Serial.println("");
      File data = SD.open("k_w_u", FILE_READ);
      printer.printBitmap(384, 661, dynamic_cast<Stream*>(&data));
      data.close();
    }
    else if (readingType >= 10 && readingType <= 19)
    {
      Serial.print("Reading type is: Open");
      Serial.println("");
      File data = SD.open("k_w_o", FILE_READ);
      printer.printBitmap(384, 661, dynamic_cast<Stream*>(&data));
      data.close();
    }
    else if (readingType >= 20)
    {
      Serial.print("Reading type is: Reversed");
      Serial.println("");
      File data = SD.open("k_w_r", FILE_READ);
      printer.printBitmap(384, 661, dynamic_cast<Stream*>(&data));
      data.close();
    }
  }

    else if (val == 760) // Ace of Cups
  {
//  int readingType = 5;
//  int readingType = 15;
//  int readingType = 25;  
  int readingType = Entropy.random(0, 31);  
  Serial.print("Ace of Cups");
  Serial.println("");
  Serial.println("");
    if (readingType <= 9)
    {
      Serial.print("Reading type is: Upright");
      Serial.println("");
      File data = SD.open("a_c_u", FILE_READ);
      printer.printBitmap(384, 661, dynamic_cast<Stream*>(&data));
      data.close();
    }
    else if (readingType >= 10 && readingType <= 19)
    {
      Serial.print("Reading type is: Open");
      Serial.println("");
      File data = SD.open("a_c_o", FILE_READ);
      printer.printBitmap(384, 661, dynamic_cast<Stream*>(&data));
      data.close();
    }
    else if (readingType >= 20)
    {
      Serial.print("Reading type is: Reversed");
      Serial.println("");
      File data = SD.open("a_c_r", FILE_READ);
      printer.printBitmap(384, 661, dynamic_cast<Stream*>(&data));
      data.close();
    }
  }

    else if (val == 780) // Two of Cups
  {
//  int readingType = 5;
//  int readingType = 15;
//  int readingType = 25;  
  int readingType = Entropy.random(0, 31);  
  Serial.print("Two of Cups");
  Serial.println("");
  Serial.println("");
    if (readingType <= 9)
    {
      Serial.print("Reading type is: Upright");
      Serial.println("");
      File data = SD.open("2_c_u", FILE_READ);
      printer.printBitmap(384, 661, dynamic_cast<Stream*>(&data));
      data.close();
    }
    else if (readingType >= 10 && readingType <= 19)
    {
      Serial.print("Reading type is: Open");
      Serial.println("");
      File data = SD.open("2_c_o", FILE_READ);
      printer.printBitmap(384, 661, dynamic_cast<Stream*>(&data));
      data.close();
    }
    else if (readingType >= 20)
    {
      Serial.print("Reading type is: Reversed");
      Serial.println("");
      File data = SD.open("2_c_r", FILE_READ);
      printer.printBitmap(384, 661, dynamic_cast<Stream*>(&data));
      data.close();
    }
  printer.feed(1);
  printer.justify('C');
  printer.setSize('S');
  printer.upsideDownOn();
  printer.println(F("Two of Cups"));    
  }

    else if (val == 800) // Three of Cups
  {
//  int readingType = 5;
//  int readingType = 15;
//  int readingType = 25;  
  int readingType = Entropy.random(0, 31);  
  Serial.print("Three of Cups");
  Serial.println("");
  Serial.println("");
    if (readingType <= 9)
    {
      Serial.print("Reading type is: Upright");
      Serial.println("");
      File data = SD.open("3_c_u", FILE_READ);
      printer.printBitmap(384, 661, dynamic_cast<Stream*>(&data));
      data.close();
    }
    else if (readingType >= 10 && readingType <= 19)
    {
      Serial.print("Reading type is: Open");
      Serial.println("");
      File data = SD.open("3_c_o", FILE_READ);
      printer.printBitmap(384, 661, dynamic_cast<Stream*>(&data));
      data.close();
    }
    else if (readingType >= 20)
    {
      Serial.print("Reading type is: Reversed");
      Serial.println("");
      File data = SD.open("3_c_r", FILE_READ);
      printer.printBitmap(384, 661, dynamic_cast<Stream*>(&data));
      data.close();
    }
  printer.feed(1);
  printer.justify('C');
  printer.setSize('S');
  printer.upsideDownOn();
  printer.println(F("Three of Cups"));    
  }

    else if (val == 820) // Four of Cups
  {
//  int readingType = 5;
//  int readingType = 15;
//  int readingType = 25;  
  int readingType = Entropy.random(0, 31);  
  Serial.print("Four of Cups");
  Serial.println("");
  Serial.println("");
    if (readingType <= 9)
    {
      Serial.print("Reading type is: Upright");
      Serial.println("");
      File data = SD.open("4_c_u", FILE_READ);
      printer.printBitmap(384, 661, dynamic_cast<Stream*>(&data));
      data.close();
    }
    else if (readingType >= 10 && readingType <= 19)
    {
      Serial.print("Reading type is: Open");
      Serial.println("");
      File data = SD.open("4_c_o", FILE_READ);
      printer.printBitmap(384, 661, dynamic_cast<Stream*>(&data));
      data.close();
    }
    else if (readingType >= 20)
    {
      Serial.print("Reading type is: Reversed");
      Serial.println("");
      File data = SD.open("4_c_r", FILE_READ);
      printer.printBitmap(384, 661, dynamic_cast<Stream*>(&data));
      data.close();
    }
  printer.feed(1);
  printer.justify('C');
  printer.setSize('S');
  printer.upsideDownOn();
  printer.println(F("Four of Cups"));    
  }

    else if (val == 840) // Five of Cups
  {
//  int readingType = 5;
//  int readingType = 15;
//  int readingType = 25;  
  int readingType = Entropy.random(0, 31);  
  Serial.print("Five of Cups");
  Serial.println("");
  Serial.println("");
    if (readingType <= 9)
    {
      Serial.print("Reading type is: Upright");
      Serial.println("");
      File data = SD.open("5_c_u", FILE_READ);
      printer.printBitmap(384, 661, dynamic_cast<Stream*>(&data));
      data.close();
    }
    else if (readingType >= 10 && readingType <= 19)
    {
      Serial.print("Reading type is: Open");
      Serial.println("");
      File data = SD.open("5_c_o", FILE_READ);
      printer.printBitmap(384, 661, dynamic_cast<Stream*>(&data));
      data.close();
    }
    else if (readingType >= 20)
    {
      Serial.print("Reading type is: Reversed");
      Serial.println("");
      File data = SD.open("5_c_r", FILE_READ);
      printer.printBitmap(384, 661, dynamic_cast<Stream*>(&data));
      data.close();
    }
  printer.feed(1);
  printer.justify('C');
  printer.setSize('S');
  printer.upsideDownOn();
  printer.println(F("Five of Cups"));    
  }

    else if (val == 860) // Six of Cups
  {
//  int readingType = 5;
//  int readingType = 15;
//  int readingType = 25;  
  int readingType = Entropy.random(0, 31);  
  Serial.print("Six of Cups");
  Serial.println("");
  Serial.println("");
    if (readingType <= 9)
    {
      Serial.print("Reading type is: Upright");
      Serial.println("");
      File data = SD.open("6_c_u", FILE_READ);
      printer.printBitmap(384, 661, dynamic_cast<Stream*>(&data));
      data.close();
    }
    else if (readingType >= 10 && readingType <= 19)
    {
      Serial.print("Reading type is: Open");
      Serial.println("");
      File data = SD.open("6_c_o", FILE_READ);
      printer.printBitmap(384, 661, dynamic_cast<Stream*>(&data));
      data.close();
    }
    else if (readingType >= 20)
    {
      Serial.print("Reading type is: Reversed");
      Serial.println("");
      File data = SD.open("6_c_r", FILE_READ);
      printer.printBitmap(384, 661, dynamic_cast<Stream*>(&data));
      data.close();
    }
  printer.feed(1);
  printer.justify('C');
  printer.setSize('S');
  printer.upsideDownOn();
  printer.println(F("Six of Cups"));    
  }

    else if (val == 880) // Seven of Cups
  {
//  int readingType = 5;
//  int readingType = 15;
//  int readingType = 25;  
  int readingType = Entropy.random(0, 31);  
  Serial.print("Seven of Cups");
  Serial.println("");
  Serial.println("");
    if (readingType <= 9)
    {
      Serial.print("Reading type is: Upright");
      Serial.println("");
      File data = SD.open("7_c_u", FILE_READ);
      printer.printBitmap(384, 661, dynamic_cast<Stream*>(&data));
      data.close();
    }
    else if (readingType >= 10 && readingType <= 19)
    {
      Serial.print("Reading type is: Open");
      Serial.println("");
      File data = SD.open("7_c_o", FILE_READ);
      printer.printBitmap(384, 661, dynamic_cast<Stream*>(&data));
      data.close();
    }
    else if (readingType >= 20)
    {
      Serial.print("Reading type is: Reversed");
      Serial.println("");
      File data = SD.open("7_c_r", FILE_READ);
      printer.printBitmap(384, 661, dynamic_cast<Stream*>(&data));
      data.close();
    }
  printer.feed(1);
  printer.justify('C');
  printer.setSize('S');
  printer.upsideDownOn();
  printer.println(F("Seven of Cups"));
  }

    else if (val == 900) // Eight of Cups
  {
//  int readingType = 5;
//  int readingType = 15;
//  int readingType = 25;  
  int readingType = Entropy.random(0, 31);  
  Serial.print("Eight of Cups");
  Serial.println("");
  Serial.println("");
    if (readingType <= 9)
    {
      Serial.print("Reading type is: Upright");
      Serial.println("");
      File data = SD.open("8_c_u", FILE_READ);
      printer.printBitmap(384, 661, dynamic_cast<Stream*>(&data));
      data.close();
    }
    else if (readingType >= 10 && readingType <= 19)
    {
      Serial.print("Reading type is: Open");
      Serial.println("");
      File data = SD.open("8_c_o", FILE_READ);
      printer.printBitmap(384, 661, dynamic_cast<Stream*>(&data));
      data.close();
    }
    else if (readingType >= 20)
    {
      Serial.print("Reading type is: Reversed");
      Serial.println("");
      File data = SD.open("8_c_r", FILE_READ);
      printer.printBitmap(384, 661, dynamic_cast<Stream*>(&data));
      data.close();
    }
  printer.feed(1);
  printer.justify('C');
  printer.setSize('S');
  printer.upsideDownOn();
  printer.println(F("Eight of Cups"));    
  }

    else if (val == 920) // Nine of Cups
  {
//  int readingType = 5;
//  int readingType = 15;
//  int readingType = 25;  
  int readingType = Entropy.random(0, 31);  
  Serial.print("Nine of Cups");
  Serial.println("");
  Serial.println("");
    if (readingType <= 9)
    {
      Serial.print("Reading type is: Upright");
      Serial.println("");
      File data = SD.open("9_c_u", FILE_READ);
      printer.printBitmap(384, 661, dynamic_cast<Stream*>(&data));
      data.close();
    }
    else if (readingType >= 10 && readingType <= 19)
    {
      Serial.print("Reading type is: Open");
      Serial.println("");
      File data = SD.open("9_c_o", FILE_READ);
      printer.printBitmap(384, 661, dynamic_cast<Stream*>(&data));
      data.close();
    }
    else if (readingType >= 20)
    {
      Serial.print("Reading type is: Reversed");
      Serial.println("");
      File data = SD.open("9_c_r", FILE_READ);
      printer.printBitmap(384, 661, dynamic_cast<Stream*>(&data));
      data.close();
    }
  printer.feed(1);
  printer.justify('C');
  printer.setSize('S');
  printer.upsideDownOn();
  printer.println(F("Nine of Cups"));    
  }

    else if (val == 940) // Ten of Cups
  {
//  int readingType = 5;
//  int readingType = 15;
//  int readingType = 25;  
  int readingType = Entropy.random(0, 31);  
  Serial.print("Ten of Cups");
  Serial.println("");
  Serial.println("");
    if (readingType <= 9)
    {
      Serial.print("Reading type is: Upright");
      Serial.println("");
      File data = SD.open("10_c_u", FILE_READ);
      printer.printBitmap(384, 661, dynamic_cast<Stream*>(&data));
      data.close();
    }
    else if (readingType >= 10 && readingType <= 19)
    {
      Serial.print("Reading type is: Open");
      Serial.println("");
      File data = SD.open("10_c_o", FILE_READ);
      printer.printBitmap(384, 661, dynamic_cast<Stream*>(&data));
      data.close();
    }
    else if (readingType >= 20)
    {
      Serial.print("Reading type is: Reversed");
      Serial.println("");
      File data = SD.open("10_c_r", FILE_READ);
      printer.printBitmap(384, 661, dynamic_cast<Stream*>(&data));
      data.close();
    }
  printer.feed(1);
  printer.justify('C');
  printer.setSize('S');
  printer.upsideDownOn();
  printer.println(F("Ten of Cups"));    
  }

    else if (val == 960) // Page of Cups
  {
//  int readingType = 5;
//  int readingType = 15;
//  int readingType = 25;  
  int readingType = Entropy.random(0, 31);  
  Serial.print("Page of Cups");
  Serial.println("");
  Serial.println("");
    if (readingType <= 9)
    {
      Serial.print("Reading type is: Upright");
      Serial.println("");
      File data = SD.open("p_c_u", FILE_READ);
      printer.printBitmap(384, 661, dynamic_cast<Stream*>(&data));
      data.close();
    }
    else if (readingType >= 10 && readingType <= 19)
    {
      Serial.print("Reading type is: Open");
      Serial.println("");
      File data = SD.open("p_c_o", FILE_READ);
      printer.printBitmap(384, 661, dynamic_cast<Stream*>(&data));
      data.close();
    }
    else if (readingType >= 20)
    {
      Serial.print("Reading type is: Reversed");
      Serial.println("");
      File data = SD.open("p_c_r", FILE_READ);
      printer.printBitmap(384, 661, dynamic_cast<Stream*>(&data));
      data.close();
    }   
  }

    else if (val == 980) // Knight of Cups
  {
//  int readingType = 5;
//  int readingType = 15;
//  int readingType = 25;  
  int readingType = Entropy.random(0, 31);  
  Serial.print("Knight of Cups");
  Serial.println("");
  Serial.println("");
    if (readingType <= 9)
    {
      Serial.print("Reading type is: Upright");
      Serial.println("");
      File data = SD.open("kn_c_u", FILE_READ);
      printer.printBitmap(384, 661, dynamic_cast<Stream*>(&data));
      data.close();
    }
    else if (readingType >= 10 && readingType <= 19)
    {
      Serial.print("Reading type is: Open");
      Serial.println("");
      File data = SD.open("kn_c_o", FILE_READ);
      printer.printBitmap(384, 661, dynamic_cast<Stream*>(&data));
      data.close();
    }
    else if (readingType >= 20)
    {
      Serial.print("Reading type is: Reversed");
      Serial.println("");
      File data = SD.open("kn_c_r", FILE_READ);
      printer.printBitmap(384, 661, dynamic_cast<Stream*>(&data));
      data.close();
    }
  }

    else if (val == 1000) // Queen of Cups
  {
//  int readingType = 5;
//  int readingType = 15;
//  int readingType = 25;  
  int readingType = Entropy.random(0, 31);  
  Serial.print("Queen of Cups");
  Serial.println("");
  Serial.println("");
    if (readingType <= 9)
    {
      Serial.print("Reading type is: Upright");
      Serial.println("");
      File data = SD.open("q_c_u", FILE_READ);
      printer.printBitmap(384, 661, dynamic_cast<Stream*>(&data));
      data.close();
    }
    else if (readingType >= 10 && readingType <= 19)
    {
      Serial.print("Reading type is: Open");
      Serial.println("");
      File data = SD.open("q_c_o", FILE_READ);
      printer.printBitmap(384, 661, dynamic_cast<Stream*>(&data));
      data.close();
    }
    else if (readingType >= 20)
    {
      Serial.print("Reading type is: Reversed");
      Serial.println("");
      File data = SD.open("q_c_r", FILE_READ);
      printer.printBitmap(384, 661, dynamic_cast<Stream*>(&data));
      data.close();
    }
  }

    else if (val == 1020) // King of Cups
  {
//  int readingType = 5;
//  int readingType = 15;
//  int readingType = 25;  
  int readingType = Entropy.random(0, 31);  
  Serial.print("King of Cups");
  Serial.println("");
  Serial.println("");
    if (readingType <= 9)
    {
      Serial.print("Reading type is: Upright");
      Serial.println("");
      File data = SD.open("k_c_u", FILE_READ);
      printer.printBitmap(384, 661, dynamic_cast<Stream*>(&data));
      data.close();
    }
    else if (readingType >= 10 && readingType <= 19)
    {
      Serial.print("Reading type is: Open");
      Serial.println("");
      File data = SD.open("k_c_o", FILE_READ);
      printer.printBitmap(384, 661, dynamic_cast<Stream*>(&data));
      data.close();
    }
    else if (readingType >= 20)
    {
      Serial.print("Reading type is: Reversed");
      Serial.println("");
      File data = SD.open("k_c_r", FILE_READ);
      printer.printBitmap(384, 661, dynamic_cast<Stream*>(&data));
      data.close();
    }
  }

    else if (val == 1040) // Ace of Swords
  {
//  int readingType = 5;
//  int readingType = 15;
//  int readingType = 25;  
  int readingType = Entropy.random(0, 31);  
  Serial.print("Ace of Swords");
  Serial.println("");
  Serial.println("");
    if (readingType <= 9)
    {
      Serial.print("Reading type is: Upright");
      Serial.println("");
      File data = SD.open("a_s_u", FILE_READ);
      printer.printBitmap(384, 661, dynamic_cast<Stream*>(&data));
      data.close();
    }
    else if (readingType >= 10 && readingType <= 19)
    {
      Serial.print("Reading type is: Open");
      Serial.println("");
      File data = SD.open("a_s_o", FILE_READ);
      printer.printBitmap(384, 661, dynamic_cast<Stream*>(&data));
      data.close();
    }
    else if (readingType >= 20)
    {
      Serial.print("Reading type is: Reversed");
      Serial.println("");
      File data = SD.open("a_s_r", FILE_READ);
      printer.printBitmap(384, 661, dynamic_cast<Stream*>(&data));
      data.close();
    }
  }

    else if (val == 1060) // Two of Swords
  {
//  int readingType = 5;
//  int readingType = 15;
//  int readingType = 25;  
  int readingType = Entropy.random(0, 31);  
  Serial.print("Two of Swords");
  Serial.println("");
  Serial.println("");
    if (readingType <= 9)
    {
      Serial.print("Reading type is: Upright");
      Serial.println("");
      File data = SD.open("2_s_u", FILE_READ);
      printer.printBitmap(384, 661, dynamic_cast<Stream*>(&data));
      data.close();
    }
    else if (readingType >= 10 && readingType <= 19)
    {
      Serial.print("Reading type is: Open");
      Serial.println("");
      File data = SD.open("2_s_o", FILE_READ);
      printer.printBitmap(384, 661, dynamic_cast<Stream*>(&data));
      data.close();
    }
    else if (readingType >= 20)
    {
      Serial.print("Reading type is: Reversed");
      Serial.println("");
      File data = SD.open("2_s_r", FILE_READ);
      printer.printBitmap(384, 661, dynamic_cast<Stream*>(&data));
      data.close();
    }
  printer.feed(1);
  printer.justify('C');
  printer.setSize('S');
  printer.upsideDownOn();
  printer.println(F("Two of Swords"));    
  }

    else if (val == 1080) // Three of Swords
  {
//  int readingType = 5;
//  int readingType = 15;
//  int readingType = 25;  
  int readingType = Entropy.random(0, 31);  
  Serial.print("Three of Swords");
  Serial.println("");
  Serial.println("");
    if (readingType <= 9)
    {
      Serial.print("Reading type is: Upright");
      Serial.println("");
      File data = SD.open("3_s_u", FILE_READ);
      printer.printBitmap(384, 661, dynamic_cast<Stream*>(&data));
      data.close();
    }
    else if (readingType >= 10 && readingType <= 19)
    {
      Serial.print("Reading type is: Open");
      Serial.println("");
      File data = SD.open("3_s_o", FILE_READ);
      printer.printBitmap(384, 661, dynamic_cast<Stream*>(&data));
      data.close();
    }
    else if (readingType >= 20)
    {
      Serial.print("Reading type is: Reversed");
      Serial.println("");
      File data = SD.open("3_s_r", FILE_READ);
      printer.printBitmap(384, 661, dynamic_cast<Stream*>(&data));
      data.close();
    }
  printer.feed(1);
  printer.justify('C');
  printer.setSize('S');
  printer.upsideDownOn();
  printer.println(F("Three of Swords"));    
  }

    else if (val == 1100) // Four of Swords
  {
//  int readingType = 5;
//  int readingType = 15;
//  int readingType = 25;  
  int readingType = Entropy.random(0, 31);  
  Serial.print("Four of Swords");
  Serial.println("");
  Serial.println("");
    if (readingType <= 9)
    {
      Serial.print("Reading type is: Upright");
      Serial.println("");
      File data = SD.open("4_s_u", FILE_READ);
      printer.printBitmap(384, 661, dynamic_cast<Stream*>(&data));
      data.close();
    }
    else if (readingType >= 10 && readingType <= 19)
    {
      Serial.print("Reading type is: Open");
      Serial.println("");
      File data = SD.open("4_s_o", FILE_READ);
      printer.printBitmap(384, 661, dynamic_cast<Stream*>(&data));
      data.close();
    }
    else if (readingType >= 20)
    {
      Serial.print("Reading type is: Reversed");
      Serial.println("");
      File data = SD.open("4_s_r", FILE_READ);
      printer.printBitmap(384, 661, dynamic_cast<Stream*>(&data));
      data.close();
    }
  printer.feed(1);
  printer.justify('C');
  printer.setSize('S');
  printer.upsideDownOn();
  printer.println(F("Four of Swords"));    
  }

    else if (val == 1120) // Five of Swords
  {
//  int readingType = 5;
//  int readingType = 15;
//  int readingType = 25;  
  int readingType = Entropy.random(0, 31);  
  Serial.print("Five of Swords");
  Serial.println("");
  Serial.println("");
    if (readingType <= 9)
    {
      Serial.print("Reading type is: Upright");
      Serial.println("");
      File data = SD.open("5_s_u", FILE_READ);
      printer.printBitmap(384, 661, dynamic_cast<Stream*>(&data));
      data.close();
    }
    else if (readingType >= 10 && readingType <= 19)
    {
      Serial.print("Reading type is: Open");
      Serial.println("");
      File data = SD.open("5_s_o", FILE_READ);
      printer.printBitmap(384, 661, dynamic_cast<Stream*>(&data));
      data.close();
    }
    else if (readingType >= 20)
    {
      Serial.print("Reading type is: Reversed");
      Serial.println("");
      File data = SD.open("5_s_r", FILE_READ);
      printer.printBitmap(384, 661, dynamic_cast<Stream*>(&data));
      data.close();
    }
  printer.feed(1);
  printer.justify('C');
  printer.setSize('S');
  printer.upsideDownOn();
  printer.println(F("Five of Swords")); 
  for(x= 0; x< 200 ; x++)  //Loop forward enough times to stop a false home
  {
    digitalWrite(stp,HIGH); //Trigger one step forward
    delayMicroseconds(1275);
    digitalWrite(stp,LOW); //Pull step pin low so it can be triggered again
    delayMicroseconds(1275);
   }    
  }

    else if (val == 1140) // Six of Swords
  {
//  int readingType = 5;
//  int readingType = 15;
//  int readingType = 25;  
  int readingType = Entropy.random(0, 31);  
  Serial.print("Six of Swords");
  Serial.println("");
  Serial.println("");
    if (readingType <= 9)
    {
      Serial.print("Reading type is: Upright");
      Serial.println("");
      File data = SD.open("6_s_u", FILE_READ);
      printer.printBitmap(384, 661, dynamic_cast<Stream*>(&data));
      data.close();
    }
    else if (readingType >= 10 && readingType <= 19)
    {
      Serial.print("Reading type is: Open");
      Serial.println("");
      File data = SD.open("6_s_o", FILE_READ);
      printer.printBitmap(384, 661, dynamic_cast<Stream*>(&data));
      data.close();
    }
    else if (readingType >= 20)
    {
      Serial.print("Reading type is: Reversed");
      Serial.println("");
      File data = SD.open("6_s_r", FILE_READ);
      printer.printBitmap(384, 661, dynamic_cast<Stream*>(&data));
      data.close();
    }
  printer.feed(1);
  printer.justify('C');
  printer.setSize('S');
  printer.upsideDownOn();
  printer.println(F("Six of Swords")); 
  for(x= 0; x< 200 ; x++)  //Loop forward enough times to stop a false home
  {
    digitalWrite(stp,HIGH); //Trigger one step forward
    delayMicroseconds(1275);
    digitalWrite(stp,LOW); //Pull step pin low so it can be triggered again
    delayMicroseconds(1275);
   }  
  }

    else if (val == 1160) // Seven of Swords
  {
//  int readingType = 5;
//  int readingType = 15;
//  int readingType = 25;  
  int readingType = Entropy.random(0, 31);  
  Serial.print("Seven of Swords");
  Serial.println("");
  Serial.println("");
    if (readingType <= 9)
    {
      Serial.print("Reading type is: Upright");
      Serial.println("");
      File data = SD.open("7_s_u", FILE_READ);
      printer.printBitmap(384, 661, dynamic_cast<Stream*>(&data));
      data.close();
    }
    else if (readingType >= 10 && readingType <= 19)
    {
      Serial.print("Reading type is: Open");
      Serial.println("");
      File data = SD.open("7_s_o", FILE_READ);
      printer.printBitmap(384, 661, dynamic_cast<Stream*>(&data));
      data.close();
    }
    else if (readingType >= 20)
    {
      Serial.print("Reading type is: Reversed");
      Serial.println("");
      File data = SD.open("7_s_r", FILE_READ);
      printer.printBitmap(384, 661, dynamic_cast<Stream*>(&data));
      data.close();
    }
  printer.feed(1);
  printer.justify('C');
  printer.setSize('S');
  printer.upsideDownOn();
  printer.println(F("Seven of Swords"));     
  for(x= 0; x< 200 ; x++)  //Loop forward enough times to stop a false home
  {
    digitalWrite(stp,HIGH); //Trigger one step forward
    delayMicroseconds(1275);
    digitalWrite(stp,LOW); //Pull step pin low so it can be triggered again
    delayMicroseconds(1275);
   }  
  }

    else if (val == 1180) // Eight of Swords
  {
//  int readingType = 5;
//  int readingType = 15;
//  int readingType = 25;  
  int readingType = Entropy.random(0, 31);  
  Serial.print("Eight of Swords");
  Serial.println("");
  Serial.println("");
    if (readingType <= 9)
    {
      Serial.print("Reading type is: Upright");
      Serial.println("");
      File data = SD.open("8_s_u", FILE_READ);
      printer.printBitmap(384, 661, dynamic_cast<Stream*>(&data));
      data.close();
    }
    else if (readingType >= 10 && readingType <= 19)
    {
      Serial.print("Reading type is: Open");
      Serial.println("");
      File data = SD.open("8_s_o", FILE_READ);
      printer.printBitmap(384, 661, dynamic_cast<Stream*>(&data));
      data.close();
    }
    else if (readingType >= 20)
    {
      Serial.print("Reading type is: Reversed");
      Serial.println("");
      File data = SD.open("8_s_r", FILE_READ);
      printer.printBitmap(384, 661, dynamic_cast<Stream*>(&data));
      data.close();
    }
  printer.feed(1);
  printer.justify('C');
  printer.setSize('S');
  printer.upsideDownOn();
  printer.println(F("Eight of Swords"));     
  for(x= 0; x< 200 ; x++)  //Loop forward enough times to stop a false home
  {
    digitalWrite(stp,HIGH); //Trigger one step forward
    delayMicroseconds(1275);
    digitalWrite(stp,LOW); //Pull step pin low so it can be triggered again
    delayMicroseconds(1275);
   }  
  }

    else if (val == 1200) // Nine of Swords
  {
//  int readingType = 5;
//  int readingType = 15;
//  int readingType = 25;  
  int readingType = Entropy.random(0, 31);  
  Serial.print("Nine of Swords");
  Serial.println("");
  Serial.println("");
    if (readingType <= 9)
    {
      Serial.print("Reading type is: Upright");
      Serial.println("");
      File data = SD.open("9_s_u", FILE_READ);
      printer.printBitmap(384, 661, dynamic_cast<Stream*>(&data));
      data.close();
    }
    else if (readingType >= 10 && readingType <= 19)
    {
      Serial.print("Reading type is: Open");
      Serial.println("");
      File data = SD.open("9_s_o", FILE_READ);
      printer.printBitmap(384, 661, dynamic_cast<Stream*>(&data));
      data.close();
    }
    else if (readingType >= 20)
    {
      Serial.print("Reading type is: Reversed");
      Serial.println("");
      File data = SD.open("9_s_r", FILE_READ);
      printer.printBitmap(384, 661, dynamic_cast<Stream*>(&data));
      data.close();
    }
  printer.feed(1);
  printer.justify('C');
  printer.setSize('S');
  printer.upsideDownOn();
  printer.println(F("Nine of Swords"));     
  }

    else if (val == 1220) // Ten of Swords
  {
//  int readingType = 5;
//  int readingType = 15;
//  int readingType = 25;  
  int readingType = Entropy.random(0, 31);  
  Serial.print("Ten of Swords");
  Serial.println("");
  Serial.println("");
    if (readingType <= 9)
    {
      Serial.print("Reading type is: Upright");
      Serial.println("");
      File data = SD.open("10_s_u", FILE_READ);
      printer.printBitmap(384, 661, dynamic_cast<Stream*>(&data));
      data.close();
    }
    else if (readingType >= 10 && readingType <= 19)
    {
      Serial.print("Reading type is: Open");
      Serial.println("");
      File data = SD.open("10_s_o", FILE_READ);
      printer.printBitmap(384, 661, dynamic_cast<Stream*>(&data));
      data.close();
    }
    else if (readingType >= 20)
    {
      Serial.print("Reading type is: Reversed");
      Serial.println("");
      File data = SD.open("10_s_r", FILE_READ);
      printer.printBitmap(384, 661, dynamic_cast<Stream*>(&data));
      data.close();
    }
  printer.feed(1);
  printer.justify('C');
  printer.setSize('S');
  printer.upsideDownOn();
  printer.println(F("Ten of Swords"));     
  }

    else if (val == 1240) // Page of Swords
  {
//  int readingType = 5;
//  int readingType = 15;
//  int readingType = 25;  
  int readingType = Entropy.random(0, 31);  
  Serial.print("Page of Swords");
  Serial.println("");
  Serial.println("");
    if (readingType <= 9)
    {
      Serial.print("Reading type is: Upright");
      Serial.println("");
      File data = SD.open("p_s_u", FILE_READ);
      printer.printBitmap(384, 661, dynamic_cast<Stream*>(&data));
      data.close();
    }
    else if (readingType >= 10 && readingType <= 19)
    {
      Serial.print("Reading type is: Open");
      Serial.println("");
      File data = SD.open("p_s_o", FILE_READ);
      printer.printBitmap(384, 661, dynamic_cast<Stream*>(&data));
      data.close();
    }
    else if (readingType >= 20)
    {
      Serial.print("Reading type is: Reversed");
      Serial.println("");
      File data = SD.open("p_s_r", FILE_READ);
      printer.printBitmap(384, 661, dynamic_cast<Stream*>(&data));
      data.close();
    }  
  }

    else if (val == 1260) // Knight of Swords
  {
//  int readingType = 5;
//  int readingType = 15;
//  int readingType = 25;  
  int readingType = Entropy.random(0, 31);  
  Serial.print("Knight of Swords");
  Serial.println("");
  Serial.println("");
    if (readingType <= 9)
    {
      Serial.print("Reading type is: Upright");
      Serial.println("");
      File data = SD.open("kn_s_u", FILE_READ);
      printer.printBitmap(384, 661, dynamic_cast<Stream*>(&data));
      data.close();
    }
    else if (readingType >= 10 && readingType <= 19)
    {
      Serial.print("Reading type is: Open");
      Serial.println("");
      File data = SD.open("kn_s_o", FILE_READ);
      printer.printBitmap(384, 661, dynamic_cast<Stream*>(&data));
      data.close();
    }
    else if (readingType >= 20)
    {
      Serial.print("Reading type is: Reversed");
      Serial.println("");
      File data = SD.open("kn_s_r", FILE_READ);
      printer.printBitmap(384, 661, dynamic_cast<Stream*>(&data));
      data.close();
    }
  }

    else if (val == 1280) // Queen of Swords
  {
//  int readingType = 5;
//  int readingType = 15;
//  int readingType = 25;  
  int readingType = Entropy.random(0, 31);  
  Serial.print("Queen of Swords");
  Serial.println("");
  Serial.println("");
    if (readingType <= 9)
    {
      Serial.print("Reading type is: Upright");
      Serial.println("");
      File data = SD.open("q_s_u", FILE_READ);
      printer.printBitmap(384, 661, dynamic_cast<Stream*>(&data));
      data.close();
    }
    else if (readingType >= 10 && readingType <= 19)
    {
      Serial.print("Reading type is: Open");
      Serial.println("");
      File data = SD.open("q_s_o", FILE_READ);
      printer.printBitmap(384, 661, dynamic_cast<Stream*>(&data));
      data.close();
    }
    else if (readingType >= 20)
    {
      Serial.print("Reading type is: Reversed");
      Serial.println("");
      File data = SD.open("q_s_r", FILE_READ);
      printer.printBitmap(384, 661, dynamic_cast<Stream*>(&data));
      data.close();
    }
  }

    else if (val == 1300) // King of Swords
  {
//  int readingType = 5;
//  int readingType = 15;
//  int readingType = 25;  
  int readingType = Entropy.random(0, 31);  
  Serial.print("King of Swords");
  Serial.println("");
  Serial.println("");
    if (readingType <= 9)
    {
      Serial.print("Reading type is: Upright");
      Serial.println("");
      File data = SD.open("k_s_u", FILE_READ);
      printer.printBitmap(384, 661, dynamic_cast<Stream*>(&data));
      data.close();
    }
    else if (readingType >= 10 && readingType <= 19)
    {
      Serial.print("Reading type is: Open");
      Serial.println("");
      File data = SD.open("k_s_o", FILE_READ);
      printer.printBitmap(384, 661, dynamic_cast<Stream*>(&data));
      data.close();
    }
    else if (readingType >= 20)
    {
      Serial.print("Reading type is: Reversed");
      Serial.println("");
      File data = SD.open("k_s_r", FILE_READ);
      printer.printBitmap(384, 661, dynamic_cast<Stream*>(&data));
      data.close();
    }
  }

    else if (val == 1320) // Ace of Pentacles
  {
//  int readingType = 5;
//  int readingType = 15;
//  int readingType = 25;  
  int readingType = Entropy.random(0, 31);  
  Serial.print("Ace of Pentacles");
  Serial.println("");
  Serial.println("");
    if (readingType <= 9)
    {
      Serial.print("Reading type is: Upright");
      Serial.println("");
      File data = SD.open("a_p_u", FILE_READ);
      printer.printBitmap(384, 661, dynamic_cast<Stream*>(&data));
      data.close();
    }
    else if (readingType >= 10 && readingType <= 19)
    {
      Serial.print("Reading type is: Open");
      Serial.println("");
      File data = SD.open("a_p_o", FILE_READ);
      printer.printBitmap(384, 661, dynamic_cast<Stream*>(&data));
      data.close();
    }
    else if (readingType >= 20)
    {
      Serial.print("Reading type is: Reversed");
      Serial.println("");
      File data = SD.open("a_p_r", FILE_READ);
      printer.printBitmap(384, 661, dynamic_cast<Stream*>(&data));
      data.close();
    }
  }

    else if (val == 1340) // Two of Pentacles
  {
//  int readingType = 5;
//  int readingType = 15;
//  int readingType = 25;  
  int readingType = Entropy.random(0, 31);  
  Serial.print("Two of Pentacles");
  Serial.println("");
  Serial.println("");
    if (readingType <= 9)
    {
      Serial.print("Reading type is: Upright");
      Serial.println("");
      File data = SD.open("2_p_u", FILE_READ);
      printer.printBitmap(384, 661, dynamic_cast<Stream*>(&data));
      data.close();
    }
    else if (readingType >= 10 && readingType <= 19)
    {
      Serial.print("Reading type is: Open");
      Serial.println("");
      File data = SD.open("2_p_o", FILE_READ);
      printer.printBitmap(384, 661, dynamic_cast<Stream*>(&data));
      data.close();
    }
    else if (readingType >= 20)
    {
      Serial.print("Reading type is: Reversed");
      Serial.println("");
      File data = SD.open("2_p_r", FILE_READ);
      printer.printBitmap(384, 661, dynamic_cast<Stream*>(&data));
      data.close();
    }
  printer.feed(1);
  printer.justify('C');
  printer.setSize('S');
  printer.upsideDownOn();
  printer.println(F("Two of Pentacles"));     
  }

    else if (val == 1360) // Three of Pentacles
  {
//  int readingType = 5;
//  int readingType = 15;
//  int readingType = 25;  
  int readingType = Entropy.random(0, 31);  
  Serial.print("Three of Pentacles");
  Serial.println("");
  Serial.println("");
    if (readingType <= 9)
    {
      Serial.print("Reading type is: Upright");
      Serial.println("");
      File data = SD.open("3_p_u", FILE_READ);
      printer.printBitmap(384, 661, dynamic_cast<Stream*>(&data));
      data.close();
    }
    else if (readingType >= 10 && readingType <= 19)
    {
      Serial.print("Reading type is: Open");
      Serial.println("");
      File data = SD.open("3_p_o", FILE_READ);
      printer.printBitmap(384, 661, dynamic_cast<Stream*>(&data));
      data.close();
    }
    else if (readingType >= 20)
    {
      Serial.print("Reading type is: Reversed");
      Serial.println("");
      File data = SD.open("3_p_r", FILE_READ);
      printer.printBitmap(384, 661, dynamic_cast<Stream*>(&data));
      data.close();
    }
  printer.feed(1);
  printer.justify('C');
  printer.setSize('S');
  printer.upsideDownOn();
  printer.println(F("Three of Pentacles")); 
  }

    else if (val == 1380) // Four of Pentacles
  {
//  int readingType = 5;
//  int readingType = 15;
//  int readingType = 25;  
  int readingType = Entropy.random(0, 31);  
  Serial.print("Four of Pentacles");
  Serial.println("");
  Serial.println("");
    if (readingType <= 9)
    {
      Serial.print("Reading type is: Upright");
      Serial.println("");
      File data = SD.open("4_p_u", FILE_READ);
      printer.printBitmap(384, 661, dynamic_cast<Stream*>(&data));
      data.close();
    }
    else if (readingType >= 10 && readingType <= 19)
    {
      Serial.print("Reading type is: Open");
      Serial.println("");
      File data = SD.open("4_p_o", FILE_READ);
      printer.printBitmap(384, 661, dynamic_cast<Stream*>(&data));
      data.close();
    }
    else if (readingType >= 20)
    {
      Serial.print("Reading type is: Reversed");
      Serial.println("");
      File data = SD.open("4_p_r", FILE_READ);
      printer.printBitmap(384, 661, dynamic_cast<Stream*>(&data));
      data.close();
    }
  printer.feed(1);
  printer.justify('C');
  printer.setSize('S');
  printer.upsideDownOn();
  printer.println(F("Four of Pentacles"));    
  }

    else if (val == 1400) // Five of Pentacles
  {
//  int readingType = 5;
//  int readingType = 15;
//  int readingType = 25;  
  int readingType = Entropy.random(0, 31);  
  Serial.print("Five of Pentacles");
  Serial.println("");
  Serial.println("");
    if (readingType <= 9)
    {
      Serial.print("Reading type is: Upright");
      Serial.println("");
      File data = SD.open("5_p_u", FILE_READ);
      printer.printBitmap(384, 661, dynamic_cast<Stream*>(&data));
      data.close();
    }
    else if (readingType >= 10 && readingType <= 19)
    {
      Serial.print("Reading type is: Open");
      Serial.println("");
      File data = SD.open("5_p_o", FILE_READ);
      printer.printBitmap(384, 661, dynamic_cast<Stream*>(&data));
      data.close();
    }
    else if (readingType >= 20)
    {
      Serial.print("Reading type is: Reversed");
      Serial.println("");
      File data = SD.open("5_p_r", FILE_READ);
      printer.printBitmap(384, 661, dynamic_cast<Stream*>(&data));
      data.close();
    }
  printer.feed(1);
  printer.justify('C');
  printer.setSize('S');
  printer.upsideDownOn();
  printer.println(F("Five of Pentacles"));    
  }

    else if (val == 1420) // Six of Pentacles
  {
//  int readingType = 5;
//  int readingType = 15;
//  int readingType = 25;  
  int readingType = Entropy.random(0, 31);  
  Serial.print("Six of Pentacles");
  Serial.println("");
  Serial.println("");
    if (readingType <= 9)
    {
      Serial.print("Reading type is: Upright");
      Serial.println("");
      File data = SD.open("6_p_u", FILE_READ);
      printer.printBitmap(384, 661, dynamic_cast<Stream*>(&data));
      data.close();
    }
    else if (readingType >= 10 && readingType <= 19)
    {
      Serial.print("Reading type is: Open");
      Serial.println("");
      File data = SD.open("6_p_o", FILE_READ);
      printer.printBitmap(384, 661, dynamic_cast<Stream*>(&data));
      data.close();
    }
    else if (readingType >= 20)
    {
      Serial.print("Reading type is: Reversed");
      Serial.println("");
      File data = SD.open("6_p_r", FILE_READ);
      printer.printBitmap(384, 661, dynamic_cast<Stream*>(&data));
      data.close();
    }
  printer.feed(1);
  printer.justify('C');
  printer.setSize('S');
  printer.upsideDownOn();
  printer.println(F("Six of Pentacles"));    
  }

    else if (val == 1440) // Seven of Pentacles
  {
//  int readingType = 5;
//  int readingType = 15;
//  int readingType = 25;  
  int readingType = Entropy.random(0, 31);  
  Serial.print("Seven of Pentacles");
  Serial.println("");
  Serial.println("");
    if (readingType <= 9)
    {
      Serial.print("Reading type is: Upright");
      Serial.println("");
      File data = SD.open("7_p_u", FILE_READ);
      printer.printBitmap(384, 661, dynamic_cast<Stream*>(&data));
      data.close();
    }
    else if (readingType >= 10 && readingType <= 19)
    {
      Serial.print("Reading type is: Open");
      Serial.println("");
      File data = SD.open("7_p_o", FILE_READ);
      printer.printBitmap(384, 661, dynamic_cast<Stream*>(&data));
      data.close();
    }
    else if (readingType >= 20)
    {
      Serial.print("Reading type is: Reversed");
      Serial.println("");
      File data = SD.open("7_p_r", FILE_READ);
      printer.printBitmap(384, 661, dynamic_cast<Stream*>(&data));
      data.close();
    }
  printer.feed(1);
  printer.justify('C');
  printer.setSize('S');
  printer.upsideDownOn();
  printer.println(F("Seven of Pentacles"));    
  }

    else if (val == 1460) // Eight of Pentacles
  {
//  int readingType = 5;
//  int readingType = 15;
//  int readingType = 25;  
  int readingType = Entropy.random(0, 31);  
  Serial.print("Eight of Pentacles");
  Serial.println("");
  Serial.println("");
    if (readingType <= 9)
    {
      Serial.print("Reading type is: Upright");
      Serial.println("");
      File data = SD.open("8_p_u", FILE_READ);
      printer.printBitmap(384, 661, dynamic_cast<Stream*>(&data));
      data.close();
    }
    else if (readingType >= 10 && readingType <= 19)
    {
      Serial.print("Reading type is: Open");
      Serial.println("");
      File data = SD.open("8_p_o", FILE_READ);
      printer.printBitmap(384, 661, dynamic_cast<Stream*>(&data));
      data.close();
    }
    else if (readingType >= 20)
    {
      Serial.print("Reading type is: Reversed");
      Serial.println("");
      File data = SD.open("8_p_r", FILE_READ);
      printer.printBitmap(384, 661, dynamic_cast<Stream*>(&data));
      data.close();
    }
  printer.feed(1);
  printer.justify('C');
  printer.setSize('S');
  printer.upsideDownOn();
  printer.println(F("Eight of Pentacles"));    
  }

    else if (val == 1480) // Nine of Pentacles
  {
//  int readingType = 5;
//  int readingType = 15;
//  int readingType = 25;  
  int readingType = Entropy.random(0, 31);  
  Serial.print("Nine of Pentacles");
  Serial.println("");
  Serial.println("");
    if (readingType <= 9)
    {
      Serial.print("Reading type is: Upright");
      Serial.println("");
      File data = SD.open("9_p_u", FILE_READ);
      printer.printBitmap(384, 661, dynamic_cast<Stream*>(&data));
      data.close();
    }
    else if (readingType >= 10 && readingType <= 19)
    {
      Serial.print("Reading type is: Open");
      Serial.println("");
      File data = SD.open("9_p_o", FILE_READ);
      printer.printBitmap(384, 661, dynamic_cast<Stream*>(&data));
      data.close();
    }
    else if (readingType >= 20)
    {
      Serial.print("Reading type is: Reversed");
      Serial.println("");
      File data = SD.open("9_p_r", FILE_READ);
      printer.printBitmap(384, 661, dynamic_cast<Stream*>(&data));
      data.close();
    }
  printer.feed(1);
  printer.justify('C');
  printer.setSize('S');
  printer.upsideDownOn();
  printer.println(F("Nine of Pentacles"));    
  }

    else if (val == 1500) // Ten of Pentacles
  {
//  int readingType = 5;
//  int readingType = 15;
//  int readingType = 25;  
  int readingType = Entropy.random(0, 31);  
  Serial.print("Ten of Pentacles");
  Serial.println("");
  Serial.println("");
    if (readingType <= 9)
    {
      Serial.print("Reading type is: Upright");
      Serial.println("");
      File data = SD.open("10_p_u", FILE_READ);
      printer.printBitmap(384, 661, dynamic_cast<Stream*>(&data));
      data.close();
    }
    else if (readingType >= 10 && readingType <= 19)
    {
      Serial.print("Reading type is: Open");
      Serial.println("");
      File data = SD.open("10_p_o", FILE_READ);
      printer.printBitmap(384, 661, dynamic_cast<Stream*>(&data));
      data.close();
    }
    else if (readingType >= 20)
    {
      Serial.print("Reading type is: Reversed");
      Serial.println("");
      File data = SD.open("10_p_r", FILE_READ);
      printer.printBitmap(384, 661, dynamic_cast<Stream*>(&data));
      data.close();
    }
  printer.feed(1);
  printer.justify('C');
  printer.setSize('S');
  printer.upsideDownOn();
  printer.println(F("Ten of Pentacles"));    
  }

    else if (val == 1520) // Page of Pentacles
  {
//  int readingType = 5;
//  int readingType = 15;
//  int readingType = 25;  
  int readingType = Entropy.random(0, 31);  
  Serial.print("Page of Pentacles");
  Serial.println("");
  Serial.println("");
    if (readingType <= 9)
    {
      Serial.print("Reading type is: Upright");
      Serial.println("");
      File data = SD.open("p_p_u", FILE_READ);
      printer.printBitmap(384, 661, dynamic_cast<Stream*>(&data));
      data.close();
    }
    else if (readingType >= 10 && readingType <= 19)
    {
      Serial.print("Reading type is: Open");
      Serial.println("");
      File data = SD.open("p_p_o", FILE_READ);
      printer.printBitmap(384, 661, dynamic_cast<Stream*>(&data));
      data.close();
    }
    else if (readingType >= 20)
    {
      Serial.print("Reading type is: Reversed");
      Serial.println("");
      File data = SD.open("p_p_r", FILE_READ);
      printer.printBitmap(384, 661, dynamic_cast<Stream*>(&data));
      data.close();
    }
  }

    else if (val == 1540) // Knight of Pentacles
  {
//  int readingType = 5;
//  int readingType = 15;
//  int readingType = 25;  
  int readingType = Entropy.random(0, 31);  
  Serial.print("Knight of Pentacles");
  Serial.println("");
  Serial.println("");
    if (readingType <= 9)
    {
      Serial.print("Reading type is: Upright");
      Serial.println("");
      File data = SD.open("kn_p_u", FILE_READ);
      printer.printBitmap(384, 661, dynamic_cast<Stream*>(&data));
      data.close();
    }
    else if (readingType >= 10 && readingType <= 19)
    {
      Serial.print("Reading type is: Open");
      Serial.println("");
      File data = SD.open("kn_p_o", FILE_READ);
      printer.printBitmap(384, 661, dynamic_cast<Stream*>(&data));
      data.close();
    }
    else if (readingType >= 20)
    {
      Serial.print("Reading type is: Reversed");
      Serial.println("");
      File data = SD.open("kn_p_r", FILE_READ);
      printer.printBitmap(384, 661, dynamic_cast<Stream*>(&data));
      data.close();
    }
  }

    else if (val == 1560) // Queen of Pentacles
  {
//  int readingType = 5;
//  int readingType = 15;
//  int readingType = 25;  
  int readingType = Entropy.random(0, 31);  
  Serial.print("Queen of Pentacles");
  Serial.println("");
  Serial.println("");
    if (readingType <= 9)
    {
      Serial.print("Reading type is: Upright");
      Serial.println("");
      File data = SD.open("q_p_u", FILE_READ);
      printer.printBitmap(384, 661, dynamic_cast<Stream*>(&data));
      data.close();
    }
    else if (readingType >= 10 && readingType <= 19)
    {
      Serial.print("Reading type is: Open");
      Serial.println("");
      File data = SD.open("q_p_o", FILE_READ);
      printer.printBitmap(384, 661, dynamic_cast<Stream*>(&data));
      data.close();
    }
    else if (readingType >= 20)
    {
      Serial.print("Reading type is: Reversed");
      Serial.println("");
      File data = SD.open("q_p_r", FILE_READ);
      printer.printBitmap(384, 661, dynamic_cast<Stream*>(&data));
      data.close();
    }
  }

    else if (val == 1580) // King of Pentacles
  {
//  int readingType = 5;
//  int readingType = 15;
//  int readingType = 25;  
  int readingType = Entropy.random(0, 31);  
  Serial.print("King of Pentacles");
  Serial.println("");
  Serial.println("");
    if (readingType <= 9)
    {
      Serial.print("Reading type is: Upright");
      Serial.println("");
      File data = SD.open("k_p_u", FILE_READ);
      printer.printBitmap(384, 661, dynamic_cast<Stream*>(&data));
      data.close();
    }
    else if (readingType >= 10 && readingType <= 19)
    {
      Serial.print("Reading type is: Open");
      Serial.println("");
      File data = SD.open("k_p_o", FILE_READ);
      printer.printBitmap(384, 661, dynamic_cast<Stream*>(&data));
      data.close();
    }
    else if (readingType >= 20)
    {
      Serial.print("Reading type is: Reversed");
      Serial.println("");
      File data = SD.open("k_p_r", FILE_READ);
      printer.printBitmap(384, 661, dynamic_cast<Stream*>(&data));
      data.close();
    }
  }

//****************************************************************************
//****************************************************************************
 
//  delay(5000);  // (For debugging only) Pause so I can see card
  delay(5);
}

void Yellow(uint8_t wait) {
  for(int i=0; i<150; i++) { // Ramp up from 0 to 255
    // Fill entire strip with white at gamma-corrected brightness level 'j':
    strip.fill(strip.Color(255, 205, 0));
    strip.setBrightness(i);
    strip.show();
    delay(wait);
  }
}

void Red(uint8_t wait) {
  for(int i=0; i<150; i++) { // Ramp up from 0 to 255
    // Fill entire strip with white at gamma-corrected brightness level 'j':
    strip.fill(strip.Color(255, 0, 0));
    strip.setBrightness(i);
    strip.show();
    delay(wait);
  }
}

void Green(uint8_t wait) {
  for(int i=0; i<150; i++) { // Ramp up from 0 to 255
    // Fill entire strip with white at gamma-corrected brightness level 'j':
    strip.fill(strip.Color(0, 245, 0));
    strip.setBrightness(i);
    strip.show();
    delay(wait);
  }
}

void Blue(uint8_t wait) {
  for(int i=0; i<150; i++) { // Ramp up from 0 to 255
    // Fill entire strip with white at gamma-corrected brightness level 'j':
    strip.fill(strip.Color(0, 0, 255));
    strip.setBrightness(i);
    strip.show();
    delay(wait);
  }
}

void White(uint8_t wait) {
  for(int i=0; i<150; i++) { // Ramp up from 0 to 255
    // Fill entire strip with white at gamma-corrected brightness level 'j':
    strip.fill(strip.Color(0, 0, 0, 255));
    strip.setBrightness(i);
    strip.show();
    delay(wait);
  }
}

void colorWipe(uint32_t color, int wait) {
  for(int i=0; i<strip.numPixels(); i++) { // For each pixel in strip...
    strip.setPixelColor(i, color);         //  Set pixel's color (in RAM)
    strip.show();                          //  Update strip to match
    delay(wait);                           //  Pause for a moment
  }
}
