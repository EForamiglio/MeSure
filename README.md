# MeSure

## Objective
Having in sight the advancement of globalization and intense bussiness abroad, a certain difficulty to ensure the quality and integrity of products that needs special conditions emmerge. To resolve this, MeSure build a device to monitor temperature, humidity and luminosity disponsing then in a lcd display and when the measures exceed a pre set value, it warnings by led and sound alarms.  

## Objetivo
Visto que estamos inseridos em um mundo globalizado e de intenso tráfego de grandes distâncias, surge assim a dificuldade de garantir a qualidade e a integridade de produtos que demandam condições especiais. Pensando nisso, nós da MeSure, desenvolvemos um dispositivo capaz de monitorar temperatura, umidade e luminosidade ambiente, dispondo essas informações em display e alertando caso preciso através de sinais luminosos e sonoros.

## Requisitos de Hardware

> 1 Arduino UNO R3
> Sensores de luminosidade (LDR), temperatura e umidade (DHT11).
> 1 Protoboard
> 1 LCD 16x2 - I2C
> 1 RTC (Real Time Clock)
> Componentes adicionais: 2 resistores, 20 cabos de ligação, 1 LED, 1 Buzzer
> 1 Bateria de 9V e suporte para bateria

## Requisitos de Software

- Programa embarcado para o Arduino que permita a leitura dos sensores, comparação dos valores de leitura dos sensores com parâmetros pré estabelecidos, obtenção de marcações de tempo do RTC e o armazenamento dos dados coletados na EEPROM
- Algoritmos para leitura dos sensores e conversão das leituras analógicas em valores de umidade, luminosidade e temperatura.
- Interface de usuário simples para mostrar os valores medidos de cada variável.
- Lógica para gerenciar o espaço disponível na EEPROM e garantir que os dados mais antigos sejam substituídos conforme necessário.
- Armazenamento dos dados coletados na EEPROM do Arduino, utilizando um formato adequado para representar os valores de umidade, luminosidade, temperatura e tempo.
- Capacidade de gerenciar o armazenamento de dados de forma eficiente, evitando a perda de informações importantes devido a problemas de memória ou falhas de energia. 

## Arquitetura do Sistema

Através do Firmware do Arduino, é feita a leitura das variáveis captadas pelos sensores, a média das leituras feitas em um minuto é mostrada no display para o usuário e os valores são comparados aos parâmetros estabelecidos no código, caso uma das variáveis exceder os parâmetros ocorre a captura das informações de data e hora pelo RTC.
Por fim são armazenados os dados na EEPROM do Arduino, substituindo os dados mais antigos para evitar o estouro de memória através de uma lista.
O firmware pode fazer uso de bibliotecas específicas para interagir com os sensores, as que foram utilizadas para o projeto são: "DHT", "Wire", "LiquidCrystal_I2C", "RTClib", "EEprom" 
As principais interconexões feitas para o sistema funcionar foram as conexões físicas entre o Arduino e os sensores para aquisição dos dados. A conexão entre Arduino e RTC para marcação de tempo. Uso das portas de entrada e saída para acessar a EEPROM e armazenar os dados coletados.

## Descrição de Hardware

- O projeto utiliza-se de uma placa de Arduino UNO R3. Seu microcontrolador é o ATmega328P da Microchip, com uma velocidade de clock de 16 MHz e 32 KB de memória flash, dos quais 0.5 KB são usados pelo bootloader. Possui uma interface USB que permite a conexão com um computador para programação e comunicação serial. Ele também dispõe de 14 pinos digitais de entrada/saída. Possui uma pequena memória EEPROM (Electrically Erasable Programmable Read-Only Memory) de 1 KB para armazenamento de dados não volátil, que será utilizada pelo projeto para aguardar os últimos 5 registros que tiverem uma das variáveis fora dos parâmetros, bem como a data e a hora do evento.
- Sua fonte de alimentação é via porta USB do computador ou com um adaptador de alimentação externo (7-12V), conectado ao jack de alimentação. O programa que utilizamos para programar os componentes foi a própria IDE (Integrated Development Environment) do Arduino, que utiliza uma linguagem de programação baseada em C/C++ simplificada.
- O sensor de temperatura e umidade utilizado foi o DHT11. Na prática, ele detecta a umidade e a temperatura, enviando estas informações para a placa microcontroladora, que deve estar programada para realizar alguma ação quando atingida determinada temperatura ou umidade. No projeto, é por meio da placa Arduino, onde é possível programar a placa microcontroladora para ligar, o LED e o Buzzer quando o ambiente atingir determinada temperatura ou nível de umidade. Ele possui algumas especificações, tais como: tensão de alimentação de 3 ~ 5,5 Vdc, faixa de corrente de 0,5 ~ 2,5 mA, faixa de operação de umidade de 20 ~ 90%, faixa de operação da temperatura de 0 ~ 50° celsius.
- O sensor de luminosidade utilizado é o LDR (Light Dependent Resistor) é um componente cuja resistência varia de acordo com a intensidade da luz. Quanto mais luz incidir sobre o componente, menor a resistência. Este sensor de luminosidade pode ser utilizado em projetos com arduino e outros microcontroladores para alarmes, automação residencial, sensores de presença e etc. Algumas de suas especificações são: tensão máxima de 150VDC, potência máxima de 100mW, resistência no escuro de 1 MΩ (Lux 0) e resistência na luz de 10-20 KΩ (Lux 10).
- Módulo Real Time Clock RTC DS3231 utilizado atua de forma que os endereços e dados sejam transferidos em série através de um barramento bidirecional I2C, e pode ser utilizado com arduino e com os demais microcontroladores. Ele fornece um relógio de precisão em tempo real (RTC) e opera junto à um cristal oscilador compensado por temperatura (TCXO) e cristal. Além disso, o módulo possui uma entrada de bateria de forma a manter a cronometragem precisa quando a alimentação principal do dispositivo é interrompida.
- O RTC opera com segundos, minutos, horas, dia, data, informações de mês e ano. A data no final do mês é ajustada automaticamente para meses com menos de 31 dias e inclui correções para anos bissextos. O relógio funciona no formato de 24 horas ou de 12 horas com um indicador AM/PM. Suas especificações são: tensão de operação VCC: 2.3VCC à 5.5VCC, chip de memória: AT24C32, contagem de segundos, minutos, horas, Data do mês, mês, dia da semana e Ano, com compensação de ano bissexto válido até 2100 e interface serial I2C de alta velocidade (400 KHZ).

## Detalhes de Funcionamento
