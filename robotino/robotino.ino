#include <Servo.h>
#include "ultrasonido.h"

#define SERVO_R_PIN 8     // Pin de señal del servo
#define SERVO_L_PIN 9

Servo servo_right;  // Objeto servo
Servo servo_left;
//#define TIEMPO_GIRO OBST_DIST00  // Tiempo en milisegundos (2 segundos)

// LED rojo -- pin 33
#define LED_R 33
// LED azul -- pin 35
#define LED_B 35


#define ECHO_PIN_L 35
#define TRIG_PIN_L 39
UltraSoundClass ultrasoundL(ECHO_PIN_L, TRIG_PIN_L);

#define ECHO_PIN_R 45
#define TRIG_PIN_R 47
UltraSoundClass ultrasoundR(ECHO_PIN_R, TRIG_PIN_R);

bool exercise_1 = false;
bool exercise_2 = true;


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
#define OBJ_DECT 1
#define AVOID 2

// Time vars
float init_time = -1;

int wall_count = 0;
#define TOTAL_WALLS 4

// Variables a cambiar
#define OBST_DIST 25
#define WALL_DIST 15 

#define MOVING_TIME 1500



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


void retroceder_recto() {  // 8 rojo a la derecha
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

/*-----------------------------------------------------------------------------*/

void loop() {
  // --- Giro hacia adelante ---
  // servo_right.write(180);  // 0 = máxima velocidad hacia adelante (ajusta si necesario)
  // servo_left.write(0);

  // check_dist();

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
            avanzar_recto();

            // Check if an object has been detected
            object = check_dist();
            if (object.place != NINGUNO_DETECTA) {
              // Stop
              stop();
              //state = OBJ_DECT;
              
              init_time = millis();
              Serial.print("Objeto detectado en ");
              Serial.println(get_action_name(object.place));

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
              // AVOID
              // Rotar drch
              girar_45_dch();
              avanzar_recto();
              delay(MOVING_TIME);
              girar_45_izq();

              
            } else if (object.place == DERECHA_DETECTA) {
              // Comprobar der -> se puede encontrar pared
              // Esquivar der
              girar_45_izq();
              avanzar_recto();
              delay(MOVING_TIME);
              girar_45_dch();

            } else if (object.place == AMBOS_DETECTAN) {
              // Comprobar pared -> se puede encontrar pared
              // Esquivar izq
              girar_45_dch();
              avanzar_recto();
              delay(MOVING_TIME);
              girar_45_izq();
            }

            // Despues de esquivar comporbamos si sigue el obstáculo
            object = check_dist();

            // Si los dos sensores detectan es muro
            if (object.place == AMBOS_DETECTAN) {
              // Si está sufucientemente cerca es muro directamente
              if (object.distL <= WALL_DIST && object.distR <= WALL_DIST) {
                state = WALL_FINDED;

              // Si no se acerca hasta la dist estipulada
              } else {
                // APROXIMARSE
                aproximacion();
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
        // Parar
        // Indicar
        stop();

        break;
      }
    }
    break;
  }
  }
}

