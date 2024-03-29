#include "DHT.h"
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <RTClib.h>
#include <EEPROM.h>

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
int tempMin = 15;
int tempMax = 25;
int umiMin = 30;
int umiMax = 60;
int lumMin = 0;
int lumMax = 30;

//luz
int valorLDR;
int intensidadeLuz;
float pinoLDR = A0;

//botao
int buttonMode = 13;
int buttonMais = 12;
int buttonMenos = 8;
int buttonEscolha = 2;
bool editando = false;
int numEdicao = 0;
bool escolha = false;
unsigned long changeTime;
unsigned long altTime;
unsigned long choiceTime;

//RTC
RTC_DS1307 rtc;
const int TAMANHO_EEPROM = 512;
const int NUM_LEITURAS = 5;
const int INICIO_EEPROM_ADDR = 0;
const int COMPRIMENTO_MAX_STRING = 39; // 14 caracteres para "DD/MM/YYYY HH:MM / ll%L / uu%U / tt.tC", mais 3 pela EEPROM
char buffer[COMPRIMENTO_MAX_STRING]; // Declaração do buffer corrigida
unsigned long rctTime;
String dataHoraAtual;
int valorEmerg = 0;

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
  
  rtc.begin();
   if (!rtc.isrunning()) {
    rtc.adjust(DateTime(__DATE__, __TIME__));
    delay(1000);
   }

  intro();
}

void loop() {
   DateTime agora = rtc.now();
   // dataHoraAtual = formatarDataHora(agora);
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
  
  if(modeState == HIGH && (millis() - changeTime) > 1000) {
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
      if(umidadeOld < umiMin || umidadeOld > umiMax || temperaturaOld < tempMin || temperaturaOld > tempMax || intensidadeLuzOld < lumMin || intensidadeLuzOld < lumMin || intensidadeLuzOld > lumMax) {
        acionaAlarme(modeState);
    dataHoraAtual = formatarDataHora(agora, intensidadeLuzOld, umidadeOld, temperaturaOld);
    valorEmerg++;
        // RTC
        if(millis() - rctTime > 4000 && valorEmerg == 1) {
          
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
        }
      } else {
    valorEmerg = 0;
  digitalWrite(ledPin, LOW);}
    }
    else {
    digitalWrite(ledPin, LOW);
      if(escolhaState == HIGH && (millis() - choiceTime) > 500) {
        escolha = !escolha;
        choiceTime = millis();
      }
      int numEditar = numEdicao - 1;
      if(maisState == HIGH && (millis() - altTime) > 500) {
        if(numEditar == 1) {
         alterarTempMais();
         edicaoTemp();
        }
        if(numEditar == 2) {
         alterarUmiMais();
         edicaoUmi();
        }
        if(numEditar == 3) {
         alterarLumMais();
         edicaoLum();
        }
          altTime = millis();
      }
      if(menosState == HIGH && (millis() - altTime) > 500) {
        if(numEditar == 1) {
         alterarTempMenos();
         edicaoTemp();
          }
          if(numEditar == 2) {
         alterarUmiMenos();
         edicaoUmi();
          }
          if(numEditar == 3) {
         alterarLumMenos();
         edicaoLum();
          }
          altTime = millis();
      }
    }
   }
  }

// Função para calcular a média das medidas de temperatura, umidade e luminosidade
void calculaMedia() {
  if(millis() - tempo > 3000) { // Verifica se passaram 15 segundos desde a última medição
    tempo = millis(); // Atualiza o tempo para o momento atual
    
    atualizaMedidas(); // Chama a função para atualizar as medidas
    
    // Calcula a média das medidas atuais e anteriores
    float mediaTemp = (temperatura + temperaturaOld) / 2;
    float mediaUmd = (umidade + umidadeOld) / 2;
    float mediaLuz = (intensidadeLuz + intensidadeLuzOld) / 2;
    
    // Exibe os valores médios no LCD
    printaValores(mediaTemp, mediaUmd, mediaLuz);
    
    // Atualiza as medidas antigas para as medidas atuais
    umidadeOld = mediaUmd;
    temperaturaOld = mediaTemp;
    intensidadeLuzOld = mediaLuz;

    // Serial.println(intensidadeLuzOld);
    // Serial.println(temperaturaOld);
    // Serial.println(umidadeOld);
  }
}

