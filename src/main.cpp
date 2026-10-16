#include <Arduino.h>
#include "telemetria.h"
#include <Preferences.h> // <- Librería nativa de memoria del ESP32
#include "simulador.h"
#include "comandos.h"
#include "maquina_estados.h"

// Creamos la herramienta para acceder a la memoria
Preferences memoriaNVS;
MissionState currentState = LAUNCH_PAD;
int packetCount = 0; // Se debe guardar en memoria no volátil

// Variables para controlar la frecuencia de 1 Hz sin usar delay()
unsigned long lastTelemetryTime = 0;
const int TELEMETRY_INTERVAL = 1000; // 1000 milisegundos = 1 segundo

// -- NUEVAS VARIABLES PARA COMANDOS --
bool telemetryEnabled = false; // Controla si enviamos datos o no (inicia apagado hasta recibir orden)
String cmdEcho = "NONE";       // Guarda el último comando recibido sin comas para la telemetría
SensorData datosCanSat;        // Hacemos la caja accesible para todo el programa

float altitudInicial = 0.0; // Guardará la altitud de la plataforma
bool calibrado = false;     // Candado de seguridad
// ---------------------------------------

void setup()
{
  Serial.begin(115200); // Inicializar comunicación serie (USB o Radio)
  // Aquí se inicializarán los sensores más adelante
  // Aquí se debe incluir la función para leer la memoria no volátil (EEPROM/Flash)
  // y recuperar 'currentState' y 'packetCount' en caso de reinicio del procesador.

  // Abrir el espacio de memoria llamado "mision" (false = modo lectura/escritura)
  memoriaNVS.begin("mision", false);

  // Recuperar los datos. El '0' es el valor por defecto si no encuentra nada guardado.
  packetCount = memoriaNVS.getInt("paquetes", 0);

  // Recuperamos el estado como un entero y lo convertimos a tu enum MissionState
  int estadoGuardado = memoriaNVS.getInt("estado", 0);
  currentState = static_cast<MissionState>(estadoGuardado);

  // Valores hardcodeados para testear la telemetría
  datosCanSat.altitude = 150.5;
  datosCanSat.temperature = 22.0;
  datosCanSat.pressure = 101.3;
  datosCanSat.gpsLat = -34.5678;
  datosCanSat.gpsLon = -58.1234;
  datosCanSat.gpsSats = 8;
}

void loop()
{
  // Obtener el tiempo actual en milisegundos
  unsigned long currentMillis = millis();

  // 1. Escuchar constantemente la radio por si llegan comandos
  receiveCommands();
  // Evaluar transiciones de la misión
  updateStateMachine();
  if (telemetryEnabled)
  {
    // Enviar telemetría exactamente cada 1000 milisegundos (1 Hz)
    if (currentMillis - lastTelemetryTime >= TELEMETRY_INTERVAL)
    {
      lastTelemetryTime = currentMillis; // Reiniciar el temporizador

      // Módulo de simulación (BORRAR O COMENTAR EL DÍA DEL VUELO REAL)
      ejecutarSimuladorFisica();

      sendTelemetry(); // Armar y enviar el paquete

      // Guardar el nuevo número en el disco duro interno al instante
      memoriaNVS.putInt("paquetes", packetCount);
      memoriaNVS.putInt("estado", currentState); //
    }
  }
}