#include "Application.hpp"

#include <exception>
#include <iostream>

int main() {
    try {
        runegenerator::Application app;
        return app.run();
    } catch (const std::exception& error) {
        std::cerr << "RuneGenerator: " << error.what() << '\n';
        return 1;
    }
}
