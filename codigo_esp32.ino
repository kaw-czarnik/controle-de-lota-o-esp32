#include <Arduino.h>
#include <WiFi.h>
#include <Wire.h>
#include <Preferences.h>

// Configuração de rede
const char *ssid     = "xxxx";
const char *password = "xxxxxx";
WiFiServer server(80);

// Configuração Sensores ultrassônico
// Sensor 1 - Entrada
const int PINO_TRIG1 = 27; 
const int PINO_ECHO1 = 26;

// Sensor 2 - lado de saída
const int PINO_TRIG2 = 25; 
const int PINO_ECHO2 = 33;

// LED dos ESP32 para status de conexão
const int PINO_LED   = 2;

// Variáveis globais
int entrada = 0;                  // Contagem de pessoas dentro
float distancia1 = 0;             // Distância lida pelo sensor 1
float distancia2 = 0;             // Distância lida pelo sensor 2
unsigned long ultimaLeitura = 0;  // Controle de tempo da última leitura

unsigned long tempoS1 = 0;        // Momento em que S1 detectou algo
unsigned long tempoS2 = 0;        // Momento em que S2 detectou algo
unsigned long ultimaContagem = 0; // Momento da última contagem registrada
const unsigned long COOLDOWN = 1000; // Tempo mínimo entre contagens (ms)

bool wifiConectado = false;

void medirDistancia1() {
  digitalWrite(PINO_TRIG1, LOW);
  delayMicroseconds(2);
  digitalWrite(PINO_TRIG1, HIGH);
  delayMicroseconds(10);
  digitalWrite(PINO_TRIG1, LOW);

  long duracao1 = pulseIn(PINO_ECHO1, HIGH, 30000);
  distancia1 = (duracao1 * 0.0343) / 2;
}

void medirDistancia2() {
  digitalWrite(PINO_TRIG2, LOW);
  delayMicroseconds(2);
  digitalWrite(PINO_TRIG2, HIGH);
  delayMicroseconds(10);
  digitalWrite(PINO_TRIG2, LOW);

  long duracao2 = pulseIn(PINO_ECHO2, HIGH, 30000);
  distancia2 = (duracao2 * 0.0343) / 2;
}

// Função para salvar contagem na memória
void salvarContagem() {
  Preferences prefs;
  prefs.begin("lotacao", false);
  prefs.putInt("entrada", entrada);
  prefs.end();
}

void carregarContagem() {
  Preferences prefs;
  prefs.begin("lotacao", true);
  entrada = prefs.getInt("entrada", 0);
  prefs.end();
}

