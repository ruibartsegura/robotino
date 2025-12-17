#include <IRremote.hpp>
#include <Servo.h>
#include "ultrasonido.h"
int pinReceptor = 2;

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
#define OBST_DIST 20   // distancia límite en cm

struct obj_detection {
    int place;
    float distR;
    float distL;
};

// To know where the object was detected and react according to it
struct obj_detection object;

int LED = 4;


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


void setup()
{
  Serial.begin(9600);
  IrReceiver.begin(pinReceptor, ENABLE_LED_FEEDBACK); // INICIA LA RECEPCIÓN
  Serial.println("Listo para comenzar a recibir");

  servo_right.attach(SERVO_R_PIN);
  servo_left.attach(SERVO_L_PIN);
  
  pinMode(LED, OUTPUT);
}

void loop()
{
  if (IrReceiver.decode())
  {
    Serial.println(IrReceiver.decodedIRData.decodedRawData, HEX);
    //IrReceiver.printIRResultShort(&Serial); // una mejora de la ultima versión de la librería
    IrReceiver.resume();

    if (IrReceiver.decodedIRData.decodedRawData == 0x2FFA00) {
      servo_right.write(180);
      servo_left.write(0);

    } else if (IrReceiver.decodedIRData.decodedRawData == 0x2EFA20) {
      servo_right.write(0);
      servo_left.write(0);

    } else if (IrReceiver.decodedIRData.decodedRawData == 0x2DFA40) {
      servo_right.write(90);
      servo_left.write(90);
    }

  }

  object = check_dist();
  if (object.place != NINGUNO_DETECTA) {
    digitalWrite(LED, HIGH);
  } else {
    digitalWrite(LED, LOW);
  }
}

// boton 1 2FFA00
// boton 2 2EFA20
// boton 3 2DFA40
