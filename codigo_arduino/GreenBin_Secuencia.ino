// Sistema de control del actuador de la puerta (motores de puente H) / Gate actuator control system (H-Bridge motors)
// Definición de pines para el primer puente H
const int RPWM = 32;
const int R_EN = 30;
const int LPWM = 33;
const int L_EN = 31;

// Pines para los finales de carrera del primer puente H
const int finalCarreraAdelante = 34;
const int finalCarreraAtras = 35;

// Definición de pines para el segundo puente H
const int RPWM2 = 38;
const int R_EN2 = 40;
const int LPWM2 = 39;
const int L_EN2 = 41;

// Pines para los finales de carrera del segundo puente H
const int finalCarreraAdelante2 = 42;
const int finalCarreraAtras2 = 43;

// Variables de estado del actuador de puerta / Gate actuator state variables
String DireccionMotor = "CW";
bool MotorActivado = false;

String DireccionMotor2 = "CW";
bool MotorActivado2 = false;

// Pines del controlador del motor paso a paso / Stepper motor driver pins
const int PUL = 5;  // Pulso
const int DIR = 6;  // Dirección
const int EN = 7;   // Habilitación

int posicionActual = 0; // Posición actual del motor paso a paso

// Gira el sistema de compartimentos hasta la posición objetivo / Rotate the compartment system to the target disposal position
void moverMotorAPosicion(int nuevaPosicion)
{
  // Calcula el número de pasos necesarios para alcanzar la posición objetivo / Calculate the number of steps required to reach the target position
  int diferenciaPasos = nuevaPosicion - posicionActual;

  if (diferenciaPasos > 0)
  {
    digitalWrite(DIR, LOW); // Dirección hacia adelante
  } else if (diferenciaPasos < 0) 
  {
    diferenciaPasos = -diferenciaPasos;
    digitalWrite(DIR, LOW);
  } else 
  {
    return; // Ya está en la posición deseada
  }

  // Genera señales de pulso para el controlador del motor paso a paso / Generate pulse signals for the stepper motor driver
  for (int i = 0; i < diferenciaPasos; i++) 
  {
    digitalWrite(PUL, HIGH);
    delayMicroseconds(8000);
    digitalWrite(PUL, LOW);
    delayMicroseconds(8000);
  }
 posicionActual = nuevaPosicion;
}

// Controla la dirección de apertura y cierre de los actuadores de la puerta / Control the opening and closing direction of the gate actuators
void motorControl(String direction, bool enable, int rpwm, int lpwm, int ren, int len)
{
  if (enable) 
  {
    if (direction == "CW") 
    {
      digitalWrite(rpwm, HIGH);
      digitalWrite(lpwm, LOW);
      digitalWrite(ren, HIGH);
      digitalWrite(len, HIGH);
    } else if (direction == "CCW") 
    {
      digitalWrite(rpwm, LOW);
      digitalWrite(lpwm, HIGH);
      digitalWrite(ren, HIGH);
      digitalWrite(len, HIGH);
    }
  } else 
  {
    digitalWrite(rpwm, LOW);
    digitalWrite(lpwm, LOW);
    digitalWrite(ren, LOW);
    digitalWrite(len, LOW);
  }
}

// Inicialización del sistema / System initialization
void setup() 
{
  pinMode(RPWM, OUTPUT);
  pinMode(R_EN, OUTPUT);
  pinMode(LPWM, OUTPUT);
  pinMode(L_EN, OUTPUT);
  pinMode(RPWM2, OUTPUT);
  pinMode(R_EN2, OUTPUT);
  pinMode(LPWM2, OUTPUT);
  pinMode(L_EN2, OUTPUT);

  pinMode(finalCarreraAdelante, INPUT_PULLUP);
  pinMode(finalCarreraAtras, INPUT_PULLUP);
  pinMode(finalCarreraAdelante2, INPUT_PULLUP);
  pinMode(finalCarreraAtras2, INPUT_PULLUP);

  digitalWrite(RPWM, LOW);
  digitalWrite(R_EN, LOW);
  digitalWrite(LPWM, LOW);
  digitalWrite(L_EN, LOW);
  digitalWrite(RPWM2, LOW);
  digitalWrite(R_EN2, LOW);
  digitalWrite(LPWM2, LOW);
  digitalWrite(L_EN2, LOW);

  pinMode(PUL, OUTPUT);
  pinMode(DIR, OUTPUT);
  pinMode(EN, OUTPUT);
  digitalWrite(EN, HIGH);
  // Inicializar la comunicación serial con el Jetson Nano / Initialize serial communication with the Jetson Nano
  Serial.begin(9600);
  Serial.println("Sistema listo. Enviar 1, 2, 3 o 4 para iniciar la secuencia.");
}

