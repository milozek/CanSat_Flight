#include "maquina_estados.h"

// Traemos mediante puentes las variables que siguen viviendo en main.cpp
extern MissionState currentState;
extern SensorData datosCanSat;
extern bool calibrado;

// --- PANEL DE CONFIGURACIÓN DE VUELO ---
// (Mudado acá porque son exclusivas de las decisiones de vuelo)
const float UMBRAL_DESPEGUE = 15.0;    // Metros sobre la plataforma para detectar despegue
const float UMBRAL_CAIDA_APOGEO = 3.0; // Metros que debe caer desde el pico para ser apogeo
const int UMBRAL_MUESTRAS_APOGEO = 3;  // Cuántas veces seguidas debe registrar caída (anti-ruido)
const float UMBRAL_ATERRIZAJE = 5.0;   // Metros mínimos para considerar que ya aterrizó

// Variables internas de la máquina de estados
float altitudMaxima = 0.0;
int contadorDescenso = 0;
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