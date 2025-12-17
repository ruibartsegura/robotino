#include <Servo.h>
#include "ultrasonido.h"

#define SERVO_R_PIN 8     // Pin de señal del servo
#define SERVO_L_PIN 9
//#define TIEMPO_GIRO OBST_DIST00  // Tiempo en milisegundos (2 segundos)

Servo servo_right;  // Objeto servo
Servo servo_left;

#define ECHO_PIN_L 51
#define TRIG_PIN_L 53
UltraSoundClass ultrasoundL(ECHO_PIN_L, TRIG_PIN_L);

#define ECHO_PIN_R 25
#define TRIG_PIN_R 27
UltraSoundClass ultrasoundR(ECHO_PIN_R, TRIG_PIN_R);

bool exercise_1 = false;
bool exercise_2 = false;


int AMBOS_DETECTAN = 0;
int DERECHA_DETECTA = 1;
int IZQ_DETECTA = 2;
int NINGUNO_DETECTA = 3;

#define OBST_DIST 20 // Probar

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


/*-----------------------------------------------------------------------------*/

int check_dist() {
  float distR = ultrasoundR.get_dist();
  float distL = ultrasoundL.get_dist();

  Serial.print("distR = ");
  Serial.println(distR);

  Serial.print("distL = ");
  Serial.println(distL);


  if(distL <= OBST_DIST && distR <= OBST_DIST) {
    return AMBOS_DETECTAN;

  } else if (distL <= OBST_DIST && distR > OBST_DIST) {
    return IZQ_DETECTA;

  } else if (distL > OBST_DIST && distR <= OBST_DIST) {
    return DERECHA_DETECTA;

  } else {
    return NINGUNO_DETECTA;
  }

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
  servo_right.write(180);  // 0 = máxima velocidad hacia adelante (ajusta si necesario)
  servo_left.write(0);

  check_dist();

  delay(100);


  if (exercise_1) {

  } else if (exercise_2){
    // Switch controller
    switch(state) {
      case SEARCHING_WALL:
      {
        switch(search_wall_state) {
          case MOVE:
          {
            break;
          }

          case OBJ_DECT:
          {
            break;
          }

          case AVOID:
          {
            break;
          }

        break;
      }

      case WALL_FINDED:
      {
        break;
      }

      case END:
      {
        break;
      }
  }
}
