#include <Adafruit_Fingerprint.h>

#if (defined(__AVR__) || defined(ESP8266)) && !defined(__AVR_ATmega2560__)
SoftwareSerial mySerial(12, 11);

#else
#define mySerial Serial1

#endif

Adafruit_Fingerprint finger = Adafruit_Fingerprint(&mySerial);


#define RELAY 8 //relay pin
#define BUZZER 5 //buzzer pin
#define ACCESS_DELAY 5000

const int button1 = 3;
const int button2 = 4;
const int button3 = 2;

int echo = 6;           // membuat variabel echo yang di set ke-pin 2
int trig = 7;           // membuat varibel trig yang di set ke-pin 3
long durasi, jarak;

// all buttons set to LOW
int buttonState1 = 0;
int buttonState2 = 0;
int buttonState3 = 0;
boolean modePassword = false;
String inputPassword = "";
String validPassword = "111";
String input = "";
int waktu = 0;
boolean valid = false;
 
void setup() 
{
  Serial.begin(9600);
  while (!Serial);  // For Yun/Leo/Micro/Zero/...
  delay(100);
  Serial.println("\n\nAdafruit finger detect test");

  // set the data rate for the sensor serial port
  finger.begin(57600);
  delay(5);
  if (finger.verifyPassword()) {
    Serial.println("Found fingerprint sensor!");
  } else {
    Serial.println("Did not find fingerprint sensor :(");
    while (1) 
    { 
      tone(BUZZER, 500);
      delay(200);
      noTone(BUZZER);
      delay(200);
    }
  }

  Serial.println(F("Reading sensor parameters"));
  finger.getParameters();
  Serial.print(F("Status: 0x")); Serial.println(finger.status_reg, HEX);
  Serial.print(F("Sys ID: 0x")); Serial.println(finger.system_id, HEX);
  Serial.print(F("Capacity: ")); Serial.println(finger.capacity);
  Serial.print(F("Security level: ")); Serial.println(finger.security_level);
  Serial.print(F("Device address: ")); Serial.println(finger.device_addr, HEX);
  Serial.print(F("Packet len: ")); Serial.println(finger.packet_len);
  Serial.print(F("Baud rate: ")); Serial.println(finger.baud_rate);

  finger.getTemplateCount();

  if (finger.templateCount == 0) {
    Serial.print("Sensor doesn't contain any fingerprint data. Please run the 'enroll' example.");
  }
  else {
    Serial.println("Waiting for valid finger...");
      Serial.print("Sensor contains "); Serial.print(finger.templateCount); Serial.println(" templates");
  }

  
  pinMode(RELAY, OUTPUT);
  pinMode(BUZZER, OUTPUT);
  noTone(BUZZER);
  digitalWrite(RELAY, HIGH);
  pinMode(button1, INPUT_PULLUP);
  pinMode(button2, INPUT_PULLUP);
  pinMode(button3, INPUT_PULLUP);
  pinMode(trig, OUTPUT);    // set pin trig menjadi OUTPUT
  pinMode(echo, INPUT);     // set pin echo menjadi INPUT
  Serial.println("Starting program !");
  Serial.println();

}
void loop() 
{ 
  waktu = waktu + 1;
  if(waktu == 500)
  {
    inputPassword = "";
    waktu = 0;
    modePassword =false;
  }
  
  // program dibawah ini agar trigger memancarakan suara ultrasonic
  digitalWrite(trig, LOW);
  delayMicroseconds(8);
  digitalWrite(trig, HIGH);
  delayMicroseconds(8);
  digitalWrite(trig, LOW);
  delayMicroseconds(8);

  durasi = pulseIn(echo, HIGH); // menerima suara ultrasonic
  jarak = (durasi / 2) / 29.1;  // mengubah durasi menjadi jarak (cm)
//  Serial.print("Jarak : ");        // menampilkan jarak pada Serial Monitor
//  Serial.println(jarak); 
  
  buttonState1 = digitalRead(button1);
  buttonState2 = digitalRead(button2);
  buttonState3 = digitalRead(button3);

  if(jarak < 25){
    openDoor();
  }

  if (buttonState1 == LOW) {
    input = "1";
    toneKey(input);
  } 
  if (buttonState2 == LOW) {
    input = "2";
    toneKey(input);
  } 
  if (buttonState3 == LOW) {
    modePassword = !modePassword;
    if(modePassword)
    {
      toneModePassword();  
    }else
    {
      input = "3";
      toneKey(input);
    }   
  }

  if(modePassword)
  {
    inputPassword = inputPassword + input;
    input = "";
  } else
  {
    if(inputPassword == validPassword)
    {
      openDoor();
    }
    
    input = "";
    inputPassword = "";
  }

  getFingerprintID();
}

void openDoor()
{
  Serial.println("Valid : Authorized access");
  Serial.println();
  digitalWrite(RELAY, LOW);
  tone(BUZZER, 500);
  delay(500);
  noTone(BUZZER);
  delay(ACCESS_DELAY);
  digitalWrite(RELAY, HIGH);
}

void invalidAccess()
{
  Serial.println("Invalid : Unauthorized access");
  Serial.println();
  tone(BUZZER, 500);
  delay(200);
  noTone(BUZZER);
  delay(200);
  tone(BUZZER, 500);
  delay(200);
  noTone(BUZZER);
  delay(200);
  tone(BUZZER, 500);
  delay(200);
  noTone(BUZZER);
}

void toneKey(String key)
{
//  Serial.print("Pressed : ");
//  Serial.println(key);
  tone(BUZZER, 500);
  delay(200);
  noTone(BUZZER);
}

void toneModePassword()
{
  tone(BUZZER, 500);
  delay(200);
  noTone(BUZZER);
  delay(75);
  tone(BUZZER, 500);
  delay(200);
  noTone(BUZZER);
}

uint8_t getFingerprintID() {
  uint8_t p = finger.getImage();
  switch (p) {
    case FINGERPRINT_OK:
      Serial.println("Image taken");
      break;
    case FINGERPRINT_NOFINGER:
//      Serial.println("No finger detected");
      return p;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Communication error");
      tone(BUZZER, 500);
      delay(200);
      noTone(BUZZER);
      delay(200);
      return p;
    case FINGERPRINT_IMAGEFAIL:
      Serial.println("Imaging error");
      return p;
    default:
      Serial.println("Unknown error");
      return p;
  }

  // OK success!

  p = finger.image2Tz();
  switch (p) {
    case FINGERPRINT_OK:
      Serial.println("Image converted");
      break;
    case FINGERPRINT_IMAGEMESS:
      Serial.println("Image too messy");
      return p;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Communication error");
      return p;
    case FINGERPRINT_FEATUREFAIL:
      Serial.println("Could not find fingerprint features");
      return p;
    case FINGERPRINT_INVALIDIMAGE:
      Serial.println("Could not find fingerprint features");
      return p;
    default:
      Serial.println("Unknown error");
      return p;
  }

  // OK converted!
  p = finger.fingerSearch();
  if (p == FINGERPRINT_OK) {
    Serial.println("Found a print match!");
  } else if (p == FINGERPRINT_PACKETRECIEVEERR) {
    Serial.println("Communication error");
    return p;
  } else if (p == FINGERPRINT_NOTFOUND) {
    Serial.println("Did not find a match");
    invalidAccess();
    return p;
  } else {
    Serial.println("Unknown error");
    return p;
  }

  // found a match!
  Serial.print("Found ID #"); Serial.print(finger.fingerID);
  Serial.print(" with confidence of "); Serial.println(finger.confidence);
  openDoor();

  return finger.fingerID;
}
