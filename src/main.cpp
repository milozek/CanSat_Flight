#include <Arduino.h>
#include "telemetria.h"
#include <Preferences.h> // <- Librería nativa de memoria del ESP32

// Creamos la herramienta para acceder a la memoria
Preferences memoriaNVS;

// // Definición de los estados operativos obligatorios de la misión
// enum MissionState
// {
//   LAUNCH_PAD,
//   ASCENT,
//   APOGEE,
//   DESCENT,
//   PROBE_RELEASE,
//   PAYLOAD_RELEASE,
//   LANDED
// };

MissionState currentState = LAUNCH_PAD;
int packetCount = 0; // Se debe guardar en memoria no volátil

// Variables para controlar la frecuencia de 1 Hz sin usar delay()
unsigned long lastTelemetryTime = 0;
const int TELEMETRY_INTERVAL = 1000; // 1000 milisegundos = 1 segundo

// -- NUEVAS VARIABLES PARA COMANDOS --
bool telemetryEnabled = false; // Controla si enviamos datos o no (inicia apagado hasta recibir orden)
String cmdEcho = "NONE";       // Guarda el último comando recibido sin comas para la telemetría
float altitudMaxima = 0.0;     // Guarda el pico más alto alcanzado
int contadorDescenso = 0;      // Cuenta cuántas veces seguidas la altitud bajó
// SensorData datosCanSat;        // Hacemos la caja accesible para todo el programa

// --- PANEL DE CONFIGURACIÓN DE VUELO ---
const float UMBRAL_DESPEGUE = 15.0;    // Metros sobre la plataforma para detectar despegue
const float UMBRAL_CAIDA_APOGEO = 3.0; // Metros que debe caer desde el pico para ser apogeo
const int UMBRAL_MUESTRAS_APOGEO = 3;  // Cuántas veces seguidas debe registrar caída (anti-ruido)
const float UMBRAL_ATERRIZAJE = 5.0;   // Metros mínimos para considerar que ya aterrizó
float altitudInicial = 0.0;            // Guardará la altitud de la plataforma
bool calibrado = false;                // Candado de seguridad
// ---------------------------------------

// Función de ayuda para convertir el estado (enum) a texto ASCII
String getStateString(MissionState state)
{
  switch (state)
  {
  case LAUNCH_PAD:
    return "LAUNCH_PAD";
  case ASCENT:
    return "ASCENT";
  case APOGEE:
    return "APOGEE";
  case DESCENT:
    return "DESCENT";
  case PROBE_RELEASE:
    return "PROBE_RELEASE";
  case PAYLOAD_RELEASE:
    return "PAYLOAD_RELEASE";
  case LANDED:
    return "LANDED";
  default:
    return "UNKNOWN";
  }
}

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

void processCommand(String rawCommand)
{
  // 1. Guardar el comando para la variable CMD_ECHO eliminando las comas
  cmdEcho = rawCommand;
  cmdEcho.replace(",", " "); // Cambia las comas por espacios para cumplir el reglamento

  // 2. Extraer las partes del comando buscando las posiciones de las comas
  int firstComma = rawCommand.indexOf(',');
  int secondComma = rawCommand.indexOf(',', firstComma + 1);
  int thirdComma = rawCommand.indexOf(',', secondComma + 1);

  // Asegurarnos de que el comando tenga la estructura mínima CMD,TEAM_ID,TIPO
  if (firstComma > 0 && secondComma > 0)
  {
    String header = rawCommand.substring(0, firstComma);
    String teamID = rawCommand.substring(firstComma + 1, secondComma);

    // Si no hay una tercera coma, el comando termina en la segunda (ej. CMD,1000,CAL)
    String cmdType = rawCommand.substring(secondComma + 1, thirdComma > 0 ? thirdComma : rawCommand.length());

    // 3. Verificar que el mensaje sea para nosotros
    if (header == "CMD" && teamID == "1000")
    {

      // Comando CX: Activar o desactivar telemetría
      if (cmdType == "CX" && thirdComma > 0)
      {
        String state = rawCommand.substring(thirdComma + 1);
        if (state == "ON")
        {
          telemetryEnabled = true;
        }
        else if (state == "OFF")
        {
          telemetryEnabled = false;
        }
      }
      // Comando RST: Reinicio de emergencia por software
      else if (cmdType == "RST")
      {
        Serial.println("Iniciando reinicio de emergencia...");
        delay(500);    // Pequeña pausa para que llegue a leerse el mensaje por radio
        ESP.restart(); // Comando nativo que reinicia el chip instantáneamente
      }
      // Comando CAL: Calibrar Altitud a cero
      else if (cmdType == "CAL")
      {
        // Por seguridad, solo permitimos calibrar si el cohete está en la plataforma
        if (currentState == LAUNCH_PAD)
        {
          altitudInicial = datosCanSat.altitude;
          calibrado = true;
          Serial.println(">>> CALIBRACION: Altitud base seteada en " + String(altitudInicial) + " m <<<");
        }
        else
        {
          Serial.println(">>> ERROR: Solo se puede calibrar en estado LAUNCH_PAD <<<");
        }
        // altitudeOffset = currentPressureAltitude; (Lógica a implementar con el barómetro)
      }
      // Comando ST: Configurar el reloj
      else if (cmdType == "ST" && thirdComma > 0)
      {
        String timeValue = rawCommand.substring(thirdComma + 1);
        // missionTime = timeValue; (Lógica a implementar con el módulo GPS)
      }
      // Comando MEC: Activar Mecanismos
      else if (cmdType == "MEC" && thirdComma > 0)
      {
        // Lógica para encender un relé, cortar un hilo de nicrom o mover un servo
      }
    }
  }
}

