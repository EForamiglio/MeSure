#include "DHT.h"
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include "Button.h"

#define DHTPIN A1
#define DHTTYPE DHT11
#define tempoBuzzer 10

//sensores
LiquidCrystal_I2C lcd(0x27, 16, 2);
DHT dht(DHTPIN, DHTTYPE);

//media
float umidadeOld, temperaturaOld, intensidadeLuzOld;
float umidade, temperatura;
unsigned long tempo;

//alarme
int ledPin = 6;
int buzzerPin = 7;
int frequencia = 0;
int Pinofalante = 10;
float tempMin = 15;
float tempMax = 25;
float umiMin = 30;
float umiMax = 60;
float lumMin = 0;
float lumMax = 30;

//luz
int valorLDR;
int intensidadeLuz;
float pinoLDR = A0;

//botao
int buttonMode = 2;
int buttonMais = 3;
int buttonMenos = 4;
int buttonEscolha = 5;
bool editando = false;
int numEdicao = 0;
bool escolha = false;
unsigned long changeTime;
unsigned long altTime;
unsigned long choiceTime;

int inicio = 0;

void setup() {
  Serial.begin(9600);

  pinMode(ledPin, OUTPUT);
  pinMode(buzzerPin,OUTPUT);
  pinMode(pinoLDR, INPUT);
  pinMode(buttonMode, INPUT);
  pinMode(buttonMais, INPUT);
  pinMode(buttonMenos, INPUT);
  pinMode(buttonEscolha, INPUT);
  lcd.init();
  lcd.setBacklight(HIGH);

  dht.begin();

  intro();
}

void loop() {
   int modeState = digitalRead(buttonMode);
   int maisState = digitalRead(buttonMais);
   int menosState = digitalRead(buttonMenos);
   int escolhaState = digitalRead(buttonEscolha);
  
  if(inicio == 0){
	atualizaMedidas();
    printaValores(temperatura, umidade, intensidadeLuz);
    umidadeOld = umidade;
    temperaturaOld = temperatura;
	intensidadeLuzOld = intensidadeLuz;
    tempo = millis();
    inicio++;
  }
  
  if(modeState == HIGH && (millis() - changeTime) > 2000) {
	if(numEdicao == 0) {
    lcd.clear();
	  editando = true;
	  introEdicao();
	  numEdicao = 1;
	  delay(1000);
    lcd.clear();
	}
	if(numEdicao == 1) {
	  edicaoTemp();
	}
	if(numEdicao == 2) {
    lcd.clear();
	  edicaoUmi();
	}
	if(numEdicao == 3) {
    lcd.clear();
	  edicaoLum();
	}
	if(numEdicao == 4) {
    lcd.clear();
    introMedicao();
    delay(1000);
    lcd.clear();
	  printaValores(temperaturaOld, umidadeOld, intensidadeLuzOld);
	  editando = false;
	}
	numEdicao++;
	changeTime = millis();
  } else {
	 if(!editando) {
    numEdicao = 0;
    calculaMedia();
	 }
	 else {
		if(escolhaState == HIGH && (millis() - choiceTime) > 1000) {
			escolha = !escolha;
      choiceTime = millis();
		}
    int numEditar = numEdicao - 1;
		if(maisState == HIGH && (millis() - altTime) > 500) {
		  if(numEditar == 1) {
		   alterarTempMais();
		   atualizaTempEdicao();
	    }
	    if(numEditar == 2) {
		   alterarUmiMais();
		   atualizaUmiEdicao();
	    }
	    if(numEditar == 3) {
		   alterarLumMais();
		   atualizaLumEdicao();
	    }
        altTime = millis();
		}
    if(menosState == HIGH && (millis() - altTime) > 500) {
		  if(numEditar == 1) {
		   alterarTempMenos();
		   atualizaTempEdicao();
	      }
	      if(numEditar == 2) {
		   alterarUmiMenos();
		   atualizaUmiEdicao();
	      }
	      if(numEditar == 3) {
		   alterarLumMenos();
		   atualizaLumEdicao();
	      }
        altTime = millis();
		}

	 }
  }
}

void calculaMedia() {
  if(millis() - tempo == 15000) {
   tempo = millis();
	
   atualizaMedidas();

   float mediaTemp = (temperatura + temperaturaOld) / 2;
   float mediaUmd = (umidade + umidadeOld) / 2;
   float mediaLuz = (intensidadeLuz + intensidadeLuzOld) / 2;

   printaValores(mediaTemp, mediaUmd, mediaLuz);

   umidadeOld = mediaUmd;
   temperaturaOld = mediaTemp;
   intensidadeLuzOld = mediaLuz;
  }
}

void atualizaMedidas() {
   umidade = dht.readHumidity();
   temperatura = dht.readTemperature();
   valorLDR = analogRead(pinoLDR);
   intensidadeLuz = map(valorLDR, 0, 1023, 0, 100);
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

void introEdicao() {
  lcd.setCursor(0, 0);
  lcd.print("Entrando no");
  lcd.setCursor(0, 1);
  lcd.print("Modo Editor");
}
void introMedicao() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Entrando no");
  lcd.setCursor(0, 1);
  lcd.print("Modo Medicao");
}
void edicaoTemp() {
  lcd.setCursor(0, 0);
  lcd.print("Editar Temp");
  lcd.setCursor(0, 1);
  lcd.print("Vmax:   ");
  lcd.setCursor(5, 1);
  lcd.print(tempMax);
  lcd.print(" Vmin:  ");
  lcd.setCursor(13, 1);
  lcd.print(tempMin);
}
void edicaoUmi() {
  lcd.setCursor(0, 0);
  lcd.print("Editar Umi");
  lcd.setCursor(0, 1);
  lcd.print("Vmax:   ");
  lcd.setCursor(5, 1);
  lcd.print(umiMax);
  lcd.print(" Vmin:  ");
  lcd.setCursor(13, 1);
  lcd.print(umiMin);
}
void edicaoLum() {
  lcd.setCursor(0, 0);
  lcd.print("Editar Lum");
  lcd.setCursor(0, 1);
  lcd.print("Vmax:   ");
  lcd.setCursor(5, 1);
  lcd.print(lumMax);
  lcd.print(" Vmin:  ");
  lcd.setCursor(13, 1);
  lcd.print(lumMin);
}
void atualizaTempEdicao() {
  lcd.setCursor(5, 1);
  lcd.print(tempMax);
  lcd.setCursor(13, 1);
  lcd.print(tempMin);
}
void atualizaUmiEdicao() {
  lcd.setCursor(5, 1);
  lcd.print(umiMax);
  lcd.setCursor(13, 1);
  lcd.print(umiMin);
}
void atualizaLumEdicao() {
  lcd.setCursor(5, 1);
  lcd.print(lumMax);
  lcd.setCursor(13, 1);
  lcd.print(lumMin);
}
void alterarTempMais() {
  if(escolha) {
	tempMax++;
  } else {
	tempMin++;
  }
}
void alterarUmiMais() {
  if(escolha) {
	umiMax++;
  } else {
	umiMin++;
  }
}
void alterarLumMais() {
  if(escolha) {
	lumMax++;
  } else {
	lumMin++;
  }
}

void alterarTempMenos() {
  if(escolha) {
	tempMax--;
  } else {
	tempMin--;
  }
}
void alterarUmiMenos() {
  if(escolha) {
	umiMax--;
  } else {
	umiMin--;
  }
}
void alterarLumMenos() {
  if(escolha) {
	lumMax--;
  } else {
	lumMin--;
  }
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
  delay(1000);
  lcd.clear();
}
