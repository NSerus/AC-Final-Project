#define TEMPERATURA_AMBIENTE 15    //define a temperatura ambiente. essa será a temperatura atingida quando a resistência estiver muito tempo desligada
#define TEMPERATURA_MAXIMA 30      //define a temperatura máxima que se obtém quando a resistência estiver muito tempo ligada
#define CONSTANTE_TEMPO_SUBIDA 30  //tempo em segundos para a temperatura subir 63,21% do intervalo entre TEMPERATURA_MAXIMA e TEMPERATURA_AMBIENTE, partindo da TEMPERATURA_AMBIENTE
#define CONSTANTE_TEMPO_DESCIDA 60 //tempo em segundos para a temperatura descer 63,21% do intervalo entre TEMPERATURA_MAXIMA e TEMPERATURA_AMBIENTE, partindo da TEMPERATURA_MAXIMA
#define BOTAO_MODOS 2
#define BOTAO_MENOS 4
#define BOTAO_MAIS 3

float temp_referencia = 16;
float temperatura = TEMPERATURA_AMBIENTE;
unsigned long tempo_inicio = 0; //indica o tempo em que a resistência foi ligada ou desligada;
float tempo_base = 100000000;   //indica o tempo que demorou a atingir a temperatura atual. Inicialmente considera-se que a resistência estava desligada há muito, muito tempo
bool ligada = false;            //indica se a resistência está ligada ou desligada. Inicialmente está desligada

int modo;                 // modo do modo de estado
int freq_ruido = 100;     // frequencia de ruido eletronico
int dias = 0;
int multiplier;
int temperaturaLoopAnterior;

unsigned long tempo30segundos;      // tempo para o timer de 30 segundos
unsigned long ruido;                // para retirar a frequencia de ruido ao tempo atual
unsigned long freq_10hz = 0;        // frequencia de 10 hz para retirar de tempo atual
unsigned long tempo_pressionar = 0; // momento em que foi pressionado botao para adicionar tempo de espera
unsigned long contadorResistencia;  // Contador para o tempo de Resistencia ligada
unsigned long contadorResistenciaDesligada;
unsigned long tempo_total;
unsigned long tempo_dia;
  
bool mod4;               // se o modo 4 esta ativo
bool estadoBotaoModos;   // estado do Botao de modos  (guardar para loop seguinte)
bool estadoBotaoMais;    // estado do Botao Mais (guardar para loop seguinte)
bool estadoBotaoMenos;   // estado do Botao Menos (guardar para loop seguinte
bool BotaoModos;         // guardar estado atual de botao de modos
bool botaoMenos;         // guardar o estado atual  menos
bool botaoMais;          // guardar o estado atual mais
bool estadoRes;          // guardar o estado atual Resistencia
bool estadoModo;         // guardar o estado atual
bool botoes_modo4;       // guardar o estado atual
bool ativoBotao;         // quando um botao de mais ou menos esta ativo


unsigned long ti = 0;   //variável para controlar o tempo de envio de dados para a porta série
#define TEMPO_PRINT 500 //tempo entre envio de dados para a porta série

void liga_resistencia(bool modo) //se modo for true a resistência liga, senão desliga.(pino 13 NAO USAR)
{
  unsigned long t, t_atual;

  t_atual = millis();
  t = t_atual - tempo_inicio;
  tempo_inicio = t_atual;


  if (ligada)
    temperatura = TEMPERATURA_MAXIMA - (TEMPERATURA_MAXIMA - TEMPERATURA_AMBIENTE) * exp(-(t + tempo_base) / (CONSTANTE_TEMPO_SUBIDA * 1000.0));
  else
    temperatura = TEMPERATURA_AMBIENTE + (TEMPERATURA_MAXIMA - TEMPERATURA_AMBIENTE) * exp(-(t + tempo_base) / (CONSTANTE_TEMPO_DESCIDA * 1000.0));

  ligada = modo;
  if (ligada)
    tempo_base = -log(((float)(TEMPERATURA_MAXIMA - temperatura)) / (TEMPERATURA_MAXIMA - TEMPERATURA_AMBIENTE)) * CONSTANTE_TEMPO_SUBIDA * 1000.0;
  else
    tempo_base = -log(((float)(temperatura - TEMPERATURA_AMBIENTE)) / (TEMPERATURA_MAXIMA - TEMPERATURA_AMBIENTE)) * CONSTANTE_TEMPO_DESCIDA * 1000.0;
}

