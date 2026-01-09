#include <Servo.h>
#include <IRremote.hpp>
#include "ultrasonido.h"

#define SERVO_R_PIN 8 // Pin del servo derecho
#define SERVO_L_PIN 9 // Pin del servo izquierdo

Servo servo_right;
Servo servo_left;

#define LED_R 33  // LED rojo
#define LED_B 31  // LED azul

#define pinReceptor 2 // Pin de IR

// Ultrasonido de la izquierda
#define ECHO_PIN_L 45
#define TRIG_PIN_L 47
UltraSoundClass ultrasoundL(ECHO_PIN_L, TRIG_PIN_L);

// Ultrasonido de la derecha
#define ECHO_PIN_R 39
#define TRIG_PIN_R 41
UltraSoundClass ultrasoundR(ECHO_PIN_R, TRIG_PIN_R);

// Ultrasonidos del lateral
#define ECHO_PIN_LAT 51
#define TRIG_PIN_LAT 53
UltraSoundClass ultrasoundLat(ECHO_PIN_LAT, TRIG_PIN_LAT);

// Variables para elegir que ejercicio ejecutar
bool exercise_1 = false;
bool exercise_2 = false;

// Tipos de detección
#define AMBOS_DETECTAN   0
#define DERECHA_DETECTA  1
#define IZQ_DETECTA      2
#define NINGUNO_DETECTA  3

// Estructura para guardar valores de una medición
struct obj_detection {
    int place;
    float distR;
    float distL;
};

// To know where the object was detected and react according to it
struct obj_detection object;

int wall_count = 0;
#define TOTAL_WALLS 4

// Variables a cambiar
#define OBST_DIST 15
#define WALL_DIST 5
#define WALL_FOUND 25

#define MOVING_TIME 1500

#define EJ1_TIEMPO 60000 
#define EJ2_TIEMPO 120000 
unsigned long ej1_start_time = 0;
unsigned long ej2_start_time = 0;

/*-----------------------------------------------------------------------------*/
// Para definir los ultrasonidos y utulizarlos
UltraSoundClass::UltraSoundClass(int _echo, int _trg) {
  pin_echo = _echo;
  pin_trg = _trg;

  pinMode(pin_trg, OUTPUT);
  pinMode(pin_echo, INPUT);
}


float UltraSoundClass::get_dist() {
  digitalWrite(pin_trg, LOW);
  delayMicroseconds(2);

  digitalWrite(pin_trg, HIGH);
  delayMicroseconds(10);
  digitalWrite(pin_trg, LOW);

  long duration = pulseIn(pin_echo, HIGH, 200000);

  float distance = duration * 0.034 / 2; // distance in cm

  return distance;
}

/*-----------------------------------------------------------------------------*/
// Devuelve el struct donde se guardan las mediciones del ultrasonidos derecho e izquierdo y en que posicion ha detectado obstáculo
struct obj_detection check_dist() {
  struct obj_detection obj_det;
  obj_det.distR = ultrasoundR.get_dist();
  delay(150);
  obj_det.distL = ultrasoundL.get_dist();

  Serial.print("Medicion de DERECHA: ");
  Serial.println(obj_det.distR);

  Serial.print("Medicion de IZQUIERDA: ");
  Serial.println(obj_det.distL);

  if (obj_det.distL <= OBST_DIST && obj_det.distR <= OBST_DIST) {
    obj_det.place = AMBOS_DETECTAN;

  } else if (obj_det.distL <= OBST_DIST && obj_det.distR > OBST_DIST) {
    obj_det.place = IZQ_DETECTA;

  } else if (obj_det.distL > OBST_DIST && obj_det.distR <= OBST_DIST) {
    obj_det.place = DERECHA_DETECTA;

  } else {
    obj_det.place = NINGUNO_DETECTA;

  }
    return obj_det;
}

/*-----------------------------------------------------------------------------*/
void setup() {
  Serial.begin(9600);

  IrReceiver.begin(pinReceptor, ENABLE_LED_FEEDBACK); // INICIA LA RECEPCIÓN IR

  // Setup de los servos
  servo_right.attach(SERVO_R_PIN);
  servo_left.attach(SERVO_L_PIN);

  // Setup de los leds
  pinMode(LED_R, OUTPUT);
  pinMode(LED_B, OUTPUT);
}

/*-----------------------------------------------------------------------------*/
// FUNCIONES MOVIMIENTO

void girar_90_izq() {
  servo_right.write(180);
  servo_left.write(180);
  delay(1550);
}

void girar_45_izq() {
  servo_right.write(180);
  servo_left.write(180);
  delay(775);
}

void girar_90_dch() {
  servo_right.write(0);
  servo_left.write(0);
  delay(1550);
}

void girar_45_dch() {
  servo_right.write(0);
  servo_left.write(0);
  delay(775);
}

void stop() {
  servo_right.write(90);
  servo_left.write(90);
}

void avanzar_recto() {
  servo_right.write(180);
  servo_left.write(0);
}


void retroceder_recto() {
  servo_right.write(0);
  servo_left.write(180);
}

