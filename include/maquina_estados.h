#ifndef MAQUINA_ESTADOS_H
#define MAQUINA_ESTADOS_H

#include <Arduino.h>
#include "telemetria.h" // Lo necesitamos para que conozca el enum MissionState

String getStateString(MissionState state);
void updateStateMachine();

#endif