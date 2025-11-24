#include <exception>
#include <iostream>

#include "Game.h"

int main()
{
    try
    {
        gol::Game game {};
        game.Begin();
    }
    catch (const std::exception& e)
    {
        std::cerr << "Fatal Error: " << e.what() << std::endl;
        throw;
    }
}