#include <Arduino.h>
#include <Servo.h>
#include <Adafruit_Sensor.h>

#include <Adafruit_BNO055.h>
#include <Wire.h>
#include "huskylens_utils.h"

#define TRIG_IZQ A2
#define ECHO_IZQ A3
#define TRIG_DER A0
#define ECHO_DER A1
#define MA1 5
#define MA2 4
#define PWM 6
#define SERVO_PIN 9
#define ENCODER_A 3

Servo direccion;
Adafruit_BNO055 bno = Adafruit_BNO055(55);
float Kp = 2.1, Ki = 0.0, Kd = 0.80;
int centroServo = 90;
int limAng = 50;
const int pwmRecto = 93;
const int pwmEsquiva = 120; // Velocidad especial al esquivar (ajusta a gusto)
const int pwmGiro = 165;
const int giros = 12;
int girosActuales = 0;
bool avanzandoExtra = false;
long encoderFinalExtra = 0;
const long PULSOS_EXTRA = 1000; // Pulsos extra tras el último giro
int lado = 0; // 0 = no definido, 1 = izquierda, 2 = derecha
const int distGiro = 30; // Distancia mínima para girar (cm)

bool velocidadVariable = false; // Si es false, siempre usa velocidad crucero
const int velocidadCrucero = 120; // 130    Ajusta este valor según tu necesidad
bool velocidadProporcionalServo = true; // NUEVO: Flag para velocidad proporcional al giro del servo
const int velocidadMinCrucero = 40;     //  52    Velocidad mínima en modo crucero proporcional

volatile long encoderCount = 0;
int orientation = 0;
float setpoint = 0;
bool bloqueado = false;
long pulsosBloqueo = 0;
int pulsosBloqueoUsar = 700;
const long PULSOS_GIRO = 900; // Bloqueo normal
const long PULSOS_GIRO_ESQUIVA = 1800; // Bloqueo si esquivó antes del giro
long ultimoEsquivaEncoder = -10000; // Para registrar el último encoder de esquiva
const long PASOS_ANTES_GIRO = 500;

long medirDistancia(int trigPin, int echoPin) {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  long duracion = pulseIn(echoPin, HIGH, 25000);
  long distancia = duracion * 0.017;
  if (distancia == 0) distancia = 4000;
  return distancia;
}

void encoderISR() {
  encoderCount++;
}

void bloquearHeading(long pulsos) {
  bloqueado = true;
  pulsosBloqueo = encoderCount + pulsos;
}

void actualizarOrientation(int delta) {
  orientation += delta;
  if (orientation < 0) orientation = 3;
  if (orientation > 3) orientation = 0;
  setpoint = orientation * 90;
  bloquearHeading(pulsosBloqueoUsar);
}

void setup() {
  pinMode(8, OUTPUT); // buzzer
  Serial.begin(9200);
  pinMode(MA1, OUTPUT);
  pinMode(MA2, OUTPUT);
  pinMode(PWM, OUTPUT);
  pinMode(ENCODER_A, INPUT_PULLUP);
  direccion.attach(SERVO_PIN);
  direccion.write(90);
  delay(500);
  direccion.write(centroServo);
  attachInterrupt(digitalPinToInterrupt(ENCODER_A), encoderISR, RISING);
  if (!bno.begin()) {
    Serial.println("No se detecta BNO055");
    while (1);
  }
  bno.setExtCrystalUse(true);
  pinMode(TRIG_IZQ, OUTPUT);
  pinMode(ECHO_IZQ, INPUT);
  pinMode(TRIG_DER, OUTPUT);
  pinMode(ECHO_DER, INPUT);
  sensors_event_t event;
  bno.getEvent(&event);
  setpoint = event.orientation.x;
  orientation = 0;
  encoderCount = 0;
  bloqueado = false;
  delay(1000);
  // Inicializar HUSKYlens
  huskylens.beginI2CUntilSuccess();
}


