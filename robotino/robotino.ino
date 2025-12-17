#include <Servo.h>
#include "ultrasonido.h"

#define SERVO_R_PIN 8     // Pin de señal del servo
#define SERVO_L_PIN 9
//#define TIEMPO_GIRO OBST_DIST00  // Tiempo en milisegundos (2 segundos)

Servo servo_right;  // Objeto servo
Servo servo_left;

#define ECHO_PIN_L 35
#define TRIG_PIN_L 39
UltraSoundClass ultrasoundL(ECHO_PIN_L, TRIG_PIN_L);

#define ECHO_PIN_R 45
#define TRIG_PIN_R 47
UltraSoundClass ultrasoundR(ECHO_PIN_R, TRIG_PIN_R);

bool exercise_1 = false;
bool exercise_2 = true;


#define AMBOS_DETECTAN   0
#define DERECHA_DETECTA  1
#define IZQ_DETECTA      2
#define NINGUNO_DETECTA  3


struct obj_detection {
    int place;
    float distR;
    float distL;
};

// To know where the object was detected and react according to it
struct obj_detection object;

bool rotar = false;
bool rotar2 = false;
bool avance = false;

#define OBST_DIST 25
#define WALL_DIST 15 

int wall_count = 0;
#define TOTAL_WALLS 4

// States
int state = 0;
#define SEARCHING_WALL 0
#define WALL_FINDED 1
#define END 2

// States of SEARCHING_WALL
int search_wall_state = 0;
#define MOVE 0
#define OBJ_DECT 1
#define AVOID 2

// Time vars
float init_time = -1;

bool end_avoid = false;
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
            return "UNKNOWN";
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

  obj_det.distR = ultrasoundR.get_dist();
  obj_det.distL = ultrasoundL.get_dist();

  Serial.print("distR = ");
  Serial.println(obj_det.distR);

  Serial.print("distL = ");
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

  servo_right.attach(SERVO_R_PIN);
  servo_left.attach(SERVO_L_PIN);
}

/*-----------------------------------------------------------------------------*/
// FUNCIONES MOVIMIENTO

void girar_90_izq() {
  servo_right.write(180);
  servo_left.write(180);
  delay(1550);
  servo_right.write(90);
  servo_left.write(90);
}

void girar_45_izq() {
  servo_right.write(180);
  servo_left.write(180);
  delay(775);
  servo_right.write(90);
  servo_left.write(90);
}

void girar_90_dch() {
  servo_right.write(0);
  servo_left.write(0);
  delay(1550);
  servo_right.write(90);
  servo_left.write(90);
}

void girar_45_dch() {
  servo_right.write(0);
  servo_left.write(0);
  delay(775);
  servo_right.write(90);
  servo_left.write(90);
}

void stop() {
  servo_right.write(90);
  servo_left.write(90);
}

void avanzar_recto() {  // 8 rojo a la derecha
  servo_right.write(180);
  servo_left.write(0);
}

/*-----------------------------------------------------------------------------*/

void loop() {
  // --- Giro hacia adelante ---
  // servo_right.write(180);  // 0 = máxima velocidad hacia adelante (ajusta si necesario)
  // servo_left.write(0);

  // check_dist();

  delay(1000);


  if (exercise_1) {

  } else if (exercise_2){
    // Switch controller
    switch(state) {
      case SEARCHING_WALL:
      {
        switch(search_wall_state) {
          case MOVE:
          {
            // Forward

            // Check if an object has been detected
            object = check_dist();
            if (object.place != NINGUNO_DETECTA) {
              // Stop
              //state = OBJ_DECT;
              
              init_time = millis();
              Serial.print("Objeto detectado en ");
              Serial.println(get_action_name(object.place));

              if (object.distL <= WALL_DIST || object.distR <= WALL_DIST) {
                // Backward
                
              } else {
                //search_wall_state = AVOID;
              }

            }
            break;
          }

          case AVOID:
          {
            if (object.place == IZQ_DETECTA) {
              // AVOID
              // Rotar drch
              
            } else if (object.place == DERECHA_DETECTA) {
              // Comprobar der -> se puede encontrar pared
              // Esquivar der
            } else if (object.place == AMBOS_DETECTAN) {
              // Comprobar pared -> se puede encontrar pared
              // Esquivar izq
            }

            if (end_avoid) {
              object = check_dist();

              if (object.place != AMBOS_DETECTAN) {
                if (object.distL <= WALL_DIST || object.distR <= WALL_DIST) {
                  state = WALL_FINDED;
                } else {
                  // APROXIMARSE
                }
              } else if (object.place != NINGUNO_DETECTA) {
                end_avoid = false;
              }
            }
            break;
          }

        break;
      }

      case WALL_FINDED:
      {
        wall_count++;
        //back
        // turn
        if (wall_count == TOTAL_WALLS){
          state = END;
        } else {
          state = SEARCHING_WALL;
        }

        break;
      }

      case END:
      {
        // Parar
        // Indicar
        break;
      }
    }
    break;
  }
  }
}

