#ifndef TELEMETRIA_H
#define TELEMETRIA_H

#include <Arduino.h>

// Definición compartida del enum
enum MissionState
{
    LAUNCH_PAD,
    ASCENT,
    APOGEE,
    DESCENT,
    PROBE_RELEASE,
    PAYLOAD_RELEASE,
    LANDED
};

// Creamos nuestra "caja" de variables de sensores
struct SensorData
{
    float altitude = 0.0;
    float temperature = 0.0;
    float pressure = 0.0;
    float voltage = 0.0;
    float current = 0.0;
    float gyroR = 0.0, gyroP = 0.0, gyroY = 0.0;
    float accelR = 0.0, accelP = 0.0, accelY = 0.0;
    String gpsTime = "00:00:00";
    float gpsAltitude = 0.0;
    float gpsLat = 0.0;
    float gpsLon = 0.0;
    int gpsSats = 0;
};

// "extern" le dice al compilador: "Estas variables existen, búscalas en main.cpp"
extern MissionState currentState;
extern int packetCount;
extern String cmdEcho;
extern SensorData datosCanSat; // Hacemos la caja accesible para todo el programa

// Aquí solo "anunciamos" que esta función va a existir
void sendTelemetry();
String getStateString(MissionState state);

#endif