// --- Lógica de esquiva con HUSKYlens ---
bool esquivando = false;
bool prioridadEsquiva = false; // Flag para prioridad de esquiva
int objetoEsquivando = 0; // 0 = nada, 1 = id1, 2 = id2
bool giroPendiente = false; // Nuevo: indica si hay un giro pendiente tras esquiva

// Parámetros para PID de esquiva y ROI
const int ROI_X_MIN = 80;   // Límite izquierdo del ROI (ajustar según resolución HUSKYlens)
const int ROI_X_MAX = 290;  // Límite derecho del ROI
const int MIN_BLOQUE_ANCHO = 25; // Tamaño mínimo de bloque para considerar esquiva
const int MIN_BLOQUE_ALTO = 45;
const int MAX_BLOQUE_ANCHO = 960; // Límite máximo de ancho para considerar esquiva
const int MAX_BLOQUE_ALTO = 9400;  // Límite máximo de alto para considerar esquiva
float Kp_husky = 0.9, Ki_husky = 0.0, Kd_husky = 1.0;
static float integral_husky = 0, last_error_husky = 0;

bool fijarDireccionGiro = true; // Flag para activar esta lógica
int direccionGiroFijada = 0;    // 0: no fijada, -1: izquierda, 1: derecha

bool esperandoPared = false;
int paredEsperada = 0; // -1 = izquierda, 1 = derecha, 0 = ninguna
const int UMBRAL_PARED = 80; // cm, ajusta según tu entorno
unsigned long paredDetectadaDesde = 0;
const unsigned long TIEMPO_PARED_MIN = 1100; // ms, tiempo mínimo para validar pared