float get_temperatura() //esta função devolverá a temperatura no formato float com uma casa decimal.(NAO USAR A0)
{
  unsigned long t, t_atual;

  t_atual = millis();
  t = t_atual - tempo_inicio;

  if (ligada)
    temperatura = TEMPERATURA_MAXIMA - (TEMPERATURA_MAXIMA - TEMPERATURA_AMBIENTE) * exp(-(t + tempo_base) / (CONSTANTE_TEMPO_SUBIDA * 1000.0));
  else
    temperatura = TEMPERATURA_AMBIENTE + (TEMPERATURA_MAXIMA - TEMPERATURA_AMBIENTE) * exp(-(t + tempo_base) / (CONSTANTE_TEMPO_DESCIDA * 1000.0));

  return (round(temperatura * 100) / 100.0);
}


void BotoesOutrosModos(bool botao, bool estadoResistor, bool estadoBotao ){
   if(botao != estadoBotao) {  //se o botao+ estiver premido, ler uma unica vez ate mudar
      ativoBotao = true;   //fazer um modo 5 novo para usar estes botoes (fica inativo depois de 30 segundos)
      liga_resistencia(estadoResistor);    //resistencia fica ligada até mudar o modo
      tempo30segundos = millis();            //guardar o instante que o fotao foi premido
    }
}

void TempRefChanger(bool botao, int signal, bool estadoBotao) { 
  unsigned long t_atual = millis(); 
  if(botao != estadoBotao) {
    tempo_pressionar = t_atual;            //guardar o tempo atual para depois
    freq_10hz = t_atual+1000;
    if(t_atual-ruido >= freq_ruido) { //ruido e se botao manteu-se premido para n repetir este codigo
       temp_referencia += (signal*0.1); //referencia vai ser igual a 0,1 c o singal do botao premido
        ruido = t_atual;
      ruido += freq_ruido;           //adicionar a freq ruido para o if ali /\ 
    }
  }
  if(estadoBotao == botao) {          // se o botao continuar premido
    if(t_atual > 1000 + tempo_pressionar) {        //e passar 1 segundo desde o instante que o botao foi premido
      if((t_atual - freq_10hz) >= 100) {    //a cada 10 hz
        freq_10hz += 100;
        temp_referencia += (signal * 0.1);   //aumentar ref por 0,1 c o sinal do botao
      }
    }
  }  
}}

  //==========================================
  //Conversão do tempo
  //==========================================

void converteTempo(unsigned long t_atual) {
 
  unsigned long converteSegundos = t_atual / 1000;
  int segundos = converteSegundos % 60; // 1 segundo tem 60 milissegundos
  int minutos = converteSegundos / 60;  // 1 minuto tem 60 segundos
  int horas = converteSegundos / 3600;  // 1 hora tem 3600 segundos

  if (minutos > 60) {
    minutos = minutos - (60 * horas);
  }

  Serial.print(horas);
  Serial.print(":");
  Serial.print(minutos);
  Serial.print(":");
  Serial.print(segundos);
}



void setup() {
  Serial.begin(115200);
  pinMode(BOTAO_MODOS, INPUT_PULLUP);
  pinMode(BOTAO_MENOS, INPUT_PULLUP);
  pinMode(BOTAO_MAIS, INPUT_PULLUP);  
}