/*--------------------------------------------------------------------------------*/
void loop() {
  if (IrReceiver.decode()) {
    Serial.println(IrReceiver.decodedIRData.decodedRawData, HEX);
    IrReceiver.resume();

    if (IrReceiver.decodedIRData.decodedRawData == 0x2FFA00) {
      exercise_1 = true;
      ej1_start_time = millis();
      digitalWrite(LED_R, HIGH);
      digitalWrite(LED_B, LOW);
    } else if (IrReceiver.decodedIRData.decodedRawData == 0x2EFA20) {
      exercise_2 = true;
      ej2_start_time = millis();
      digitalWrite(LED_B, HIGH);
      digitalWrite(LED_R, LOW);
    }

    struct obj_detection obj_det;
    bool follow_wall = false;
    bool avoid = false;

    while (exercise_1) {
      // Señal mando parar ejecucion
      if (IrReceiver.decodedIRData.decodedRawData == 0x2DFA40) {
        exercise_1 = false;
        exercise_2 = false;
        stop();
      }

      // Si el minuto de tiempo del ejercicio 1 termina se para la ejecución
      if (millis() - ej1_start_time >= EJ1_TIEMPO) {
        exercise_1 = false;
        stop();
        digitalWrite(LED_R, LOW);
        digitalWrite(LED_B, LOW);
        break;
      }


      // Empieza buscando la pared en alguno de sus costados, si no la encuentra gira 90º y vuelve a comprobar, 
      // así hasta que la encuentre
      if (!follow_wall) {
        stop();
        digitalWrite(LED_B, HIGH);

        // Medir 5 veces la distancia al muro 
        for (int x = 0; x < 5; x++) {
          //obj_det.distL = ultrasoundL.get_dist();
          float dist_lat = ultrasoundLat.get_dist();

          Serial.println(obj_det.distL);
          Serial.println(dist_lat);

          // Si hay una pared a menos de 25 cm empieza a seguirla
          if (dist_lat <= WALL_FOUND && dist_lat > 0) {
            follow_wall = true;
          }
          delay(200);
        }

        // Si no encontró la pared girar 90º para revisar el sig costado
        if (!follow_wall) {
          girar_90_dch();
        }
      } else {
        // Si ha encontrado pared empieza a seguirla
        digitalWrite(LED_B, LOW);
        digitalWrite(LED_R, HIGH);

        // Medir distancia
        float dist_lat = ultrasoundLat.get_dist();
        Serial.println(dist_lat);

        // Si está muy cerca se aleja
        if (dist_lat < WALL_DIST && dist_lat > 0) {
          servo_right.write(90);
          servo_left.write(0);

        // Si está muy lejos se acerca
        } else if (dist_lat > WALL_FOUND) {
          servo_right.write(180);
          servo_left.write(90);

        // Avanza
        } else {
          digitalWrite(LED_B, HIGH);
          avanzar_recto();
        }
        delay(100);
      }
    }

    while (exercise_2) {
      digitalWrite(LED_R, HIGH);

      // Señal mando parar ejecucion
      if (IrReceiver.decodedIRData.decodedRawData == 0x2DFA40) {
        exercise_1 = false;
        exercise_2 = false;
        stop();
      }

      // Si se pasan los dos minutos parar ejecución
      if (millis() - ej2_start_time >= EJ2_TIEMPO) {
        exercise_2 = false;
        stop();
        digitalWrite(LED_B, LOW);
        digitalWrite(LED_R, LOW);
        break;
      }

      // Obtener distancias
      obj_det = check_dist();

      // Si detecta obstaculos activar la variable
      if (obj_det.place != NINGUNO_DETECTA) {
        avoid = true;
        Serial.println("OBSTACULO");
      }

      // Sin obstáculo avanza
      if (!avoid) {
        Serial.println("AVANZA");
        servo_right.write(180);
        servo_left.write(0);
        digitalWrite(LED_B, HIGH);

      // Con obstáculo comprueba donde está
      } else {
        digitalWrite(LED_B, HIGH);
        digitalWrite(LED_R, LOW);

        avoid = false;

        // Si está a la izquierda se mueve a la derecha y lo esquiva
        if (obj_det.place == IZQ_DETECTA) {
          Serial.println("IZQUIERDA");
          girar_90_dch(); // Gira

          // Avanza
          servo_right.write(180);
          servo_left.write(0);

          delay(2000);

          girar_90_izq();

        // Si está a la derecha se mueve a la izquierda y lo esquiva
        } else if (obj_det.place == DERECHA_DETECTA) {
          Serial.println("DERECHA");
          girar_90_izq();

          // Avanza
          servo_right.write(180);
          servo_left.write(0);

          delay(2000);

          girar_90_dch(); // Gira

        // Si ambos sensores detectan es que es una pared
        } else if (obj_det.place == AMBOS_DETECTAN) {

          // Aumentar contador
          wall_count++;

          // Comprobar que no haya llegado al máximo
          if (wall_count >= TOTAL_WALLS) {
            stop();
            exercise_2 = false;
          }

          // Notificar visualmente que ha encontrado una pared
          for (int k = 0; k < 5; k++) {
            digitalWrite(LED_R, !digitalRead(LED_R));
            delay(500);
            digitalWrite(LED_B, !digitalRead(LED_B));
            delay(500);
          }

          // Girar a la derecha para buscar la siguiente pared
          girar_90_dch();
        }
      }
    }
  }
}
