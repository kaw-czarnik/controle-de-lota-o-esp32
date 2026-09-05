# 🚪 Sistema de Controle de Lotação e Fluxo em Tempo Real (ESP32)

Um sistema IoT de controle e monitoramento de lotação desenvolvido com **ESP32** e **Sensores Ultrassônicos**. O projeto identifica o sentido de movimentação (entrada ou saída de pessoas), salva os dados na memória não volátil e disponibiliza um dashboard em tempo real via web server embutido.

---

## 🛠️ Minhas Contribuições no Projeto

Neste projeto em grupo, atuei diretamente na parte de hardware e validação física:
* **Testes de Componentes:** Verificação individual de funcionamento dos sensores ultrassônicos (HC-SR04), ESP32 e LEDs.
* **Montagem e Construção Física:** Auxílio no posicionamento físico e alinhamento dos sensores no protótipo para garantir a precisão da leitura do fluxo.
* **Testes de Campo / Físicos:** Validação prática da lógica de detecção de passagem (testes de velocidade, distância de corte e prevenção de falsos positivos).

---

## 📌 Funcionalidades

- **Contagem Bidirecional Dinâmica:** Diferencia pessoas entrando e saindo através da sequência temporal de acionamento dos dois sensores.
- **Servidor Web Embutido:** Interface gráfica criada com Bootstrap e Chart.js exibida diretamente no navegador.
- **Atualização em Tempo Real:** Requisições via `fetch` em segundo plano para atualização de dados sem recarregar a página.
- **Memória Não Volátil (NVS):** Salva a contagem atual usando a biblioteca `Preferences`, garantindo que os dados não sejam perdidos ao reiniciar o ESP32.
- **Tratamento de Ruídos (Filtros de Segurança):**
  - *Cooldown:* Intervalo mínimo entre contagens para evitar leitura dupla.
  - *Timeout:* Reseta leituras incompletas se a pessoa não completar a passagem.
  - *Filtro de Simultaneidade:* Desconsidera acionamentos simultâneos (inferiores a 50ms).

---

## 🧩 Componentes de Hardware

- **1x** Placa ESP32 (DevKit)
- **2x** Sensores Ultrassônicos HC-SR04
- **1x** LED Indicador de Status (Wi-Fi)
- Protoboard e Jumpers para conexões

---

## 🔌 Mapeamento de Pinos (Pinout)

| Componente | Pino do Componente | Pino do ESP32 (GPIO) |
| :--- | :--- | :--- |
| **Sensor 1 (Entrada)** | TRIG | GPIO 27 |
| **Sensor 1 (Entrada)** | ECHO | GPIO 26 |
| **Sensor 2 (Saída)** | TRIG | GPIO 25 |
| **Sensor 2 (Saída)** | ECHO | GPIO 33 |
| **LED Status** | Anodo (+) | GPIO 2 |

---

## 🧠 Lógica de Funcionamento

1. Os sensores medem continuamente distâncias inferiores a **50 cm**.
2. **Entrada:** O *Sensor 1* detecta primeiro e o *Sensor 2* detecta em seguida ($Tempo_{S1} < Tempo_{S2}$).
3. **Saída:** O *Sensor 2* detecta primeiro e o *Sensor 1* detecta em seguida ($Tempo_{S2} < Tempo_{S1}$).
4. **Resets e Filtros:** Se o segundo sensor não for ativado dentro de 1 segundo após o primeiro, a leitura é descartada.

---

## 💻 Dashboard Web

Ao se conectar à mesma rede que o ESP32, acesse o IP exibido no Monitor Serial para visualizar:
- **Lotação Atual:** Quantidade total de pessoas presentes.
- **Porcentagem de Ocupação:** Baseada na capacidade limite definida (padrão: 50 pessoas).
- **Leitura Atual dos Sensores:** Distância capturada em tempo real por cada um dos sensores.
- **Gráfico Rosca (Doughnut):** Representação visual intuitiva da ocupação vs. espaço livre.

---

## 🚀 Como Executar o Projeto

1. **Requisitos de Software:**
   - [Arduino IDE](https://www.arduino.cc/en/software) com suporte a placas ESP32 instalado.
   - Bibliotecas nativas: `WiFi.h`, `Wire.h`, `Preferences.h`.

2. **Configuração da Rede:**
   No código-fonte, substitua as credenciais de Wi-Fi pelos dados da sua rede:
   ```cpp
   const char *ssid     = "NOME_DA_SUA_REDE";
   const char *password = "SENHA_DA_SUA_REDE";

```

3. **Upload:**
* Conecte o ESP32 ao computador via cabo USB.
* Selecione a placa **ESP32 Dev Module** e a porta COM correspondente.
* Faça o upload do código.
* Abra o **Monitor Serial** (115200 baud) para capturar o endereço IP atribuído.



---

## 👥 Créditos e Desenvolvimento

Projeto desenvolvido em equipe para aplicação de conceitos de IoT, eletrônica digital e sistemas em tempo real.

```
* **GitHub:** [@kaw-czarnik](https://www.google.com/search?q=https://github.com/kaw-czarnik)
* **GitHub:** [@joaovcampos-dev](https://www.google.com/search?q=https://github.com/joaovcampos-dev)
* **GitHub:** [@joaoBaratto](https://www.google.com/search?q=https://github.com/JoaoBaratto)
```