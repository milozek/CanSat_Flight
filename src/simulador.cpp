#include "simulador.h"
#include <Arduino.h>
#include "telemetria.h" // Incluimos esto para acceder a datosCanSat y a los estados (LAUNCH_PAD, etc.)

// Le avisamos al simulador que estas variables existen físicamente en main.cpp
extern float altitudInicial;
extern float altitudMaxima;
extern bool calibrado;
extern MissionState currentState; // Cambiá "int" por el nombre de tu enum si es distinto (ej. FlightState)

void ejecutarSimuladorFisica()
{
    float altitudSimulada = datosCanSat.altitude - altitudInicial;

    if (currentState == LAUNCH_PAD && calibrado)
    {
        datosCanSat.altitude += 5.0;
    }
    else if (currentState == ASCENT)
    {
        if (altitudMaxima < 50.0)
        {
            datosCanSat.altitude += 5.0;
        }
        else
        {
            datosCanSat.altitude -= 2.0;
        }
    }
    else if (currentState == DESCENT)
    {
        datosCanSat.altitude -= 4.0;
    }
}