void verificarPassagem() {
  bool detectadoS1 = (distancia1 > 0 && distancia1 < 50);
  bool detectadoS2 = (distancia2 > 0 && distancia2 < 50);

  // Registra o momento da primeira detecção de cada sensor
  if (detectadoS1 && tempoS1 == 0) tempoS1 = millis();
  if (detectadoS2 && tempoS2 == 0) tempoS2 = millis();

  // Se só um sensor foi ativado e passou 3 segundos, reseta
  if (tempoS1 > 0 && tempoS2 == 0 && millis() - tempoS1 > 1000) tempoS1 = 0;
  if (tempoS2 > 0 && tempoS1 == 0 && millis() - tempoS2 > 1000) tempoS2 = 0;

  // Se os dois sensores foram ativados, analisa a direção
  if (tempoS1 > 0 && tempoS2 > 0) {
    unsigned long diferenca = abs((long)tempoS1 - (long)tempoS2);

    // Se ativaram quase juntos (menos de 50ms), ignora — pode ser ruído
    if (diferenca < 50) {
      tempoS1 = 0;
      tempoS2 = 0;
      return;
    }

    // Só registra se passou o tempo de cooldown
    if (millis() - ultimaContagem >= COOLDOWN) {
      if (tempoS1 < tempoS2) {
        entrada += 1;
        Serial.println("Entrada registrada. Total: " + String(entrada));
      } else {
        if (entrada > 0) entrada -= 1;
        Serial.println("Saída registrada. Total: " + String(entrada));
      }
      salvarContagem();
      ultimaContagem = millis();
    }

    // Reseta os tempos para a próxima detecção
    tempoS1 = 0;
    tempoS2 = 0;
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(PINO_LED, OUTPUT);
  digitalWrite(PINO_LED, LOW);
  carregarContagem();

  // Configura os pinos dos sensores
  pinMode(PINO_TRIG1, OUTPUT);
  pinMode(PINO_ECHO1, INPUT);
  pinMode(PINO_TRIG2, OUTPUT);
  pinMode(PINO_ECHO2, INPUT);

  WiFi.begin(ssid, password);
  Serial.print("Conectando ao WiFi");
  unsigned long inicioConexao = millis();

  while (WiFi.status() != WL_CONNECTED) {
    if (millis() - inicioConexao >= 30000) {
      Serial.println("\nWiFi não encontrado. Continuando sem rede...");
      digitalWrite(PINO_LED, HIGH); // Acende LED vermelho
      wifiConectado = false;
      break;
    }
    delay(500);
    Serial.print(".");
  }

  if (WiFi.status() == WL_CONNECTED) {
    wifiConectado = true;
    digitalWrite(PINO_LED, LOW);
    Serial.println(WiFi.localIP().toString());
    server.begin();
  }
}

void loop() {
  // Lê os sensores e verifica passagem a cada ciclo
  if (millis() - ultimaLeitura >= 1) {
    ultimaLeitura = millis();
    medirDistancia1();
    medirDistancia2();
    verificarPassagem();
  }

  if (!wifiConectado) return;

  WiFiClient client = server.available();
  if (client) {
    String currentLine = "";

    while (client.connected()) {
      if (client.available()) {
        char c = client.read();

        if (c == '\n') {
          if (currentLine.length() == 0) {
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println();
            client.print(R"rawliteral(
              <!DOCTYPE html>
              <html lang="pt-BR">
              <head>
                  <meta charset="UTF-8">
                  <meta name="viewport" content="width=device-width, initial-scale=1.0">
                  <title>Sistema de Lotação</title>
                  <link href="https://cdn.jsdelivr.net/npm/bootstrap@5.3.3/dist/css/bootstrap.min.css" rel="stylesheet">
                  <style>
                      ul.navbar {
                          list-style-type: none;
                          margin: 0;
                          padding: 0;
                          background-color: darkblue;
                          display: flex;
                          justify-content: space-around;
                      }
                      ul.navbar li a {
                          display: block;
                          color: white;
                          padding: 14px 16px;
                          text-decoration: none;
                      }
                      ul.navbar li a:hover {
                          background-color: blue;
                          border-radius: 50%;
                          color: lightgray;
                      }
                      body { text-align: center; }
                  </style>
              </head>
              <body>
                  <nav>
                      <ul class="navbar">
                          <li><a href="#home">Home</a></li>
                          <li><a href="#lotacao">Lotação</a></li>
                          <li><a href="#sobre">Sobre</a></li>
                      </ul>
                  </nav>
                  <div class="container py-4">
                      <h1>Sistema de Lotação</h1>
                      <div class="row g-4">
                          <div class="col-md-6">
                              <div class="card">
                                  <div class="card-body">
                                      <h3>Lotação</h3>
                                      <h1 id="lotacao">0</h1>
                                  </div>
                              </div>
                          </div>
                          <div class="col-md-6">
                              <div class="card">
                                  <div class="card-body">
                                      <h3>Ocupação (50)</h3>
                                      <h1 id="total">0%</h1>
                                  </div>
                              </div>
                          </div>
                          <div class="col-md-6">
                              <div class="card">
                                  <div class="card-body">
                                      <h3>Sensor Interno</h3>
                                      <h1 id="distancia1">0</h1>
                                  </div>
                              </div>
                          </div>
                          <div class="col-md-6">
                              <div class="card">
                                  <div class="card-body">
                                      <h3>Sensor Externo</h3>
                                      <h1 id="distancia2">0</h1>
                                  </div>
                              </div>
                          </div>
                          <div class="col-md-6">
                              <div class="card">
                                  <div class="card-body">
                                      <h3>Ocupação atual</h3>
                                      <canvas id="grafico" width="100" height="50"></canvas>
                                  </div>
                              </div>
                          </div>
                      </div>
                  </div>

                  <script src="https://cdn.jsdelivr.net/npm/chart.js"></script>
                  <script>
                    const ctx = document.getElementById('grafico').getContext('2d');
                    const grafico = new Chart(ctx, {
                      type: 'doughnut',
                      data: {
                        labels: ['Ocupado', 'Livre'],
                        datasets: [{
                          data: [0, 50],
                          backgroundColor: ['red', 'green']
                        }]
                      }
                    });

                    async function atualizar() {
                      let resposta = await fetch('/dados');
                      let json = await resposta.json();

                      document.getElementById('distancia1').innerHTML = json.d1;
                      document.getElementById('distancia2').innerHTML = json.d2;
                      document.getElementById('lotacao').innerHTML = json.entrada;

                      let prct = Math.round((json.entrada / 50) * 100);
                      document.getElementById('total').innerHTML = prct + '%';

                      grafico.data.datasets[0].data = [json.entrada, 50 - json.entrada];
                      grafico.update();
                    }

                    setInterval(atualizar, 500);
                  </script>
              </body>
              </html>
            )rawliteral");
            client.println();
            break;

          } else {
            currentLine = "";
          }

        } else if (c != '\r') {
          currentLine += c;
        }

        if (currentLine.endsWith("GET /dados")) {
          client.println("HTTP/1.1 200 OK");
          client.println("Content-Type: application/json");
          client.println("Connection: close");
          client.println();
          client.println("{\"entrada\":" + String(entrada) + ",\"d1\":" + String(distancia1) + ",\"d2\":" + String(distancia2) + "}");
          break;
        }
      }
    }
    client.stop();
  }
}