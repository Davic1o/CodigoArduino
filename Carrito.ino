#include <WiFi.h>
#include <WebServer.h>
#include <HTTPClient.h>

// Wi-Fi
const char* ssid = "Davicio";
const char* password = "pablitos";

// Pines motores L298N
#define MOTOR_A1 15
#define MOTOR_A2 2
#define MOTOR_B1 4
#define MOTOR_B2 16

// Pines sensores ultrasónicos
#define TRIG_FRONT 5
#define ECHO_FRONT 18
#define TRIG_BACK 19
#define ECHO_BACK 21

// Buzzer y LDR
#define BUZZER_PIN 22
#define LDR_PIN 34
#define LED_IZQ 23
#define LED_DER 25

// Umbrales
const int umbralParada = 7;
const int umbralBuzzerMin = 10;
const int umbralBuzzerMax = 20;
const int umbralLuz = 2000;

// Variables de estado
String ultimoComando = "";
bool motoresActivos = false;
unsigned long ultimoTiempoComando = 0;
const unsigned long tiempoLimite = 5000;

WebServer server(80);

// BD
const char* eventoUrl = "https://pruebas.entramadosec.com/crear_evento.php";

// Prototipos
float medirDistancia(int trig, int echo);
void enviarEvento(String direccion);
void detener();

void setup() {
  Serial.begin(115200);

  pinMode(MOTOR_A1, OUTPUT);
  pinMode(MOTOR_A2, OUTPUT);
  pinMode(MOTOR_B1, OUTPUT);
  pinMode(MOTOR_B2, OUTPUT);

  pinMode(TRIG_FRONT, OUTPUT);
  pinMode(ECHO_FRONT, INPUT);
  pinMode(TRIG_BACK, OUTPUT);
  pinMode(ECHO_BACK, INPUT);

  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(BUZZER_PIN, LOW);
  pinMode(LED_IZQ, OUTPUT);
  pinMode(LED_DER, OUTPUT);

  WiFi.begin(ssid, password);
  Serial.print("Conectando al Wi-Fi...");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500); Serial.print(".");
  }
  Serial.println("\n✅ Conectado. IP: " + WiFi.localIP().toString());

  server.on("/comando", HTTP_POST, recibirComando);
  server.begin();
  Serial.println("🌐 Servidor web iniciado");
}

void loop() {
  server.handleClient();

  float distanciaFrontal = medirDistancia(TRIG_FRONT, ECHO_FRONT);
  float distanciaTrasera = medirDistancia(TRIG_BACK, ECHO_BACK);
  int valorLuz = analogRead(LDR_PIN);

  Serial.print("📏 Adelante: ");
  Serial.print(distanciaFrontal);
  Serial.print(" cm | Atrás: ");
  Serial.print(distanciaTrasera);
  Serial.print(" cm | Luz: ");
  Serial.println(valorLuz);

  if (motoresActivos) {
    bool pita = false;

    if ((ultimoComando == "adelante" && distanciaTrasera > umbralBuzzerMin && distanciaTrasera <= umbralBuzzerMax) ||
        (ultimoComando == "atras" && distanciaFrontal > umbralBuzzerMin && distanciaFrontal <= umbralBuzzerMax)) {
      pita = true;
    }

    digitalWrite(BUZZER_PIN, pita ? HIGH : LOW);

    // Detener si la distancia es crítica (sin importar el comando)
    if (distanciaFrontal <= umbralParada || distanciaTrasera <= umbralParada) {
      detener();
      enviarEvento("Detenido (" + ultimoComando + ")");
      Serial.println("🛑 Detenido por obstáculo.");
    }

    // Timeout
    if (millis() - ultimoTiempoComando > tiempoLimite) {
      detener();
      Serial.println("⏱️ Timeout sin comando. Motores detenidos.");
    }

  } else {
    digitalWrite(BUZZER_PIN, LOW);
  }

  if (valorLuz < umbralLuz) {
    digitalWrite(LED_IZQ, HIGH);
    digitalWrite(LED_DER, HIGH);
  } else {
    digitalWrite(LED_IZQ, LOW);
    digitalWrite(LED_DER, LOW);
  }

  delay(200);
}

void recibirComando() {
  if (!server.hasArg("plain")) {
    server.send(400, "text/plain", "❌ No se recibió ningún comando.");
    return;
  }

  String comando = server.arg("plain");
  comando.toLowerCase();
  Serial.println("📨 Comando recibido: " + comando);

  if (comando == "atras") moverAdelante();
  else if (comando == "adelante") moverAtras();
  else if (comando == "izquierda") girarIzquierda();
  else if (comando == "derecha") girarDerecha();
  else {
    detener();
    server.send(400, "text/plain", "❌ Comando inválido.");
    return;
  }

  ultimoComando = comando;
  motoresActivos = true;
  ultimoTiempoComando = millis();
  server.send(200, "text/plain", "✅ Ejecutando: " + comando);
}

float medirDistancia(int trig, int echo) {
  digitalWrite(trig, LOW);
  delayMicroseconds(2);
  digitalWrite(trig, HIGH);
  delayMicroseconds(10);
  digitalWrite(trig, LOW);
  long duracion = pulseIn(echo, HIGH, 30000);
  return duracion * 0.034 / 2.0;
}

void moverAdelante() {
  Serial.println("🚗 Adelante");
  digitalWrite(MOTOR_A1, HIGH);
  digitalWrite(MOTOR_A2, LOW);
  digitalWrite(MOTOR_B1, LOW);
  digitalWrite(MOTOR_B2, LOW);
}

void moverAtras() {
  Serial.println("🔙 Atrás");
  digitalWrite(MOTOR_A1, LOW);
  digitalWrite(MOTOR_A2, HIGH);
  digitalWrite(MOTOR_B1, LOW);
  digitalWrite(MOTOR_B2, LOW);
}

void girarIzquierda() {
  Serial.println("↪️ Izquierda");
  digitalWrite(MOTOR_B1, HIGH);
  digitalWrite(MOTOR_B2, LOW);
  digitalWrite(MOTOR_A1, LOW);
  digitalWrite(MOTOR_A2, LOW);
}

void girarDerecha() {
  Serial.println("↩️ Derecha");
  digitalWrite(MOTOR_B1, LOW);
  digitalWrite(MOTOR_B2, HIGH);
  digitalWrite(MOTOR_A1, LOW);
  digitalWrite(MOTOR_A2, LOW);
}

void detener() {
  Serial.println("🛑 Detenido");
  digitalWrite(MOTOR_A1, LOW);
  digitalWrite(MOTOR_A2, LOW);
  digitalWrite(MOTOR_B1, LOW);
  digitalWrite(MOTOR_B2, LOW);
  motoresActivos = false;
  digitalWrite(BUZZER_PIN, LOW);
}

void enviarEvento(String direccion) {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    http.begin(eventoUrl);
    http.addHeader("Content-Type", "application/json");

    String jsonData = "{\"posicion\": \"" + direccion + "\"}";
    int httpCode = http.POST(jsonData);
    String response = http.getString();

    Serial.print("📡 Código HTTP (evento): ");
    Serial.println(httpCode);
    Serial.println("📬 Respuesta: " + response);

    http.end();
  } else {
    Serial.println("❌ Wi-Fi no conectado. No se pudo enviar evento.");
  }
}
