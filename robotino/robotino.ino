#include <Servo.h>
#include <IRremote.hpp>
#include "ultrasonido.h"

#define SERVO_R_PIN 8     // Pin de señal del servo
#define SERVO_L_PIN 9

Servo servo_right;  // Objeto servo
Servo servo_left;
//#define TIEMPO_GIRO OBST_DIST00  // Tiempo en milisegundos (2 segundos)

#define LED_R 33  // LED rojo
#define LED_B 31  // LED azul

#define pinReceptor 2

#define ECHO_PIN_L 45
#define TRIG_PIN_L 47
UltraSoundClass ultrasoundL(ECHO_PIN_L, TRIG_PIN_L);

#define ECHO_PIN_R 39
#define TRIG_PIN_R 41
UltraSoundClass ultrasoundR(ECHO_PIN_R, TRIG_PIN_R);

#define ECHO_PIN_LAT 51
#define TRIG_PIN_LAT 53
UltraSoundClass ultrasoundLat(ECHO_PIN_LAT, TRIG_PIN_LAT);

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

// States
int state = 0;
#define SEARCHING_WALL 0
#define WALL_FINDED 1
#define END 2

// States of SEARCHING_WALL
int search_wall_state = 0;
#define MOVE 0
//#define OBJ_DECT 1
#define AVOID 1

// Time vars
float init_time = -1;

int wall_count = 0;
#define TOTAL_WALLS 4

// Variables a cambiar
#define OBST_DIST 15
#define WALL_DIST 5

#define MOVING_TIME 1500

#define MEDICIONES 5

#define EJ1_TIEMPO 60000 
#define EJ2_TIEMPO 120000 
unsigned long ej1_start_time = 0;
unsigned long ej2_start_time = 0;

/*-----------------------------------------------------------------------------*/
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
char* get_action_name(int act_id){
    switch (act_id) {
        case AMBOS_DETECTAN:
            return "AMBOS_DETECTAN";
        case DERECHA_DETECTA:
            return "DERECHA_DETECTA";
        case IZQ_DETECTA:
            return "IZQ_DETECTA";
        case NINGUNO_DETECTA:
            return "NINGUNO_DETECTA";
        default: 
            return "UNKNOWN";  Serial.begin(9600);
    }
}

// Get the diff of times between a pass time (t0) and a actual time (t1) in secs
// Return -1 if any time is illegal
float get_time_diff(long t0, long t1) {
  if (t0 < 0 || t1 < 0)
    return -1;
  else
    return (t1 - t0) / 1000;
}

struct obj_detection check_dist() {
  struct obj_detection obj_det;

  // Conseguir 10 valores
  float distR[MEDICIONES];
  float distL[MEDICIONES];
  for (int x = 0; x < MEDICIONES; x++) {
    distR[x] = ultrasoundR.get_dist();
    distL[x] = ultrasoundL.get_dist();
  }

  // Hacer la media entre los valores que no se desvien mucho e ignorando max y min del resto
  float sum_R;
  float min_R = distR[0], max_R = distR[0];
  int n_R = MEDICIONES;

  float sum_L;
  float min_L = distL[0], max_L = distL[0];
  int n_L = MEDICIONES;

  char arr[MEDICIONES];
  for (int i = 0; i < MEDICIONES; i++) {
    if (distR[i] > 100) { // Ignorar valor
      n_R--;
      continue;
    }

    sum_R += arr[i];
    if (distR[i] < min_R)
      min_R = distR[i];
    if (distR[i] > max_R)
      max_R = distR[i];
  }

  for (int j = 0; j < MEDICIONES; j++) {
    // Ignorar valor
    if (distL[j] > 100) {
      n_L--;
      continue;
    }

    sum_L += arr[j];
    if (distL[j] < min_L)
      min_L = distL[j];
    if (distL[j] > max_L)
      max_L = distL[j];
  }

  // Si se han borrado todal las mediciones o solo queda el min y max volver a empezar
  if (n_R <= 2 || n_L <= 2) 
    check_dist();

  obj_det.distR = (sum_R - min_R - max_R) / (n_R - 2);
  obj_det.distL = (sum_L - min_L - max_L) / (n_L - 2);

  Serial.print("Medicion de DERECHA: ");
  Serial.println(obj_det.distR);

  Serial.print("Medicion de IZQUIERDA: ");
  Serial.println(obj_det.distL);