// Função para atualizar as medidas de umidade, temperatura e luminosidade
void atualizaMedidas() {
  // Lê a umidade e a temperatura do sensor DHT
  umidade = dht.readHumidity();
  temperatura = dht.readTemperature();
  
  // Lê a intensidade de luz do sensor LDR
  valorLDR = analogRead(pinoLDR);
  intensidadeLuz = map(valorLDR, 0, 1023, 0, 100); // Mapeia o valor lido para um intervalo de 0 a 100
  
  
}

// Função para acionar o alarme sonoro e visual
void acionaAlarme(int modeState) {
  // Gera um som crescente no buzzer e acende o LED
//  for (frequencia = 150; frequencia < 1800; frequencia += 1) {
//    tone(buzzerPin, frequencia, tempoBuzzer);
//    delay(1);
//  }
//  digitalWrite(ledPin, HIGH);
//  delay(10);
//  
//  // Gera um som decrescente no buzzer e apaga o LED
//  for (frequencia = 1800; frequencia > 150; frequencia -= 1) {
//    tone(buzzerPin, frequencia, tempoBuzzer);
//    delay(1);
//  }
  digitalWrite(ledPin, HIGH);
  tone(buzzerPin, 1500, tempoBuzzer);
  
  if(modeState == HIGH && (millis() - changeTime) > 1000) {
      lcd.clear();
      editando = true;
      introEdicao();
      numEdicao = 1;
      delay(1000);
      lcd.clear();
      changeTime = millis();
  }
}

// Função para imprimir os valores de temperatura, umidade e luminosidade no LCD
void printaValores(float temp, float umd, int lum) {
  lcd.setCursor(0, 0); // Define o cursor na posição (0, 0) do LCD
  lcd.print("U:"); // Imprime "U:" na primeira linha do LCD
  lcd.print(umd, 1); // Imprime o valor da umidade com uma casa decimal
  lcd.print("%"); // Imprime o símbolo de porcentagem
  lcd.setCursor(10, 0); // Define o cursor na posição (10, 0) do LCD
  lcd.print("L:"); // Imprime "L:" na primeira linha do LCD
  lcd.print(lum); // Imprime o valor da luminosidade
  lcd.print("%"); // Imprime o símbolo de porcentagem
  lcd.setCursor(0, 1); // Define o cursor na posição (0, 1) do LCD
  lcd.print("T:"); // Imprime "T:" na segunda linha do LCD
  lcd.print(temp, 1); // Imprime o valor da temperatura com uma casa decimal
  lcd.print("C"); // Imprime o símbolo de graus Celsius
}



///Edição de variáveis
// Função para exibir uma mensagem de introdução ao entrar no modo de edição
void introEdicao() {
  lcd.setCursor(0, 0); // Define o cursor na posição (0, 0) do LCD
  lcd.print("Entrando no"); // Imprime "Entrando no" na primeira linha do LCD
  lcd.setCursor(0, 1); // Define o cursor na posição (0, 1) do LCD
  lcd.print("Modo Editor"); // Imprime "Modo Editor" na segunda linha do LCD
}

// Função para exibir uma mensagem de introdução ao entrar no modo de medição
void introMedicao() {
  lcd.clear(); // Limpa o conteúdo do LCD
  lcd.setCursor(0, 0); // Define o cursor na posição (0, 0) do LCD
  lcd.print("Entrando no"); // Imprime "Entrando no" na primeira linha do LCD
  lcd.setCursor(0, 1); // Define o cursor na posição (0, 1) do LCD
  lcd.print("Modo Medicao"); // Imprime "Modo Medicao" na segunda linha do LCD
}

