#pragma once

#include <memory>
#include <string>

struct AircraftType;

#include "airport.hpp"
#include "AircraftManager.hpp"
#include "AircraftFactory.h"

class TowerSimulation
{
private:
    bool help        = false;
    std::unique_ptr<Airport> airport;
    std::unique_ptr<AircraftManager> aircraft_manager;
    std::unique_ptr<AircraftFactory> aircraft_factory;

    std::string data_path;

    void create_random_aircraft();

    void create_keystrokes();
    static void display_help() ;
    void display_airline(unsigned);

    void init_airport();
public:
    ~TowerSimulation() = default;
    TowerSimulation(int argc, char** argv);
    TowerSimulation(const TowerSimulation&) = delete;
    TowerSimulation& operator=(const TowerSimulation&) = delete;

    void launch();
};
