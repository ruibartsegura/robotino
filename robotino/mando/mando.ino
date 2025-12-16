#include <IRremote.hpp>
int pinReceptor = 2;

void setup()
{
  Serial.begin(9600);
  IrReceiver.begin(pinReceptor, ENABLE_LED_FEEDBACK); // INICIA LA RECEPCIÓN
  Serial.println("Listo para comenzar a recibir");
}

void loop()
{
  if (IrReceiver.decode())
  {
    Serial.println(IrReceiver.decodedIRData.decodedRawData, HEX);
    //IrReceiver.printIRResultShort(&Serial); // una mejora de la ultima versión de la librería
    IrReceiver.resume();
  }
}

// boton 1 2FFA00
// boton 2 2EFA20
// boton 3 2DFA4ó0