void loop() {
  // Calcular error de heading y bloqueo general (después de definir heading)
  
  sensors_event_t event;
  bno.getEvent(&event);
  float heading = event.orientation.x;
  long distIzq = medirDistancia(TRIG_IZQ, ECHO_IZQ);
  long distDer = medirDistancia(TRIG_DER, ECHO_DER);
  float errorHeading = heading - setpoint;
  if (errorHeading > 180) errorHeading -= 360;
  if (errorHeading < -180) errorHeading += 360;
  bool bloqueoHeadingGeneral = abs(errorHeading) > 17;
  // Buzzer
  if (distIzq > 200 || distDer > 200) {
    tone(8, 5000, 10);
  } else if (distIzq > 90 || distDer > 90) {
    tone(8, 1500, 10);
  }

  // --- Lógica de esquiva con HUSKYlens: si ve bloque id1 o id2 mayor a 10x10, girar ---
  huskylens.request();
  bool bloque_valido = false;
  int id_detectado = 0;
  int x_bloque = 0, y_bloque = 0, w_bloque = 0, h_bloque = 0;
  int max_area = 0;
  for (int id = 1; id <= 2; id++) {
    if (huskylens.isAppear(id, HUSKYLENSResultBlock)) {
      HUSKYLENSBlockInfo bloque = huskylens.readBlockParameter(id);
      int area = bloque.width * bloque.height;
      // Nueva condición: descartar si el ancho es más de 2.5 veces el alto
      bool proporcionValida = (bloque.width <= 1.1 * bloque.height);
      if (bloque.width > MIN_BLOQUE_ANCHO && bloque.width <= MAX_BLOQUE_ANCHO &&
          bloque.height > MIN_BLOQUE_ALTO && bloque.height <= MAX_BLOQUE_ALTO &&
          area > max_area && proporcionValida) {
        bloque_valido = true;
        id_detectado = id;
        x_bloque = bloque.xCenter;
        y_bloque = bloque.yCenter;
        w_bloque = bloque.width;
        h_bloque = bloque.height;
        max_area = area;
      }
    }
  }

  if (bloque_valido) {
    prioridadEsquiva = true;
    ultimoEsquivaEncoder = encoderCount;
    // Si está bloqueado por giro, marcar giro pendiente
    if (bloqueado) {
      giroPendiente = true;
      bloqueado = false; // Interrumpe el giro para esquivar
    }
    // Corrección FIJA según el lado
    int correction = 40; // Ajusta este valor para el ángulo de giro deseado
    if (id_detectado == 1) {
      // ID1: esquiva hacia la derecha
      direccion.write(centroServo - correction);
    } else if (id_detectado == 2) {
      // ID2: esquiva hacia la izquierda
      direccion.write(centroServo + correction);
    }
    // Limitar el ángulo para no exceder el rango seguro
    int servo_angle = direccion.read();
    if (servo_angle < centroServo - limAng) servo_angle = centroServo - limAng;
    if (servo_angle > centroServo + limAng) servo_angle = centroServo + limAng;
    direccion.write(servo_angle);
    esquivando = true;

    // --- Ajuste de velocidad de esquiva según ángulo del servo ---
    int velocidadEsquivaActual = pwmEsquiva;
    if (servo_angle >= 77 && servo_angle <= 103) {
      velocidadEsquivaActual = pwmEsquiva - 18;
      if (velocidadEsquivaActual < 0) velocidadEsquivaActual = 0;
    }

    int velocidadActual = velocidadCrucero;
    if (velocidadVariable) {
        velocidadActual = velocidadEsquivaActual;
    }
    analogWrite(PWM, velocidadActual);

    digitalWrite(MA1, HIGH);
    digitalWrite(MA2, LOW);
    Serial.print("HUSKYlens: ID="); Serial.print(id_detectado);
    Serial.print(" X="); Serial.print(x_bloque);
    Serial.print(" Y="); Serial.print(y_bloque);
    Serial.print(" W="); Serial.print(w_bloque);
    Serial.print(" H="); Serial.println(h_bloque);
    return;
  } else if (esquivando) {
    direccion.write(centroServo);
    esquivando = false;
    prioridadEsquiva = false;
    objetoEsquivando = 0;
    // Si había un giro pendiente, reactivar el bloqueo de heading
    if (giroPendiente) {
      bloquearHeading(pulsosBloqueoUsar);
      giroPendiente = false;
    }
  }

  // PID para mantener heading
  float error = heading - setpoint;
  if (error > 180) error -= 360;
  if (error < -180) error += 360;
  static float integral = 0, last_error = 0;
  integral += error;
  float derivative = error - last_error;
  float correction = Kp * error + Ki * integral + Kd * derivative;
  int servo_angle = centroServo - correction;
  if (servo_angle < centroServo - limAng) servo_angle = centroServo - limAng;
  if (servo_angle > centroServo + limAng) servo_angle = centroServo + limAng;
  direccion.write(servo_angle);
  last_error = error;

  // Avanzar motor: PWM depende de si está girando (bloqueado) o recto
  int velocidadActual = velocidadCrucero;
  if (velocidadVariable) {
    if (esquivando) {
        velocidadActual = pwmEsquiva;
    } else if (bloqueado) {
        velocidadActual = pwmGiro;
    } else {
        velocidadActual = pwmRecto;
    }
  }

  // NUEVO: velocidad proporcional al giro del servo en modo crucero
  if (!velocidadVariable && velocidadProporcionalServo) {
    int desviacion = abs(servo_angle - centroServo);
    // Cuanto más gire el servo, más rápido va (mínimo en recto, máximo en limAng)
    velocidadActual = velocidadMinCrucero + (velocidadCrucero - velocidadMinCrucero) * desviacion / limAng;
    if (velocidadActual > velocidadCrucero) velocidadActual = velocidadCrucero;
    if (velocidadActual < velocidadMinCrucero) velocidadActual = velocidadMinCrucero;
  }

  analogWrite(PWM, velocidadActual);
  digitalWrite(MA1, HIGH);
  digitalWrite(MA2, LOW);

  // Cambiar heading solo si no está bloqueado
  if (!bloqueado && !bloqueoHeadingGeneral && !prioridadEsquiva && !esperandoPared) {
  pulsosBloqueoUsar = PULSOS_GIRO;

  if (fijarDireccionGiro && direccionGiroFijada != 0) {
    if (direccionGiroFijada == -1 && distIzq > 75) {
      long distDerGiro = medirDistancia(TRIG_DER, ECHO_DER);
      pulsosBloqueoUsar = (distDerGiro < distGiro) ? 00 : 00;
      actualizarOrientation(-1);
      girosActuales++;
      if (girosActuales == giros) {
        avanzandoExtra = true;
        encoderFinalExtra = encoderCount + PULSOS_EXTRA;
      }
      // Esperar a que reaparezca la pared izquierda
      esperandoPared = true;
      paredEsperada = -1;
    } else if (direccionGiroFijada == 1 && distDer > 75) {
      long distIzqGiro = medirDistancia(TRIG_IZQ, ECHO_IZQ);
      pulsosBloqueoUsar = (distIzqGiro < distGiro) ? 00 : 00;
      actualizarOrientation(1);
      girosActuales++;
      if (girosActuales == giros) {
        avanzandoExtra = true;
        encoderFinalExtra = encoderCount + PULSOS_EXTRA;
      }
      // Esperar a que reaparezca la pared derecha
      esperandoPared = true;
      paredEsperada = 1;
    }
  } else {
    if (distIzq > 75) {
      long distDerGiro = medirDistancia(TRIG_DER, ECHO_DER);
      pulsosBloqueoUsar = (distDerGiro < distGiro) ? 0 : 0;
      actualizarOrientation(-1);
      girosActuales++;
      if (fijarDireccionGiro) direccionGiroFijada = -1;
      if (girosActuales == giros) {
        avanzandoExtra = true;
        encoderFinalExtra = encoderCount + PULSOS_EXTRA;
      }
      esperandoPared = true;
      paredEsperada = -1;
    } else if (distDer > 75) {
      long distIzqGiro = medirDistancia(TRIG_IZQ, ECHO_IZQ);
      pulsosBloqueoUsar = (distIzqGiro < distGiro) ? 0 : 0;
      actualizarOrientation(1);
      girosActuales++;
      if (fijarDireccionGiro) direccionGiroFijada = 1;
      if (girosActuales == giros) {
        avanzandoExtra = true;
        encoderFinalExtra = encoderCount + PULSOS_EXTRA;
      }
      esperandoPared = true;
      paredEsperada = 1;
    }
  }
  } else {
    float errorHeading = heading - setpoint;
    if (errorHeading > 180) errorHeading -= 360;
    if (errorHeading < -180) errorHeading += 360;
    if (encoderCount >= pulsosBloqueo && abs(errorHeading) < 15) {
      bloqueado = false;
    }
  }

  // Lógica para liberar el giro cuando la pared reaparece (con validación por tiempo)
  if (esperandoPared) {
    bool paredDetectada = false;
    if (paredEsperada == -1 && distIzq < UMBRAL_PARED) paredDetectada = true;
    if (paredEsperada == 1 && distDer < UMBRAL_PARED) paredDetectada = true;

    if (paredDetectada) {
      if (paredDetectadaDesde == 0) paredDetectadaDesde = millis();
      if (millis() - paredDetectadaDesde > TIEMPO_PARED_MIN) {
        esperandoPared = false;
        paredEsperada = 0;
        paredDetectadaDesde = 0;
      }
    } else {
      paredDetectadaDesde = 0; // Reinicia si deja de detectar la pared
    }
  }

  // (Opcional) imprimir estado por Serial
  Serial.print("Heading: "); Serial.print(heading);
  Serial.print(" | Setpoint: "); Serial.print(setpoint);
  Serial.print(" | Orientation: "); Serial.print(orientation);
  Serial.print(" | Encoder: "); Serial.print(encoderCount);
  Serial.print(" | US Izq: "); Serial.print(distIzq);
  Serial.print(" | US Der: "); Serial.println(distDer);
  if (avanzandoExtra) {
    if (encoderCount >= encoderFinalExtra) {
      Serial.println("Avance extra completado. Deteniendo.");
      digitalWrite(MA1, LOW);
      digitalWrite(MA2, LOW);
      while (1); // Detener el programa
    }
  } else if (girosActuales > giros) {
    Serial.println("Número máximo de giros alcanzado. Deteniendo.");
    digitalWrite(MA1, LOW);
    digitalWrite(MA2, LOW);
    while (1); // Detener el programa
  }
}

