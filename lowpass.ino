#include <DueTimer.h>
#define PIN_INPUT A0    // pino analog-digital
#define PIN_OUTPUT DAC0 // pino digital-analog

const float ADC_CONV = 3.3f / 4095.0f;   // 12-bit ADC fator de conversão
const float DAC_CONV = 4095.0f / 3.3f;   // 12-bit DAC fator de conversão
volatile float prev_input = 0.0f;   // x(k-1)
volatile float prev_output = 0.0f;  // y(k-1)

// converte valores de inteiro em seu valor de tensão
inline float int_to_voltage(int signal) {
  return signal * ADC_CONV;
}

// converte valores de tensão em inteiros que se encaixam no intervalo 0-4095
inline int voltage_to_int(float voltage) {
  if (voltage <= 0.0f) return 0;
  if (voltage >= 3.3f) return 4095;
  return static_cast<int>(voltage * DAC_CONV + 0.5f);  // arrendonda pro mais próximo
}

// função responsável pela filtragem do sinal, conforme calculado, atualizado valores globais para próxima iteração
float filter(float signal) {
  // y(k) = b0*x(k) + b1*x(k-1) + a1*y(k-1)
  float output = (0.047679f * signal) + (0.047679f * prev_input) + (0.904761f * prev_output);
  
  prev_input = signal;
  prev_output = output;
  
  return output;
}

// esta função é chamada 1 vez a cada 10 kHz, lendo o pino analógico definido (A0), convertendo em valores de tensão,
// chamando a função de filtragem de sinal, convertendo em valores de inteiro e re-escrevendo o sinal na saída definida(DAC0)
void sample() {
  float realVal = int_to_voltage(analogRead(PIN_INPUT));
  float filteredVal = filter(realVal);
  analogWrite(PIN_OUTPUT, voltage_to_int(filteredVal));

  // debug, testando DAC1 para ver a frequência de chamada fica em 5 kHz (dac1_state precisa de 2x para contabilizar)
  static bool dac1_state = false;
  if (dac1_state) analogWrite(DAC1, 4095);  // output high (3.3V)
  else analogWrite(DAC1, 0);                // output low (0V)
  dac1_state = !dac1_state;
}

// esta função é executada 1x automaticamente durante a inicialização do arduino, 
void setup() {
  Serial.begin(115200);       // configura a taxa de transferência serial
  analogReadResolution(12);   // seta a resolução de leitura em 12 bits
  analogWriteResolution(12);  // seta a resolução de escrita em 12 bits
  Timer3.attachInterrupt(sample).start(100);  // cria a função de interrupção, chamada 1x a cada 10 kHz
  prev_input = prev_output = 0.0f;  // inicialização de variáveis
}

// esta função precisa estar presente, sendo executada indefinidamente pelo arduino, mas não possui código algum
void loop() { }
