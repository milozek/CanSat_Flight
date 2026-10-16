#include "comandos.h"
#include "telemetria.h"
#include <Preferences.h>

// Le indicamos al compilador dónde viven físicamente estas variables
extern bool telemetryEnabled;
extern int packetCount;
extern String cmdEcho;
extern Preferences memoriaNVS;
extern MissionState currentState;
extern float altitudInicial;
extern bool calibrado;
extern SensorData datosCanSat;

// --- CORTÁ Y PEGÁ ACÁ TUS DOS FUNCIONES ---
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
            // Comando WIPE: Borrado total de la memoria de vuelo
            else if (cmdType == "WIPE")
            {
                Serial.println(">>> ALERTA: Borrando disco de memoria interno... <<<");
                memoriaNVS.clear(); // Elimina absolutamente todo dentro del espacio "mision"
                delay(1000);        // Pausa para asegurar que el mensaje de alerta llegue por radio
                ESP.restart();      // Forzamos un reinicio para que el chip despierte en estado puro
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