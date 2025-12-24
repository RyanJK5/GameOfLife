#include <exception>
#include <iostream>

#include "ConfigLoader.h"
#include "Game.h"

int main()
{
    try
    {
        auto config = gol::StyleLoader::LoadYAML<ImVec4>(std::filesystem::path("config") / "style.yaml");
        if (!config)
			throw std::exception(config.error().Description.c_str());
        gol::Game game { *config };
        game.Begin();
    }
    catch (const std::exception& e)
    {
        std::cerr << "Fatal Error: " << e.what() << std::endl;
        throw;
    }
}