// Função para exibir a tela de edição de temperatura no LCD
void edicaoTemp() {
  lcd.setCursor(0, 0); // Define o cursor na posição (0, 0) do LCD
  lcd.print("Editar Temp"); // Imprime "Editar Temp" na primeira linha do LCD
  lcd.setCursor(0, 1); // Define o cursor na posição (0, 1) do LCD
  lcd.print("Vmax:   "); // Imprime "Vmax:   " na segunda linha do LCD
  lcd.setCursor(5, 1); // Define o cursor na posição (5, 1) do LCD
  lcd.print(tempMax); // Imprime o valor de tempMax na segunda linha do LCD
  lcd.print(" Vmin:  "); // Imprime " Vmin:  " na segunda linha do LCD
  lcd.setCursor(13, 1); // Define o cursor na posição (13, 1) do LCD
  lcd.print(tempMin); // Imprime o valor de tempMin na segunda linha do LCD
}

// Função para exibir a tela de edição de umidade no LCD
void edicaoUmi() {
  lcd.setCursor(0, 0); // Define o cursor na posição (0, 0) do LCD
  lcd.print("Editar Umi"); // Imprime "Editar Umi" na primeira linha do LCD
  lcd.setCursor(0, 1); // Define o cursor na posição (0, 1) do LCD
  lcd.print("Vmax:   "); // Imprime "Vmax:   " na segunda linha do LCD
  lcd.setCursor(5, 1); // Define o cursor na posição (5, 1) do LCD
  lcd.print(umiMax); // Imprime o valor de umiMax na segunda linha do LCD
  lcd.print(" Vmin:  "); // Imprime " Vmin:  " na segunda linha do LCD
  lcd.setCursor(13, 1); // Define o cursor na posição (13, 1) do LCD
  lcd.print(umiMin); // Imprime o valor de umiMin na segunda linha do LCD
}

// Função para exibir a tela de edição de luminosidade no LCD
void edicaoLum() {
  lcd.setCursor(0, 0); // Define o cursor na posição (0, 0) do LCD
  lcd.print("Editar Lum"); // Imprime "Editar Lum" na primeira linha do LCD
  lcd.setCursor(0, 1); // Define o cursor na posição (0, 1) do LCD
  lcd.print("Vmax:   "); // Imprime "Vmax:   " na segunda linha do LCD
  lcd.setCursor(5, 1); // Define o cursor na posição (5, 1) do LCD
  lcd.print(lumMax); // Imprime o valor de lumMax na segunda linha do LCD
  lcd.print(" Vmin:  "); // Imprime " Vmin:  " na segunda linha do LCD
  lcd.setCursor(13, 1); // Define o cursor na posição (13, 1) do LCD
  lcd.print(lumMin); // Imprime o valor de lumMin na segunda linha do LCD
}

// Funções para aumentar os valores de temperatura, umidade e luminosidade
void alterarTempMais() {
  if(escolha) { // Se escolha for verdadeiro
    tempMax++; // Incrementa tempMax
  } else {
    tempMin++; // Caso contrário, incrementa tempMin
  }
}

void alterarUmiMais() {
  if(escolha) { // Se escolha for verdadeiro
    umiMax++; // Incrementa umiMax
  } else {
    umiMin++; // Caso contrário, incrementa umiMin
  }
}

void alterarLumMais() {
  if(escolha) { // Se escolha for verdadeiro
    lumMax++; // Incrementa lumMax
  } else {
    lumMin++; // Caso contrário, incrementa lumMin
  }
}

// Funções para diminuir os valores de temperatura, umidade e luminosidade
void alterarTempMenos() {
  if(escolha) { // Se escolha for verdadeiro
    tempMax--; // Decrementa tempMax
  } else {
    tempMin--; // Caso contrário, decrementa tempMin
  }
}

void alterarUmiMenos() {
  if(escolha) { // Se escolha for verdadeiro
    umiMax--; // Decrementa umiMax
  } else {
    umiMin--; // Caso contrário, decrementa umiMin
  }
}

void alterarLumMenos() {
  if(escolha) { // Se escolha for verdadeiro
    lumMax--; // Decrementa lumMax
  } else {
    lumMin--; // Caso contrário, decrementa lumMin
  }
}

///RTC

