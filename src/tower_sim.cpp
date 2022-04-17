#include "tower_sim.hpp"

#include "GL/opengl_interface.hpp"
#include "AircraftManager.hpp"
#include "airport.hpp"
#include "config.hpp"
#include "img/image.hpp"
#include "img/media_path.hpp"
#include "AircraftFactory.h"

#include <cassert>
#include <cstdlib>
#include <ctime>

using namespace std::string_literals;

TowerSimulation::TowerSimulation(int argc, char **argv) : help{(argc > 1) && (std::string{argv[1]} == "--help"s || std::string{argv[1]} == "-h"s)}
{
    MediaPath::initialize(argv[0]);
    data_path = help && argc > 2 ? argv[2] : argc > 1 ? argv[1]
                                                      : "";
    std::srand(static_cast<unsigned int>(std::time(nullptr)));
    GL::init_gl(argc, argv, "Airport Tower Simulation");
    aircraft_manager = std::make_unique<AircraftManager>();

    create_keystrokes();
}
void TowerSimulation::create_random_aircraft()
{
    assert(airport); // make sure the airport is initialized before creating aircraft
    aircraft_manager->add_aircraft(aircraft_factory->create_aircraft(airport->get_tower()));
}

void TowerSimulation::display_airline(unsigned number)
{
    assert(number <= 7);
    const std::string &airline = airlines[number];
    const unsigned count = aircraft_manager->count_aircraft_on_airline(airline);
    std::cout << count << " aircraft for the line : " << airline << std::endl;
}

void TowerSimulation::create_keystrokes()
{
    GL::keystrokes.emplace('x', []()
                           { GL::exit_loop(); });
    GL::keystrokes.emplace('q', []()
                           { GL::exit_loop(); });
    GL::keystrokes.emplace('c', [this]()
                           { create_random_aircraft(); });
    GL::keystrokes.emplace('+', []()
                           { GL::change_zoom(0.95f); });
    GL::keystrokes.emplace('-', []()
                           { GL::change_zoom(1.05f); });
    GL::keystrokes.emplace('f', []()
                           { GL::toggle_fullscreen(); });
    GL::keystrokes.emplace('i', []()
                           { GL::change_framerate(+1); });
    GL::keystrokes.emplace('d', []()
                           { GL::change_framerate(-1); });
    GL::keystrokes.emplace('p', []()
                           { GL::is_paused = !GL::is_paused; });

    GL::keystrokes.emplace('o', []()
                           { GL::change_framerate_modifier(1.01); });
    GL::keystrokes.emplace('l', []()
                           { GL::change_framerate_modifier(0.99); });
    GL::keystrokes.emplace('m', [this]()
                           { aircraft_manager->display_crash_number(); });
    for (auto i = 0; i < 8; i++)
    {
        GL::keystrokes.emplace('0' + i, [this, i]()
                               { display_airline(i); });
    }
}

void TowerSimulation::display_help()
{
    std::cout << "This is an airport tower simulator" << std::endl
              << "the following keystrokes have meaning:" << std::endl;

    for (const auto &[key, action] : GL::keystrokes)
    {
        std::cout << key << ' ';
    }
    std::cout << std::endl;
}

void TowerSimulation::init_airport()
{
    airport = std::make_unique<Airport>(one_lane_airport, Point3D{0.f, 0.f, 0.f},
                                        new img::Image{one_lane_airport_sprite_path.get_full_path()}, *aircraft_manager);
    GL::move_queue.emplace(airport.get());
}

void TowerSimulation::launch()
{
    assert(airport == nullptr && aircraft_factory == nullptr);
    if (help)
    {
        display_help();
        return;
    }
    init_airport();
    aircraft_factory = data_path.empty() ? std::make_unique<AircraftFactory>() : AircraftFactory::LoadTypes(MediaPath{data_path});

    GL::loop();
}
