#include "DHT.h"
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

#define DHTPIN A1
#define DHTTYPE DHT11
#define tempoBuzzer 10

LiquidCrystal_I2C lcd(0x27, 16, 2);

DHT dht(DHTPIN, DHTTYPE);
float umidadeOld, temperaturaOld;
float umidade, temperatura;
unsigned long tempo;
int inicio = 0;
int ledPin = 6;
int buzzerPin = 7;
int frequencia = 0;
int Pinofalante = 10;
int valorLDR;
int intensidadeLuz;
float pinoLDR = A0;

void setup() {
  Serial.begin(9600);

  pinMode(ledPin, OUTPUT);
  pinMode(buzzerPin,OUTPUT);
  pinMode(pinoLDR, INPUT);
  lcd.init();
  lcd.setBacklight(HIGH);

  dht.begin();

  intro();
}

void loop() {
  umidade = dht.readHumidity();
  temperatura = dht.readTemperature();
  valorLDR = analogRead(pinoLDR);
  intensidadeLuz = map(valorLDR, 0, 1023, 0, 100);

  if(inicio == 0){
    printaValores(temperatura, umidade, intensidadeLuz);
    umidadeOld = umidade;
    temperaturaOld = temperatura;
    tempo = millis();
    inicio++;
  }

  if (umidade < 30 || umidade > 60 || temperatura < 15 || temperatura > 25 || intensidadeLuz == 0 || intensidadeLuz > 30) {
//    acionaAlarme(); 
  }

  if(millis() - tempo == 15000) {
    tempo = millis();

    float mediaTemp = (temperatura + temperaturaOld) / 2;
    float mediaUmd = (umidade + umidadeOld) / 2;

    printaValores(mediaTemp, mediaUmd, intensidadeLuz);

    umidadeOld = umidade;
    temperaturaOld = temperatura;

    Serial.println("PrintaMedia");
  }
}

void acionaAlarme() {
  for (frequencia = 150; frequencia < 1800; frequencia += 1)
  {
    tone(buzzerPin, frequencia, tempoBuzzer);
    delay(1);
  }
  digitalWrite(ledPin, HIGH);
    delay(10);
  for (frequencia = 1800; frequencia > 150; frequencia -= 1)
  {
    tone(buzzerPin, frequencia, tempoBuzzer);
    delay(1);
  }
  digitalWrite(ledPin, LOW);
  delay(10);
}

void printaValores(float temp, float umd, int lum) {
  lcd.setCursor(0, 0);
  lcd.print("U:");
  lcd.print(umd, 1);
  lcd.print("%");
  lcd.setCursor(10, 0);
  lcd.print("L:");
  lcd.print(lum);
  lcd.print("%");
  lcd.setCursor(0, 1);
  lcd.print("T:");
  lcd.print(temp, 1);
  lcd.print("C");
}

void intro(){
  int i=4;
  int line=0;

  byte regua[] = {
  B10101,
  B11101,
  B11111,
  B00000,
  B00000,
  B00000,
  B00000,
  B00000

};

byte reguaBaixa[] = {
  B00000,
  B00000,
  B00000,
  B00000,
  B00000,
  B10101,
  B11101,
  B11111
};


  lcd.createChar(0, reguaBaixa);
  lcd.createChar(1,regua);
  
  
  for(int i=0;i<6;i++){
    lcd.clear();
    lcd.setCursor(i,0);
    lcd.print("MeSure");
    delay(250);
  }
  
  while(i<13){
    lcd.setCursor(i,1);
    lcd.write(byte(line));
    
    i++;
    
    if(i==12 && line==1){
     break;
    }
    else if(i==12){
      delay(250);
      i=4;
      line++;
    }
  }
  
  delay(1000);
  lcd.clear();
  lcd.print("CARREGANDO...");
}
