#include <Arduino.h>
#include <Wire.h>
#include <Servo.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BNO055.h>
// Pines para HC-SR04 (usando A0=14, A1=15, A2=16, A3=17)
#define TRIG_IZQ 14 // A0
#define ECHO_IZQ 15 // A1
#define TRIG_DER 16 // A2
#define ECHO_DER 17 // A3

int orientation = 0; // 0=North, 90=East, 180=South, 270=West
int facing = 0;



// Pines del motor y encoder
#define MA1 5
#define MA2 4
#define PWM 6
#define ENCODER_A 3

// Servo y BNO055
#define SERVO_PIN 9

// Pines XSHUT
#define XSHUT1 A3
#define XSHUT2 A6

// Direcciones I2C nuevas
// No se requieren direcciones I2C para HC-SR04

// Parámetros del encoder y rueda
const float WHEEL_DIAMETER_MM = 35.0;
const float WHEEL_PERIMETER_MM = WHEEL_DIAMETER_MM * 3.1416;
const int ENCODER_PULSES_PER_REV = 420;
const float PULSES_PER_MM = ENCODER_PULSES_PER_REV / WHEEL_PERIMETER_MM;
const int DIST_MM = 450; // 10 cm
const int TARGET_PULSES = PULSES_PER_MM * DIST_MM;

// PID parámetros
float Kp = 0.9;   // Proporcional
float Ki = 0.0;   // Integral
float Kd = 1.0;   // Derivativo

volatile long encoderCount = 0;
Servo direccion;
Adafruit_BNO055 bno = Adafruit_BNO055(55);

long medirDistancia(int trigPin, int echoPin) {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  long duracion = pulseIn(echoPin, HIGH, 25000); // 25ms timeout
  long distancia = duracion * 0.017;
  return distancia; // en cm
}

void encoderISR() {
  encoderCount++;
}

void setup() {
  Serial.begin(115200);
  pinMode(8, OUTPUT); // buzzer
  pinMode(MA1, OUTPUT);
  pinMode(TRIG_IZQ, OUTPUT);
  pinMode(ECHO_IZQ, INPUT);
  pinMode(TRIG_DER, OUTPUT);
  pinMode(ECHO_DER, INPUT);
  delay(200);
  digitalWrite(XSHUT2, HIGH);
  delay(10);
  tone(8, 1500, 50);
  delay(70);
  tone(8, 1500, 50);
  delay(70);
}

void moveMotorPID(bool forward, int pulses) {
  encoderCount = 0;

  // Inicializa motor
  if (forward) {
    digitalWrite(MA1, HIGH);
    digitalWrite(MA2, LOW);
  } else {
    digitalWrite(MA1, LOW);
    digitalWrite(MA2, HIGH);
  }

  int maxPWM = 200;
  int minPWM = 80;

  // PID variables
  float setpoint = 0;
  float last_error = 0;
  float integral = 0;

  // Lee el heading inicial
  sensors_event_t event;
  bno.getEvent(&event);
  setpoint = 0 + facing; // Heading absoluto
  if (forward == false) {
    setpoint += 180;
    if (setpoint > 360) setpoint -= 360;
  }

  while (encoderCount < pulses) {
    // Lee heading actual
    bno.getEvent(&event);
    float heading = event.orientation.x;

    // Calcula error de heading (considera wrap-around 0-360)
    float error = heading - setpoint;
    if (error > 180) error -= 360;
    if (error < -180) error += 360;

    integral += error;
    float derivative = error - last_error;

    float correction = Kp * error + Ki * integral + Kd * derivative;

    // Corrige servo (90 es recto)
    int servo_angle = 85 - correction;
    if (servo_angle < 50) servo_angle = 50;
    if (servo_angle > 150) servo_angle = 150;
    direccion.write(servo_angle);

    // Control de velocidad proporcional a la distancia restante
    int error_dist = pulses - encoderCount;
    int pwmValue = 0.5 * error_dist;
    if (pwmValue > maxPWM) pwmValue = maxPWM;
    if (pwmValue < minPWM) pwmValue = minPWM;
    analogWrite(PWM, pwmValue);


  // Leer sensores ultrasónicos y mostrar por Serial
  long distIzq = medirDistancia(TRIG_IZQ, ECHO_IZQ);
  long distDer = medirDistancia(TRIG_DER, ECHO_DER);

  Serial.print("Pulsos: "); Serial.print(encoderCount);
  Serial.print(" | Heading: "); Serial.print(heading);
  Serial.print(" | Servo: "); Serial.print(servo_angle);
  Serial.print(" | PWM: "); Serial.print(pwmValue);
  Serial.print(" | US Izq: "); Serial.print(distIzq); Serial.print(" cm");
  Serial.print(" | US Der: "); Serial.print(distDer); Serial.println(" cm");

    last_error = error;
    delay(10);
  }

  // Detener motor y centrar servo
  analogWrite(PWM, 0);
  digitalWrite(MA1, LOW);
  digitalWrite(MA2, LOW);
  direccion.write(90);
}

void loop() {
  Serial.println("Moviendo adelante 10cm...");
  moveMotorPID(true, TARGET_PULSES);
  orientation = orientation + 1;
  if (orientation >= 4) orientation = 0;
  facing = orientation * 90;

  delay(500); // Pequeña pausa antes de repetir
}