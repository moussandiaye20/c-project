#pragma once

#include "GL/displayable.hpp"
#include "aircraft_types.hpp"
#include "config.hpp"
#include "geometry.hpp"
#include "tower.hpp"
#include "waypoint.hpp"

#include <string>
#include <string_view>
#include <cmath>

class Aircraft : public GL::Displayable
{
friend std::ostream& operator<<(std::ostream& stream, const Aircraft& aircraft) {
    return stream << "Aircraft: " << aircraft.flight_number << " | " << aircraft.has_terminal()
    << " | " << aircraft.fuel << " | " << aircraft.type.min_fuel() << " | " << aircraft.type.max_fuel;
}
private:
    const AircraftType& type;               // The life time of this field is less than the container in AircraftFactory so no dangling ref here
    const std::string flight_number;        // Aircraft identifier
    Point3D pos, speed;                     // note: the speed should always be normalized to length 'speed'
    WaypointQueue waypoints = {};           // Path of the aircraft
    Tower& control;                         // Reference to the Tower
    bool landing_gear_deployed = false;     // is the landing gear deployed?
    bool is_at_terminal        = false;     // is the aircraft at a terminal
    double fuel                = 0;         // fuel level

    // turn the aircraft to arrive at the next waypoint
    // try to facilitate reaching the waypoint after the next by facing the
    // right way to this end, we try to face the point Z on the line spanned by
    // the next two waypoints such that Z's distance to the next waypoint is
    // half our distance so: |w1 - pos| = d and [w1 - w2].normalize() = W and Z
    // = w1 + W*d/2
    void turn_to_waypoint();
    void turn(Point3D& direction);

    // select the correct tile in the plane texture (series of 8 sprites facing
    // [North, NW, W, SW, S, SE, E, NE])
    [[nodiscard]] unsigned int get_speed_octant() const;
    // when we arrive at a terminal, signal the tower
    void arrive_at_terminal();
    // deploy and retract landing gear depending on next waypoints
    bool operate_landing_gear();
    [[nodiscard]] bool is_on_ground() const { return pos.z() < DISTANCE_THRESHOLD; }
    [[nodiscard]] float max_speed() const { return is_on_ground() ? type.max_ground_speed : type.max_air_speed; }
    double static compute_initial_fuel(const AircraftType& type) {
        const double f = std::rand() % (type.max_fuel - static_cast<int>(type.min_fuel()));
        return type.min_fuel() + f;
    }
    template<bool front = false>
    void add_waypoint(const Waypoint& wp) {
        if constexpr (front) waypoints.push_front(wp);
        else waypoints.push_back(wp);
    }

public:
    Aircraft(const Aircraft&) = delete;
    Aircraft& operator=(const Aircraft&) = delete;
    ~Aircraft() override;
    Aircraft(const AircraftType& type_, const std::string_view& flight_number_, const Point3D& pos_,
             const Point3D& speed_, Tower& control_) :
        GL::Displayable { pos_.x() + pos_.y() },
        type { type_ },
        flight_number { flight_number_ },
        pos { pos_ },
        speed { speed_ },
        control { control_ },
        fuel {compute_initial_fuel(type_)}
    {
        speed.cap_length(max_speed());
    }

    [[nodiscard]] const std::string& get_flight_num() const { return flight_number; }
    [[nodiscard]] float distance_to(const Point3D& p) const { return pos.distance_to(p); }
    [[nodiscard]] bool is_low_on_fuel() const { return fuel < type.min_fuel(); }
    [[nodiscard]] unsigned get_missing_fuel() const { return type.max_fuel - (unsigned)std::ceil(fuel); }

    [[nodiscard]] bool is_circling() const;
    [[nodiscard]] bool has_terminal() const;
    void refill(unsigned&);

    bool operator<(const Aircraft &rhs) const;
    bool operator>(const Aircraft &rhs) const;
    bool operator<=(const Aircraft &rhs) const;
    bool operator>=(const Aircraft &rhs) const;

    void display() const override;
    bool move(double);

    friend class Tower;
};