// Método para formatar uma data e hora em uma string
String formatarDataHora(DateTime dt, int lum, float umd, float temp) {
  // Define um buffer para armazenar a data e hora formatadas
  char buffer[COMPRIMENTO_MAX_STRING];
  // Usa snprintf para formatar a data e hora no buffer // 14 caracteres para "DD/MM/YYYY HH:MM / ll%L / uu%U / tt.tC", mais 3 pela EEPROM
  snprintf(buffer, COMPRIMENTO_MAX_STRING, "%02d/%02d/%04d %02d:%02d / %02dL / %02dU / %02dC", dt.day(), dt.month(), dt.year(), dt.hour(), dt.minute(), lum, (int)umd, (int)temp);
  // Retorna a data e hora formatadas como uma string
  return String(buffer);
}

// Método para atualizar os dados na EEPROM com uma nova leitura
void atualizarEEPROM(String novaLeitura) {
  // Atualiza a EEPROM removendo a leitura mais antiga e inserindo a nova
  for (int i = NUM_LEITURAS - 1; i > 0; i--) {
    // Lê a leitura anterior da EEPROM
    String leituraAnterior = lerEEPROM(INICIO_EEPROM_ADDR + (i - 1) * COMPRIMENTO_MAX_STRING);
    // Escreve a leitura anterior em uma posição à frente na EEPROM
    escreverEEPROM(INICIO_EEPROM_ADDR + i * COMPRIMENTO_MAX_STRING, leituraAnterior);
  }
  
  // Insere a nova leitura no início da EEPROM
  escreverEEPROM(INICIO_EEPROM_ADDR, novaLeitura);
}

// Método para escrever dados na EEPROM a partir de um determinado endereço
void escreverEEPROM(int endereco, String dados) {
  // Converte a string de dados em um array de caracteres (buffer)
  dados.toCharArray(buffer, dados.length() + 1);
  // Escreve os caracteres um a um na EEPROM a partir do endereço especificado
  for (int i = 0; i < COMPRIMENTO_MAX_STRING; i++) {
    EEPROM.write(endereco + i, buffer[i]);
  }
}

// Método para ler dados da EEPROM a partir de um determinado endereço
String lerEEPROM(int endereco) {

  // Cria um buffer para armazenar os dados lidos da EEPROM
  char buffer[COMPRIMENTO_MAX_STRING];
  // Lê os caracteres um a um da EEPROM e os armazena no buffer
  for (int i = 0; i < COMPRIMENTO_MAX_STRING; i++) {
    buffer[i] = EEPROM.read(endereco + i);
  }
  // Retorna os dados lidos da EEPROM como uma string
  return String(buffer);
}

///Introdução
void intro(){
  int i=4; // Inicializa uma variável 'i' com o valor 4
  int line=0; // Inicializa uma variável 'line' com o valor 0

  // Define um array de bytes representando um caractere personalizado de uma régua vazia
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

  // Define um array de bytes representando um caractere personalizado de uma régua cheia
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

  // Cria os caracteres personalizados na posição 0 e 1 da memória do LCD
  lcd.createChar(0, reguaBaixa);
  lcd.createChar(1, regua);
  
  // Mostra a animação "MeSure" movendo-se para a direita na primeira linha do LCD
  for(int i=0;i<6;i++){
    lcd.clear();
    lcd.setCursor(i,0);
    lcd.print("MeSure");
    delay(250);
  }
  
  // Mostra a régua de progresso na segunda linha do LCD
  while(i<13){ // Executa enquanto 'i' for menor que 13
    lcd.setCursor(i,1); // Define o cursor na posição 'i', segunda linha
    lcd.write(byte(line)); // Escreve o caractere personalizado correspondente à linha 'line'
    
    i++; // Incrementa 'i'
    
    if(i==12 && line==1){ // Se 'i' for igual a 12 e 'line' for igual a 1
      break; // Sai do loop
    }
    else if(i==12){ // Se 'i' for igual a 12
      delay(250); // Espera 250ms
      i=4; // Reinicializa 'i' para 4
      line++; // Incrementa 'line'
    }
  }
  
  delay(1000); // Espera 1 segundo
  lcd.clear(); // Limpa o LCD
  lcd.print("CARREGANDO..."); // Mostra "CARREGANDO..." no LCD
  delay(1000); // Espera 1 segundo
  lcd.clear(); // Limpa o LCD
}