  if(obj_det.distL <= OBST_DIST && obj_det.distR <= OBST_DIST) {
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

  IrReceiver.begin(pinReceptor, ENABLE_LED_FEEDBACK); // INICIA LA RECEPCIÓN

  servo_right.attach(SERVO_R_PIN);
  servo_left.attach(SERVO_L_PIN);

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

void aproximacion() {

  while (object.distL >= WALL_DIST && object.distR >= WALL_DIST) {
    object = check_dist();
    avanzar_recto();

    // Seguridad por falsa pared, si ambos dejan de detectar obj es que era un
    // obstavulo y no pared porque la pared es lo sufucientemente alta para que 
    // se vea siemrpe  
    if (object.distL >= OBST_DIST && object.distR >= OBST_DIST) {
      // retroceder porque se habrá acercado demasiado al obj
      retroceder_recto();
      delay(MOVING_TIME);

      // esquivar a derecha
      girar_45_dch();
      avanzar_recto();
      delay(MOVING_TIME);
      girar_45_izq();

      search_wall_state = MOVE; // Volver al caso por defecto
     return;
    }
  }

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
    while (exercise_1) {
      // Señal mando parar ejecucion
      if (IrReceiver.decodedIRData.decodedRawData == 0x2DFA40) {
        exercise_1 = false;
        exercise_2 = false;
        stop();
      }

      if (millis() - ej1_start_time >= EJ1_TIEMPO) {
        exercise_1 = false;
        stop();
        digitalWrite(LED_R, LOW);
        digitalWrite(LED_B, LOW);
        break;
      }

      obj_det.distR = ultrasoundR.get_dist();
      obj_det.distL = ultrasoundL.get_dist();
      float dist_lat = ultrasoundLat.get_dist();
      if (obj_det.distL <= WALL_DIST && obj_det.distR <= WALL_DIST) {
        girar_90_dch();
        follow_wall = true;
        digitalWrite(LED_B, HIGH);
      } else if (obj_det.distR <= WALL_DIST && obj_det.distL > WALL_DIST) {
        servo_right.write(180);
        servo_left.write(180);
      } else if (obj_det.distL <= WALL_DIST && obj_det.distR > WALL_DIST) {
        servo_right.write(0);
        servo_left.write(0);
      } else {
        avanzar_recto();
      }
      delay(500);

      if (dist_lat <= 3 && follow_wall) {
        servo_right.write(0);
        servo_left.write(0);
      } else if (dist_lat >= 8 && follow_wall) {
        servo_right.write(180);
        servo_left.write(180);
      }
    }

    while (exercise_2) {
      // Señal mando parar ejecucion
      if (IrReceiver.decodedIRData.decodedRawData == 0x2DFA40) {
        exercise_1 = false;
        exercise_2 = false;
        stop();
      }

      if (millis() - ej2_start_time >= EJ2_TIEMPO) {
        exercise_2 = false;
        stop();
        digitalWrite(LED_B, LOW);
        digitalWrite(LED_R, LOW);
        break;
      }

      // Switch controller
      switch(state) {
        case SEARCHING_WALL:
        {
          switch(search_wall_state) {
            case MOVE:
            {
              // Forward
              avanzar_recto();

              // Check if an object has been detected
              object = check_dist();
              if (object.place != NINGUNO_DETECTA) {
                // Stop
                stop();

                if (object.distL <= WALL_DIST || object.distR <= WALL_DIST) {
                  // Backward
                  retroceder_recto();
                  delay(MOVING_TIME);
                  search_wall_state = AVOID;
                  
                } else {
                  search_wall_state = AVOID;
                }

              }
              break;
            }

            case AVOID:
            {
              if (object.place == IZQ_DETECTA) {
                digitalWrite(LED_B, HIGH);
                digitalWrite(LED_R, LOW);

                girar_45_dch();
                avanzar_recto();
                delay(MOVING_TIME);
                girar_45_izq();

                
              } else if (object.place == DERECHA_DETECTA) {
                digitalWrite(LED_B, LOW);
                digitalWrite(LED_R, HIGH);

                girar_45_izq();
                avanzar_recto();
                delay(MOVING_TIME);
                girar_45_dch();

              } else if (object.place == AMBOS_DETECTAN) {
                digitalWrite(LED_B, HIGH);
                digitalWrite(LED_R, HIGH);

                girar_45_dch();
                avanzar_recto();
                delay(MOVING_TIME);
                girar_45_izq();
              }

              digitalWrite(LED_B, LOW);
              digitalWrite(LED_R, LOW);

              // Despues de esquivar comporbamos si sigue el obstáculo
              object = check_dist();

              // Si los dos sensores detectan es muro
              if (object.place == AMBOS_DETECTAN) {
                // Si está sufucientemente cerca es muro directamente
                if (object.distL <= WALL_DIST && object.distR <= WALL_DIST) {
                  state = WALL_FINDED;

                  for (int x = 0; x < 6; x++) {
                    digitalWrite(LED_B, !digitalRead(LED_B));
                    digitalWrite(LED_R, !digitalRead(LED_R));
                    delay(300);
                  }

                // Si no se acerca hasta la dist estipulada
                } else {
                  // APROXIMARSE
                  aproximacion();

                  for (int x = 0; x < 6; x++) {
                    digitalWrite(LED_B, !digitalRead(LED_B));
                    digitalWrite(LED_R, !digitalRead(LED_R));
                    delay(300);
                  }

                  state = WALL_FINDED;
                }

              // Si no detecta nada, vuelve a caminar recto proque esquivó el obst
              } else if (object.place == NINGUNO_DETECTA) {
                search_wall_state = MOVE;
              }

              // Si no se da ninguno de los casos anteriores entonces vuelve a esquivar
              break;
            }

          break;
        }

        case WALL_FINDED:
        {
          wall_count++;
          if (wall_count == TOTAL_WALLS){
            state = END;

          // Girar a la derecha para encarar siguiente pared
          } else { 
            retroceder_recto();
            delay(MOVING_TIME);
            girar_90_dch();
            state = SEARCHING_WALL;
          }

          break;
        }

        case END:
        {
          digitalWrite(LED_R, !digitalRead(LED_R));
          delay(500);
          digitalWrite(LED_B, !digitalRead(LED_B));
          delay(500);
          stop();

          break;
        }
      }
      break;
    }

    }
  }
}
