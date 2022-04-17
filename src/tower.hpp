#pragma once

#include "waypoint.hpp"

#include <algorithm>
#include <unordered_map>
#include <utility>
#include <vector>

class Airport;
class Aircraft;
class Terminal;

class Tower
{
private:
    using AircraftToTerminal = std::unordered_map<const Aircraft*, size_t>;

    Airport& airport;
    // aircrafts may reserve a terminal
    // if so, we need to save the terminal number in order to liberate it when the craft leaves
    AircraftToTerminal reserved_terminals = {};

    static WaypointQueue get_circle();
    WaypointQueue instruction_aux(Aircraft&);
public:
    ~Tower() = default;
    Tower(const Tower&) = delete;
    Tower& operator=(const Tower&) = delete;
    explicit Tower(Airport& airport_) : airport { airport_ } {}

    // produce instructions for aircraft
    WaypointQueue get_instructions(Aircraft& aircraft);
    void arrived_at_terminal(const Aircraft& aircraft);
    WaypointQueue reserve_terminal(Aircraft& aircraft);
    void on_aircraft_crash(const Aircraft& aircraft);
};
