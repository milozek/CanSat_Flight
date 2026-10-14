#include <Arduino.h>

// Definición de los estados operativos obligatorios de la misión
enum MissionState {
  LAUNCH_PAD,
  ASCENT,
  APOGEE,
  DESCENT,
  PROBE_RELEASE,
  PAYLOAD_RELEASE,
  LANDED
};

MissionState currentState = LAUNCH_PAD;
int packetCount = 0; // Se debe guardar en memoria no volátil

void setup() {
  Serial.begin(115200); // Inicializar comunicación serie (USB o Radio)
  
  // Aquí se debe incluir la función para leer la memoria no volátil (EEPROM/Flash)
  // y recuperar 'currentState' y 'packetCount' en caso de reinicio del procesador.
}

void updateStateMachine() {
  switch (currentState) {
    case LAUNCH_PAD:
      // Esperar detección de despegue (ej. aceleración > umbral en eje Z)
      // if (despegueDetectado) currentState = ASCENT;
      break;
      
    case ASCENT:
      // Monitorear presión/altitud. Si empieza a disminuir de forma sostenida:
      // if (altitudMaximaAlcanzada) currentState = APOGEE;
      break;
      
    case APOGEE:
      // Desplegar paracaídas principal (15 m/s)
      // activarMecanismo();
      currentState = DESCENT;
      break;
      
    case DESCENT:
      // Monitorear descenso. Al alcanzar el 80% de la altitud máxima:
      // if (altitud <= altitudMaxima * 0.8) currentState = PROBE_RELEASE;
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

void sendTelemetry() {
  // Ensamblar la cadena ASCII separada por comas y enviarla por la radio
  // Ejemplo: Serial.print("1000,13:14:02," + String(packetCount) + ",F,");
  
  packetCount++; // Incrementar por cada paquete enviado
}

void loop() {
  // 1. Leer y actualizar datos de todos los sensores (GPS, IMU, Barómetro)
  
  // 2. Evaluar transiciones de la misión
  updateStateMachine();
  
  // 3. Enviar telemetría a 1 Hz (se debe usar millis() en lugar de delay)
  sendTelemetry();
  
  // 4. Guardar 'packetCount' y 'currentState' en memoria Flash/NVS
  
  delay(1000); // Placeholder simple para la frecuencia de 1 Hz
}