#include "app/Application.h"
#include <exception>
#include <iostream>

int main()
{
    try
    {
        WorkBalance::App::Application app;
        app.run();
        return 0;
    }
    catch (const std::exception &e)
    {
        std::cerr << "Error: " << e.what() << '\n';
        return 1;
    }
}
