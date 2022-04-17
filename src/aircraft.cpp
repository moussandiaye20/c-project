#include "aircraft.hpp"

#include "GL/opengl_interface.hpp"
#include "aircraftCrash.hpp"

#include <cmath>


Aircraft::~Aircraft() {
    control.on_aircraft_crash(*this);
}

void Aircraft::turn_to_waypoint()
{
    if (!waypoints.empty())
    {
        Point3D target = waypoints[0];
        if (waypoints.size() > 1)
        {
            const float d   = (waypoints[0] - pos).length();
            const Point3D W = (waypoints[0] - waypoints[1]).normalize(d / 2.0f);
            target += W;
        }
        auto t = target - pos - speed;
        turn(t);
    }
}

void Aircraft::turn(Point3D& direction)
{
    (speed += direction.cap_length(type.max_accel)).cap_length(max_speed());
}

unsigned int Aircraft::get_speed_octant() const
{
    const float speed_len = speed.length();
    if (speed_len <= 0) return 0;
    const Point3D norm_speed{speed * (1.0f / speed_len)};
    const float angle =
            (norm_speed.y() > 0) ? 2.0f * 3.141592f - std::acos(norm_speed.x()) : std::acos(norm_speed.x());
    // partition into NUM_AIRCRAFT_TILES equal pieces
    return (static_cast<int>(std::round((angle * NUM_AIRCRAFT_TILES) / (2.0f * 3.141592f))) + 1) %
           NUM_AIRCRAFT_TILES;
}

// when we arrive at a terminal, signal the tower
void Aircraft::arrive_at_terminal()
{
    assert(is_at_terminal == false);
    control.arrived_at_terminal(*this);  // we arrived at a terminal, so start servicing
    is_at_terminal = true;
}

// deploy and retract landing gear depending on next waypoints
bool Aircraft::operate_landing_gear()
{
    if (waypoints.size() <= 1u) return false;

    const auto it            = waypoints.begin();
    const bool ground_before = it->is_on_ground();
    const bool ground_after  = std::next(it)->is_on_ground();
    // deploy/retract landing gear when landing/lifting-off
    if (ground_before && !ground_after)
    {
        if (!SILENT_TERMINAL) std::cout << flight_number << " lift off" << std::endl;
        return true;
    }
    if (!ground_before && ground_after)
    {
        if (!SILENT_TERMINAL) std::cout << flight_number << " is now landing..." << std::endl;
        landing_gear_deployed = true;
    }
    else if (!ground_before)
    {
        landing_gear_deployed = false;
    }
    return false;
}

// Move the aircraft and return True if the aircraft need to be destroyed
// A destruction appear in 2 cases: No fuel left and lifting off.
bool Aircraft::move(double dt)
{
    assert(dt > 0);
    if (fuel <= 0)                                                          // Crash if no fuel
        throw AircraftCrash {flight_number, pos, speed, out_of_fuel};
    if (!is_on_ground()) {                                                  // Decrease fuel level
        fuel -= dt * type.fuel_consumption * (speed.length() / max_speed());
    }
    if (waypoints.empty()) {                                                // Update path when empty
        for (const auto& wp: control.get_instructions(*this))
        {
            const bool front = false;
            add_waypoint<front>(wp);
        }
    }
    if (is_circling()) {                                                    // If making circles
        auto wp = control.reserve_terminal(*this);                          // Try to update the path
        if (!wp.empty()) {
            waypoints = wp;                                                 // If path to terminal update the path
        }
    }
    if (is_at_terminal) return false;                                       // If serviced don't move
    turn_to_waypoint();                                                     // Rotate
    pos += speed * static_cast<float>(dt);                                  // Move

    if (!waypoints.empty() && distance_to(waypoints.front()) < DISTANCE_THRESHOLD)  // If close enough, remove the waypoint
    {
        if (waypoints.front().is_at_terminal()) arrive_at_terminal();               // If at terminal -> service
        else if (operate_landing_gear()) return true;                               // If not at terminal and lifting off -> destroy
        waypoints.pop_front();                                                      // Remove waypoint
    }

    if (is_on_ground() && !landing_gear_deployed)                                   // Invalid state caused by speed
        throw AircraftCrash {flight_number, pos, speed, bad_landing};
    if (!is_on_ground() && speed.length() < SPEED_THRESHOLD)                        // If flying to slow -> sink
        pos.z() -= SINK_FACTOR * (SPEED_THRESHOLD - speed.length());

    GL::Displayable::z = pos.x() + pos.y();                                         // Update z
    return false;
}

void Aircraft::display() const
{
    type.texture.draw(project_2D(pos), { PLANE_TEXTURE_DIM, PLANE_TEXTURE_DIM }, get_speed_octant());
}
bool Aircraft::is_circling() const
{
    return !waypoints.empty() && !waypoints.back().is_on_ground() && !landing_gear_deployed;
}
bool Aircraft::has_terminal() const
{
    return !waypoints.empty() && waypoints.back().is_at_terminal();
}

bool Aircraft::operator<(const Aircraft &rhs) const {
    if (has_terminal() != rhs.has_terminal()) return has_terminal();
    return fuel < rhs.fuel;
}

void Aircraft::refill(unsigned int& fuel_stock) {
    const auto needed = get_missing_fuel();
    if (fuel_stock == 0) return;
    if (fuel_stock < needed) {
//        std::cout << "Refuelling " << fuel_stock << ". Aircraft not full." << std::endl;
        fuel += fuel_stock;
        fuel_stock = 0;
    } else {
//        std::cout << "Refuelling " << needed << ". Aircraft full." << std::endl;
        fuel += needed;
        fuel_stock -= needed;
    }
}
// -------------------------
//  Automatically generated
// -------------------------
bool Aircraft::operator>(const Aircraft &rhs) const {
    return rhs < *this;
}
bool Aircraft::operator<=(const Aircraft &rhs) const {
    return !(rhs < *this);
}
bool Aircraft::operator>=(const Aircraft &rhs) const {
    return !(*this < rhs);
}