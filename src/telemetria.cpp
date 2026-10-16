#include "telemetria.h" // Conecta este archivo con su índice

// Convierte los milisegundos del ESP32 a un reloj real HH:MM:SS
String getMissionTime()
{
    unsigned long totalSeconds = millis() / 1000;
    int hours = (totalSeconds / 3600) % 24;
    int minutes = (totalSeconds / 60) % 60;
    int seconds = totalSeconds % 60;

    char timeBuffer[9]; // Espacio para "HH:MM:SS" y el carácter nulo final
    snprintf(timeBuffer, sizeof(timeBuffer), "%02d:%02d:%02d", hours, minutes, seconds);

    return String(timeBuffer);
}

void sendTelemetry()
{
    // 1. Variables simuladas con las resoluciones pedidas por el reglamento
    String teamID = "1000";                // Identificación del equipo de 4 dígitos
    String missionTime = getMissionTime(); // Hora UTC
    String mode = "F";                     // 'F' para vuelo, 'S' para simulación
    String stateStr = getStateString(currentState);

    // float altitude = 150.5;   // Resolución 0.1 metros
    // float temperature = 22.3; // Resolución 0.1 grados
    // float pressure = 101.3;   // Resolución 0.1 kPa
    // float voltage = 8.4;      // Resolución 0.1 voltios
    // float current = 0.15;     // Resolución 0.01 amperios

    // float gyroR = 0.1, gyroP = -0.2, gyroY = 0.0;
    // float accelR = 0.0, accelP = 0.0, accelY = 9.81;

    // String gpsTime = "13:14:02"; // Hora del receptor GPS
    // float gpsAltitude = 155.0;   // Resolución 0.1 metros
    // float gpsLat = -34.1234;     // Resolución 0.0001 grados
    // float gpsLon = -58.1234;     // Resolución 0.0001 grados
    // int gpsSats = 8;             // Número entero
    // // String cmdEcho = "CXON";           // Último comando recibido

    // 2. Armado de la cadena de texto separada por comas (sin comas extrañas adentro)
    String telemetry = teamID + "," +
                       missionTime + "," +
                       String(packetCount) + "," +
                       mode + "," +
                       stateStr + "," +
                       String(datosCanSat.altitude, 1) + "," +
                       String(datosCanSat.temperature, 1) + "," +
                       String(datosCanSat.pressure, 1) + "," +
                       String(datosCanSat.voltage, 1) + "," +
                       String(datosCanSat.current, 2) + "," +
                       String(datosCanSat.gyroR, 2) + "," + String(datosCanSat.gyroP, 2) + "," + String(datosCanSat.gyroY, 2) + "," +
                       String(datosCanSat.accelR, 2) + "," + String(datosCanSat.accelP, 2) + "," + String(datosCanSat.accelY, 2) + "," +
                       datosCanSat.gpsTime + "," +
                       String(datosCanSat.gpsAltitude, 1) + "," +
                       String(datosCanSat.gpsLat, 4) + "," +
                       String(datosCanSat.gpsLon, 4) + "," +
                       String(datosCanSat.gpsSats) + "," +
                       cmdEcho;

    // 3. Envío por radio/USB con el único carácter de retorno de carro exigido
    Serial.print(telemetry);
    Serial.print("\r"); // Retorno de carro obligatorio
    packetCount++;      // Incrementar por cada paquete enviado
}