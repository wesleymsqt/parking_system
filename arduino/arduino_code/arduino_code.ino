#include <SPI.h>
#include <MFRC522.h>
#include <Servo.h>

#define RST_PIN 9    // Redefinir pino para RC522
#define SS_PIN 10    // Pino de seleção escravo para RC522
#define SERVO_PIN 4  // Pino para Servo Motor
#define BUZZER_PIN 6

MFRC522 mfrc522(SS_PIN, RST_PIN);  // Criar instância MFRC522
MFRC522::MIFARE_Key key;           // Chave de autenticação
Servo myServo;                     // Criar instância Servo

void setup() {
  Serial.begin(9600);   // Iniciando comunicação serial
  SPI.begin();          // Barramento SPI de inicialização
  pinMode(BUZZER_PIN, OUTPUT);
  mfrc522.PCD_Init();   // Inicialização MFRC522
  myServo.attach(SERVO_PIN); // Fixe o servo ao pino
  // Preparar chave de autenticação padrão
  for (byte i = 0; i < 6; i++) {
    key.keyByte[i] = 0xFF;
  }

  Serial.println("Place your RFID card near the reader...");
}

void loop() {
  // Procure por novos cartões
  if (!mfrc522.PICC_IsNewCardPresent() || !mfrc522.PICC_ReadCardSerial()) {
    delay(50);
    return;
  }

  // O bloco a ser lido (ajuste se necessário)
  byte blockAddr = 4;
  byte buffer[18];
  byte size = sizeof(buffer);

  // Autentica o setor
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

  // Leia o bloco
  status = mfrc522.MIFARE_Read(blockAddr, buffer, &size);
  if (status != MFRC522::STATUS_OK) {
    Serial.print("Reading failed: ");
    Serial.println(mfrc522.GetStatusCodeName(status));
    mfrc522.PICC_HaltA();
    return;
  }

  // Verifique se os dados contêm "pago"
  if (buffer[0] == 'p' && buffer[1] == 'a' && buffer[2] == 'i' && buffer[3] == 'd') {
    Serial.println("Card paid, opening gate...");
    tone(BUZZER_PIN, 2000, 500);
    myServo.write(0);   // Girar servo em 90 graus
    delay(5000);        // Aguarde até que o servo alcance a posição
    myServo.write(90);
  } else {
    Serial.println("Unpaid card.");
    tone(BUZZER_PIN, 120, 500);
  }

  // Pare o PICC e pare a criptografia
  mfrc522.PICC_HaltA();
  mfrc522.PCD_StopCrypto1();
}