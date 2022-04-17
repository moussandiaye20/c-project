#pragma once

#include "GL/dynamic_object.hpp"
#include "AircraftManager.hpp"
#include "GL/displayable.hpp"
#include "airport_type.hpp"
#include "GL/texture.hpp"
#include "img/image.hpp"
#include "geometry.hpp"
#include "terminal.hpp"
#include "runway.hpp"
#include "tower.hpp"

#include <vector>

class Airport : public GL::Displayable, public GL::DynamicObject
{
private:
    const AirportType& type;
    const Point3D pos;
    const GL::Texture2D texture;
    std::vector<Terminal> terminals;
    AircraftManager& manager;
    Tower tower;
    unsigned fuel_stock = 0;
    unsigned ordered_fuel = 0;
    double next_refill_time = 0;

    // reserve a terminal
    // if a terminal is free, return
    // 1. a sequence of waypoints reaching the terminal from the runway-end and
    // 2. the number of the terminal (used for liberating the terminal later)
    // otherwise, return an empty waypoint-vector and any number
    std::pair<WaypointQueue, size_t> reserve_terminal(Aircraft& aircraft)
    {
        const auto it = std::find_if(terminals.begin(), terminals.end(),
                [](const Terminal& t) { return !t.in_use(); });

        if (!(it != terminals.end())) return { {}, 0u };                     // If no terminal left -> Empty queue and terminal

        it->assign_craft(aircraft);                                             // Assign craft
        const auto term_idx = std::distance(terminals.begin(), it);         // Get the terminal's id
        return { type.air_to_terminal(pos, 0, term_idx), term_idx };    // Return a path to reach the terminal
    }

    WaypointQueue start_path(const size_t terminal_number)
    {
        assert(terminal_number < terminals.size());
        return type.terminal_to_air(pos, 0, terminal_number);
    }

    Terminal& get_terminal(const size_t terminal_number) {
        assert(terminal_number < terminals.size());
        return terminals.at(terminal_number);
    }

    void refuel_all(double dt) {
        assert(dt > 0);
        if (next_refill_time <= 0) {
            const auto old = ordered_fuel;
            fuel_stock += ordered_fuel;
            ordered_fuel = std::min(FUEL_TANKER, manager.get_required_fuel());
            next_refill_time = FUEL_REFILL_FREQUENCY;
            std::cout << "Received : " << old << " | Stock : " << fuel_stock << " | Ordered : " << ordered_fuel << std::endl;
        } else {
            next_refill_time -= dt;
        }
        std::for_each(terminals.begin(), terminals.end(), [this](Terminal& t){t.refill_aircraft_if_needed(fuel_stock);});
    }

public:
    ~Airport() override = default;
    Airport(const Airport&) = delete;
    Airport& operator=(const Airport&) = delete;
    Airport(const AirportType& type_, const Point3D& pos_, const img::Image* image, AircraftManager& _manager,
            const float z_ = 1.0f) :
        GL::Displayable { z_ },
        type { type_ },
        pos { pos_ },
        texture { image },
        terminals { type.create_terminals() },
        manager {_manager},
        tower { *this }
    {}

    Tower& get_tower() { return tower; }

    void display() const override { texture.draw(project_2D(pos), { 2.0f, 2.0f }); }

    void move(double dt) override
    {
        assert(dt);
        std::for_each(terminals.begin(), terminals.end(), [dt](Terminal& t){t.move(dt);});
        refuel_all(dt);
    }

    void on_aircraft_crash(const Aircraft& aircraft) {
        for (auto& terminal : terminals) {
            terminal.on_aircraft_crash(aircraft);
        }
    }

    friend class Tower;
};
