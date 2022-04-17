#pragma once

#include "GL/dynamic_object.hpp"
#include "aircraft.hpp"
#include "geometry.hpp"

#include <cassert>

class Terminal : public GL::DynamicObject
{
private:
    double service_progress    = SERVICE_CYCLES;
    Aircraft* booked_in_aircraft = nullptr;
    const Point3D pos;

public:
    Terminal(const Terminal&) = delete;
    Terminal& operator=(const Terminal&) = delete;
    explicit Terminal(const Point3D& pos_) : pos { pos_ } {}

    [[nodiscard]] bool in_use() const { return booked_in_aircraft != nullptr; }
    [[nodiscard]] bool is_servicing() const {
        return service_progress < SERVICE_CYCLES
        || (booked_in_aircraft != nullptr && booked_in_aircraft->is_low_on_fuel());
    }
    void assign_craft(Aircraft& aircraft) { booked_in_aircraft = &aircraft; }

    void start_service(const Aircraft& aircraft)
    {
        assert(aircraft.distance_to(pos) < DISTANCE_THRESHOLD);
        if (!SILENT_TERMINAL)
            std::cout << "now servicing " << aircraft.get_flight_num() << "...\n";
        service_progress = 0;
    }

    void finish_service()
    {
        assert(booked_in_aircraft != nullptr);
        if (is_servicing()) return;
        if (!SILENT_TERMINAL) std::cout << "done servicing " << booked_in_aircraft->get_flight_num() << '\n';
        booked_in_aircraft = nullptr;
    }

    void refill_aircraft_if_needed(unsigned& fuel_stock) {
        if (booked_in_aircraft != nullptr && booked_in_aircraft->is_low_on_fuel())
            booked_in_aircraft->refill(fuel_stock);
    }

    void move(double dt) override
    {
        if (in_use() && is_servicing())
        {
            service_progress += dt;
        }
    }
    void on_aircraft_crash(const Aircraft& aircraft) {
        if (&aircraft == booked_in_aircraft) {
            booked_in_aircraft = nullptr;
        }
    }
};