#ifndef CITY_SETUP_H
#define CITY_SETUP_H

#include <vector>
#include "Bus.h"
#include "Graph.h"
#include "Train.h"

void buildNovaCityGraph(Graph& graph);
std::vector<Bus> createDefaultBuses();
std::vector<Train> createDefaultTrains();

#endif