void receiveCommands()
{
  // Verificar si hay datos llegando por la radio/USB
  if (Serial.available() > 0)
  {
    // Leer todo el texto hasta que el comando termine con un "Enter" (salto de línea)
    String incomingData = Serial.readStringUntil('\n');
    incomingData.trim(); // Limpiar espacios invisibles al principio y al final

    if (incomingData.length() > 0)
    {
      processCommand(incomingData);
    }
  }
}

void updateStateMachine()
{
  switch (currentState)
  {
  case LAUNCH_PAD:
    // DETECCIÓN DE DESPEGUE: Si sube 15 metros, asumimos que despegó
    if (calibrado && datosCanSat.altitude > UMBRAL_DESPEGUE)
    {
      currentState = ASCENT;
      Serial.println(">>> EVENTO: DESPEGUE DETECTADO - MODO ASCENSO <<<");
    }
    break;

  case ASCENT:
    // Monitorear presión/altitud. Si empieza a disminuir de forma sostenida:
    // 1. Rastrear siempre el punto más alto
    if (datosCanSat.altitude > altitudMaxima)
    {
      altitudMaxima = datosCanSat.altitude;
      contadorDescenso = 0; // Reiniciar contador de seguridad porque sigue subiendo
    }
    // 2. DETECCIÓN DE APOGEO: Si cae más de 3 metros respecto al máximo
    else if (datosCanSat.altitude < (altitudMaxima - UMBRAL_CAIDA_APOGEO))
    {
      contadorDescenso++; // Anotamos una caída

      // 3. Confirmación anti-ruido: Solo si cae 3 veces seguidas (3 segundos)
      if (contadorDescenso >= UMBRAL_MUESTRAS_APOGEO)
      {
        currentState = APOGEE;
        Serial.println(">>> EVENTO: APOGEO DETECTADO - CÚPULA ALCANZADA <<<");
      }
    }
    break;

  case APOGEE:
    // Desplegar paracaídas principal (15 m/s)
    // activarMecanismo();
    currentState = DESCENT;
    Serial.println(">>> EVENTO: PARACAÍDAS PRINCIPAL DESPLEGADO <<<");
    break;

  case DESCENT:
    // Monitorear descenso. Al alcanzar el 80% de la altitud máxima:
    // if (altitud <= altitudMaxima * 0.8) currentState = PROBE_RELEASE;
    // DETECCIÓN DE ATERRIZAJE: Altitud cercana a cero y sin cambios
    if (datosCanSat.altitude < UMBRAL_ATERRIZAJE)
    {
      currentState = LANDED;
      Serial.println(">>> EVENTO: ATERRIZAJE CONFIRMADO <<<");
    }
    break;

  case PROBE_RELEASE:
    // Liberar el segundo sistema de descenso (5 m/s)
    currentState = PAYLOAD_RELEASE;
    break;

  case PAYLOAD_RELEASE:
    // Continuar monitoreando hasta tocar tierra
    // if (altitudCercanaACero && noHayMovimiento) currentState = LANDED;
    break;

  case LANDED:
    // Detener telemetría de alta frecuencia y asegurar encendido de baliza de audio
    break;
  }
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

      datosCanSat.altitude += 5.0;

      sendTelemetry(); // Armar y enviar el paquete

      // Guardar el nuevo número en el disco duro interno al instante
      memoriaNVS.putInt("paquetes", packetCount);
      // memoriaNVS.putInt("estado", currentState); // (Lo habilitaremos cuando cambien los estados)
    }
  }
}