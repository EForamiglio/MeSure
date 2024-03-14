#include "DHT.h"
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

#define DHTPIN A1
#define DHTTYPE DHT11

LiquidCrystal_I2C lcd(0x27, 16, 2);

DHT dht(DHTPIN, DHTTYPE);
float umidadeOld, temperaturaOld;
float umidade, temperatura;
unsigned long tempo;
int inicio = 0;

void setup() {
  Serial.begin(9600);

  lcd.init();
  lcd.setBacklight(HIGH);

  dht.begin();
}

// the loop function runs over and over again forever
void loop() {
  umidade = dht.readHumidity();
  temperatura = dht.readTemperature();

  if(inicio == 0){
    printaValores(temperatura, umidade);
    umidadeOld = umidade;
    temperaturaOld = temperatura;
    tempo = millis();
    inicio++;
  }
  
  if(millis() - tempo == 15000) {
    tempo = millis();

    float mediaTemp = (temperatura + temperaturaOld) / 2;
    float mediaUmd = (umidade + umidadeOld) / 2;

    printaValores(mediaTemp, mediaUmd);

    umidadeOld = umidade;
    temperaturaOld = temperatura;

    Serial.println("PrintaMedia");
  }
}

void printaValores(float temp, float umd) {
  lcd.setCursor(0, 0);
     lcd.print("U: ");
     lcd.print(umd);
     lcd.print(" %");
     lcd.setCursor(0, 1);
     lcd.print("T: ");
     lcd.print(temp);
     lcd.print(" C");
}
