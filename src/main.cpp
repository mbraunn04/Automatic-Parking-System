// ---------------------------------------------------------------------------------- //
// PROJETO UERGS PORTAS ABERTAS - SISTEMA DE ESTACIONAMENTO AUTOMÁTICO COM ARDUINO    //
// ---------------------------------------------------------------------------------- //

#include <Servo.h>
#include <Arduino.h>

Servo cancela; // Cria um objeto do tipo Servo para controlar um servo motor

// Definindo os pinos dos sensores e do servo motor
#define pin_sensor1_cancela A0
#define pin_sensor2_cancela A1
#define pin_cancela 13
#define TOTAL_SENSORES 5
const int pins_sensores_vagas[TOTAL_SENSORES] = {2, 3, 4, 5, 6}; // Pinos dos 5 sensores de obstáculo das vagas
const int pins_LEDs[TOTAL_SENSORES] = {7, 8, 9, 10, 11}; // Pinos dos 5 sensores de LEDs das vagas

// Declaração de variáveis globais
int vagasLivres = 0;
bool cancelaAberta = false;
int sts_cancela_ent = 0, sts_cancela_saida = 0;
int sts_vagas[TOTAL_SENSORES];

unsigned long tempoAnterior = 0; // Controle de Tempo com millis()
const unsigned long INTERVALO_LEITURA = 500; // Atualiza a cada 500ms (0,5s)

int vagasAnterior = -1; // Armazena o estado anterior para só avisar quando o número de vagas MUDAR

// Protótipos de funções
void abrirCancela();
void fecharCancela();
int controleVagas();

// --------------------------------------------------------------------- //
// SETUP                                                                 //
// --------------------------------------------------------------------- //

void setup() {

  Serial.begin(9600);
  
  // Definindo os pinos dos sensores de obstáculo como ENTRADAS
  for (int i = 0; i < TOTAL_SENSORES; i++) {
    pinMode(pins_sensores_vagas[i], INPUT);
  }
  pinMode(pin_sensor1_cancela, INPUT);
  pinMode(pin_sensor2_cancela, INPUT);

  // Definindo os pinos dos LEDs como SAÍDAS
  for (int i = 0; i < TOTAL_SENSORES; i++) {
    pinMode(pins_LEDs[i], OUTPUT);
  }

  // Associando o pino do servo ao objeto do servo
  cancela.attach(pin_cancela);

}

// --------------------------------------------------------------------- //
// LOOP                                                                  //
// --------------------------------------------------------------------- //

void loop() {

  unsigned long tempoAtual = millis();

  // LEITURA PERIÓDICA DAS VAGAS (A cada 500ms)
  if (tempoAtual - tempoAnterior >= INTERVALO_LEITURA) {
    tempoAnterior = tempoAtual;

    // Executa a varredura chamando a função
    vagasLivres = controleVagas();

    // Só imprime/envia se houver mudança nas vagas
    if (vagasLivres != vagasAnterior) {
      Serial.println("====================================");
      Serial.print("TOTAL DE VAGAS LIVRES: ");
      Serial.print(vagasLivres);
      Serial.print("/");
      Serial.println(TOTAL_SENSORES);
      Serial.println("------------------------------------");

      // Imprime o status individual de cada vaga
      for (int i = 0; i < TOTAL_SENSORES; i++) {
        Serial.print("Vaga ");
        Serial.print(i + 1);
        Serial.print(": ");

        if (sts_vagas[i] == 1) {
          Serial.println("[ LIVRE ]");
        } else {
          Serial.println("[ OCUPADA ]");
        }
      }
      Serial.println("====================================\n");

      vagasAnterior = vagasLivres; // Atualiza a referência
    }
  }
  
  // LÓGICA DE CONTROLE DA CANCELA
  // Leitura dos sensores da cancela
  sts_cancela_ent = digitalRead(pin_sensor1_cancela);
  sts_cancela_saida = digitalRead(pin_sensor2_cancela);

  // ABRIR CANCELA
  if (cancelaAberta == false) {
    // Caso A: Carro entrando (precisa ter vaga livre)
    if (digitalRead(pin_sensor1_cancela) == 0 && vagasLivres > 0) {
      abrirCancela();
      cancelaAberta = true;
    }
    // Caso B: Carro saindo do estacionamento
    else if (digitalRead(pin_sensor2_cancela) == 0) {
      abrirCancela();
      cancelaAberta = true;
    }
  }

  // FECHAR CANCELA
  if (cancelaAberta == true) {

    if (digitalRead(pin_sensor1_cancela) == 1 && digitalRead(pin_sensor2_cancela) == 1) { // Só fecha a cancela depois que o carro passar pelos dois sensores
      delay(1000); // Aguarda o carro terminar de passar totalmente

      if (digitalRead(pin_sensor1_cancela) == 1 && digitalRead(pin_sensor2_cancela) == 1) { // verificação de segurança - evitar fechar a cancela no carro da traseira
        fecharCancela();
        cancelaAberta = false;
      } 
    }
  }
}

// --------------------------------------------------------------------- //
// IMPLEMENTAÇÃO COMPLETA DAS FUNÇÕES                                    //
// --------------------------------------------------------------------- //

// Função para controlar o servo e abrir a cancela
void abrirCancela() {
  for (int pos = 0; pos <= 90; pos += 1) { // move a cancela de 0 graus para 90 graus
    cancela.write(pos);              
    delay(15);                       
  }
}

// Função para controlar o servo e fechar a cancela
void fecharCancela() {
  for (int pos = 90; pos >= 0; pos -= 1) { // move a cancela de 90 graus para 0 graus
    cancela.write(pos);              
    delay(15);                       
  }
}

int controleVagas() {
  int contadorVagas = 0;

  for (int i = 0; i < TOTAL_SENSORES; i++) {
    int estadoSensor = digitalRead(pins_sensores_vagas[i]);

    if (estadoSensor == HIGH) { // Sensor em HIGH = VAGA LIVRE
      contadorVagas++;
      sts_vagas[i] = 1; // 1 representa LIVRE
      digitalWrite(pins_LEDs[i], LOW); // Apaga LED indicativo
    } else { // Sensor em LOW = VAGA OCUPADA
      sts_vagas[i] = 0; // 0 representa OCUPADA
      digitalWrite(pins_LEDs[i], HIGH); // Acende LED indicativo
    }
  }

  return contadorVagas;
}


