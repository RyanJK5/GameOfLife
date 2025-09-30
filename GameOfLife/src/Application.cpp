#include "GameWindow.h"
#include <exception>
#include <iostream>

#include <fstream>

int main()
{
    try
    {
        gol::GameWindow window {};
        window.Begin();
    }
    catch (std::exception e)
    {
        std::cerr << e.what() << std::endl;
        return -1;
    }
}