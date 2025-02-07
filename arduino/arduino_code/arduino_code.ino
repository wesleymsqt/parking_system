#include <SPI.h>
#include <MFRC522.h>
#include <Servo.h>

#define RST_PIN 9    // Reset Pin for RC522
#define SS_PIN 10    // Slave Select Pin for RC522
#define SERVO_PIN 4  // Pin for Servo Motor
#define BUZZER_PIN 6

MFRC522 mfrc522(SS_PIN, RST_PIN);  // Create MFRC522 instance
MFRC522::MIFARE_Key key;           // Authentication key
Servo myServo;                     // Create Servo instance

void setup() {
  Serial.begin(9600);   // Initialize serial communication
  SPI.begin();          // Init SPI bus
  pinMode(BUZZER_PIN, OUTPUT);
  mfrc522.PCD_Init();   // Init MFRC522
  myServo.attach(SERVO_PIN); // Attach servo to pin
  // Prepare default authentication key (FFFFFFFFFFFF)
  for (byte i = 0; i < 6; i++) {
    key.keyByte[i] = 0xFF;
  }

  Serial.println("Place your RFID card near the reader...");
}

void loop() {
  // Look for new cards
  if (!mfrc522.PICC_IsNewCardPresent() || !mfrc522.PICC_ReadCardSerial()) {
    delay(50);
    return;
  }

  // The block to read (adjust if needed)
  byte blockAddr = 4;
  // Buffer for data
  byte buffer[18];
  byte size = sizeof(buffer);

  // Authenticate the sector
  MFRC522::StatusCode status = mfrc522.PCD_Authenticate(
    MFRC522::PICC_CMD_MF_AUTH_KEY_A,
    blockAddr,
    &key,
    &(mfrc522.uid)
  );

  if (status != MFRC522::STATUS_OK) {
    Serial.print("Authentication failed: ");
    Serial.println(mfrc522.GetStatusCodeName(status));
    mfrc522.PICC_HaltA();
    return;
  }

  // Read the block
  status = mfrc522.MIFARE_Read(blockAddr, buffer, &size);
  if (status != MFRC522::STATUS_OK) {
    Serial.print("Reading failed: ");
    Serial.println(mfrc522.GetStatusCodeName(status));
    mfrc522.PICC_HaltA();
    return;
  }

  // Check if the data contains "paid"
  if (buffer[0] == 'p' && buffer[1] == 'a' && buffer[2] == 'i' && buffer[3] == 'd') {
    Serial.println("Card paid, opening gate...");
    tone(BUZZER_PIN, 2000, 500);
    myServo.write(0);  // Rotate servo to 90 degrees
    delay(5000);        // Wait for servo to reach position
    myServo.write(90);
  } else {
    Serial.println("Unpaid card.");
    tone(BUZZER_PIN, 120, 500);
  }

  // Halt PICC and stop crypto
  mfrc522.PICC_HaltA();
  mfrc522.PCD_StopCrypto1();
}