void loop() {
  tempo_total=millis();
  char ch; //variável para receber o byte da porta série
  botaoMenos = !digitalRead(BOTAO_MENOS);
  botaoMais = !digitalRead(BOTAO_MAIS);
  BotaoModos = !digitalRead(BOTAO_MODOS);
 
  
  //==========================================
  //MÁQUINA DE ESTADOS
  //==========================================

  if(ativoBotao==false){
    switch(modo) {                       // dependendo do modo
      case 0: //funciona       se 1
          mod4 = false;                   //dizer que o modo 4 não esta ativo (para mudar o uso dos botoes + e -)
          liga_resistencia(true);        // resistencia sempre ligada no modo 1
      break;
      case 1:                //se 2
          if(get_temperatura() > temp_referencia) {                  //manter temperatura na temp referencia
              liga_resistencia(false);
            } else if(get_temperatura() < temp_referencia) {
              liga_resistencia(true);
            }
      break;
      case 2:                //se 3
          liga_resistencia(false);           // desligar a resistencia sempre
      break;
      case 3:                //se 4
          mod4 = true;                       //ativar os botoes para modo 4
          if(botaoMais == HIGH){              //se botao+ for premido usar metodo acima descrito
              TempRefChanger(botaoMais, 1, estadoBotaoMais);
          }
          estadoBotaoMais = botaoMais;      //mudar estado no fim do metodo para saber se esta premido ou não

          if(botaoMenos == HIGH){            //se botao- for premido usar metodo acima descrito
              TempRefChanger(botaoMenos, -1, estadoBotaoMenos);
          }
          estadoBotaoMenos = botaoMenos;   //mudar estado no fim do metodo para saber se esta premido ou não
      break;
    }
  }

  
  //==========================================
  //Botões + e – noutros modos
  //==========================================
  
  if(mod4 == false) {            //se o modo 4 não estiver ativo
    if(botaoMais == HIGH) {
      BotoesOutrosModos(botaoMais, true, estadoBotaoMais);
    }else if(botaoMenos == HIGH) {
      BotoesOutrosModos(botaoMenos, false,estadoBotaoMenos);
    }
    if(millis() >= 3000 + tempo30segundos) {     //usando a variavel do instante,retira-se esse tempo, contando assim 30 segundos do 0
      ativoBotao = false;                //mudar para o modo que estava antes de o botao ser premido
    } 
    estadoBotaoMenos = botaoMenos;    //guardar o estado do botao mais para nao repetir no seguinte loop
    estadoBotaoMais = botaoMais;    //guardar o estado do botao mais para nao repetir no seguinte loop  
  }else { 
    ativoBotao = false;  //se o modo 4 estiver ativo desativar os botoes
  }
 
  
  //==========================================
  //MUDANÇA DE MODOS E RUIDO
  //==========================================

  //ruido como primeiro parametro
  if(BotaoModos == HIGH) {      //se botao de mudança ser premido
    if(BotaoModos != estadoBotaoModos) {
      if((millis() - ruido) >= freq_ruido) {  //passar ruido e apenas ser neste instante de ser premido
        ativoBotao = false;
        modo++;    //mudar modo
        // mudar para metodo
        if(modo == 4) { //se modo ser 4
            modo = 0;  // voltar para 0
        }
        ruido = millis();            //para iniciar o contador a 0 
        ruido += freq_ruido; 
      }
    }
  }
  estadoBotaoModos = BotaoModos; //estado do botao depois deste metodo (se ainda esta premido)

  
  //==========================================
  //Contador de resistencia
  //==========================================
  
  if(ligada == true){
    contadorResistencia = millis();
    contadorResistencia -= contadorResistenciaDesligada;
  }else if(ligada == false){
    contadorResistenciaDesligada = millis();
    contadorResistenciaDesligada -= contadorResistencia;
  }

  
  //==========================================
  //Contador de dia
  //==========================================
  tempo_dia = tempo_total - multiplier*86400000;
  if(tempo_total == 86400000){
    multiplier++;
  }
  
  
  //==========================================
  //Outputs
  //==========================================
  
  Serial.print("Modo: ");
  Serial.println(modo + 1);
  Serial.print("Temperatura: ");
  Serial.print(get_temperatura());
  Serial.print(" ");
  Serial.print("Temp. referencia: ");
  Serial.println(temp_referencia);
  Serial.print("Tempo da Resistencia ligada: ");
  converteTempo(contadorResistencia);
  Serial.print(" ");
  Serial.print("Tempo total ligado: ");
  converteTempo(tempo_dia);
  Serial.println();
  Serial.println();
  
  
  //==========================================
    
  if (millis() - ti >= TEMPO_PRINT) {
    ti += TEMPO_PRINT;
  }

  if (Serial.available()) { //se há bytes na porta série
    ch = Serial.read();     //lê-se o byte para a variável ch
    switch (ch) {
      case '0':             //dígito para desligar a resistência
        liga_resistencia(false);
        break;
      case '1':             //digito para ligar a resistência
        liga_resistencia(true);
        break;
    }
  }
}
