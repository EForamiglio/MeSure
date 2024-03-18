#include <Wire.h>
#include <RTClib.h>
#include <EEPROM.h>

RTC_DS1307 rtc;
const int TAMANHO_EEPROM = 512;
const int NUM_LEITURAS = 5;
const int INICIO_EEPROM_ADDR = 0;
const int COMPRIMENTO_MAX_STRING = 17; // 14 caracteres para "DD/MM/YYYY HH:MM", mais 3 pela EEPROM
char buffer[COMPRIMENTO_MAX_STRING]; // Declaração do buffer corrigida

void setup() {
  Serial.begin(9600);
  Wire.begin();
  rtc.begin();

  // Inicia a EEPROM e limpa ela
  for (int i = 0; i < NUM_LEITURAS; i++) {
    escreverEEPROM(INICIO_EEPROM_ADDR + i * COMPRIMENTO_MAX_STRING, "00/00/0000 00:00");
  }
}

void loop() {
  DateTime agora = rtc.now();
  String dataHoraAtual = formatarDataHora(agora);

  // Atualiza a EEPROM
  atualizarEEPROM(dataHoraAtual);
  
  // Imprime a fila
  Serial.print("Fila: ");
  for (int i = 0; i < NUM_LEITURAS; i++) {
    String leitura = lerEEPROM(INICIO_EEPROM_ADDR + i * COMPRIMENTO_MAX_STRING);
    if (leitura != "00/00/0000 00:00") {
      Serial.print(leitura);
    }
    if (i < NUM_LEITURAS - 1 && leitura != "00/00/0000 00:00") {
      Serial.print(", ");
    }
  }
  Serial.println();
  
  delay(1000);
}

String formatarDataHora(DateTime dt) {
  char buffer[COMPRIMENTO_MAX_STRING];
  snprintf(buffer, COMPRIMENTO_MAX_STRING, "%02d/%02d/%04d %02d:%02d", dt.day(), dt.month(), dt.year(), dt.hour(), dt.minute());
  return String(buffer);
}

void atualizarEEPROM(String novaLeitura) {
  // Atualiza a EEPROM removendo a leitura mais antiga e inserindo a nova
  for (int i = NUM_LEITURAS - 1; i > 0; i--) {
    String leituraAnterior = lerEEPROM(INICIO_EEPROM_ADDR + (i - 1) * COMPRIMENTO_MAX_STRING);
    escreverEEPROM(INICIO_EEPROM_ADDR + i * COMPRIMENTO_MAX_STRING, leituraAnterior);
  }
  
  // Insere a nova leitura
  escreverEEPROM(INICIO_EEPROM_ADDR, novaLeitura);
}

void escreverEEPROM(int endereco, String dados) {
  dados.toCharArray(buffer, dados.length() + 1);
  for (int i = 0; i < COMPRIMENTO_MAX_STRING; i++) {
    EEPROM.write(endereco + i, buffer[i]);
  }
}

String lerEEPROM(int endereco) {
  for (int i = 0; i < COMPRIMENTO_MAX_STRING; i++) {
    buffer[i] = EEPROM.read(endereco + i);
  }
  buffer[COMPRIMENTO_MAX_STRING - 1] = '\0'; // Caractere que sinaliza o término da string
  return String(buffer);
}
