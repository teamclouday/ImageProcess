#define STB_IMAGE_IMPLEMENTATION
#include "App.hpp"

#include <iostream>
#include <stdexcept>
#include <cstdlib>

int main()
{
    try
    {
        Application app;
        app.run();
    }
    catch(const std::runtime_error& err)
    {
        std::cerr << err.what() << std::endl;
        exit(-1);
    }
    return 0;
}