void loop() 
{
  // Wait for a waste classification command from the Jetson Nano / Wait for a waste classification command from the Jetson Nano
  if (Serial.available() > 0) 
  {
    char comando = Serial.read();
    int pasosObjetivo = 0;

    // Asigna una posición objetivo para cada categoría de residuos / Assign a target position for each waste category
    switch (comando) 
    {
      case '1': pasosObjetivo = 1911; break;
      case '2': pasosObjetivo = 1274; break;
      case '3': pasosObjetivo = 637; break;
      case '4': pasosObjetivo = 2548; break;
      default:
        Serial.println("Comando no válido. Enviar 1, 2, 3 o 4.");
        return;
    }

    Serial.print("Moviendo a ");
    Serial.print(pasosObjetivo);
    Serial.println(" pasos.");
    // Gira el sistema de compartimentos a la posición de categoría seleccionada / Rotate the compartment system to the selected category position
    moverMotorAPosicion(pasosObjetivo);

    Serial.println("Abriendo compuertas...");
    DireccionMotor = "CW";
    MotorActivado = true;
    DireccionMotor2 = "CW";
    MotorActivado2 = true;

    // Abrir el sistema de puertas hasta que se activen los interruptores de límite / Open the door system until the limit switches are activated
    while (MotorActivado || MotorActivado2) 
    {
      if (MotorActivado && digitalRead(finalCarreraAdelante) == LOW) 
      {
        MotorActivado = false;
        Serial.println("Compuerta 1 abierta completamente.");
      }
      if (MotorActivado2 && digitalRead(finalCarreraAdelante2) == LOW) 
      {
        MotorActivado2 = false;
        Serial.println("Compuerta 2 abierta completamente.");
      }
      motorControl(DireccionMotor, MotorActivado, RPWM, LPWM, R_EN, L_EN);
      motorControl(DireccionMotor2, MotorActivado2, RPWM2, LPWM2, R_EN2, L_EN2);
    }

    Serial.println("Cerrando compuertas...");
    DireccionMotor = "CCW";
    MotorActivado = true;
    DireccionMotor2 = "CCW";
    MotorActivado2 = true;

    // Cierra el sistema de puertas hasta que se activen los interruptores de límite / Close the gate system until the limit switches are activated
    while (MotorActivado || MotorActivado2) 
    {
      if (MotorActivado && digitalRead(finalCarreraAtras) == LOW) 
      {
        MotorActivado = false; // Detener el motor al llegar al final
        Serial.println("Compuerta 1 cerrada completamente.");
      }
      if (MotorActivado2 && digitalRead(finalCarreraAtras2) == LOW) 
      {
        MotorActivado2 = false;
        Serial.println("Compuerta 2 cerrada completamente.");
      }
      motorControl(DireccionMotor, MotorActivado, RPWM, LPWM, R_EN, L_EN);
      motorControl(DireccionMotor2, MotorActivado2, RPWM2, LPWM2, R_EN2, L_EN2);
    }

    Serial.println("Completando la vuelta a 2550 pasos...");
    // Devuelve el sistema giratorio a la posición inicial / Return the rotating system to the home position
    moverMotorAPosicion(2550);

    // Avisar a la Nano Jetson que la secuencia ha terminado / Notify the Jetson Nano that the sorting sequence has finished
    Serial.println("Secuencia completada. Enviando confirmación a la Nano Jetson...");
    Serial.println("FIN");

    Serial.println("Esperando nueva orden...");
  }
}
