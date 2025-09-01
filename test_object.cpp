#include "types/object.hpp"
#include <iostream>

int main(int argc, char** argv) {
    df_object_t objectA = 100;
    df_object_t objectB = "hello world";
    df_object_t objectC = 3.1416;

    std::cout << objectA << "\n";
    std::cout << objectB << "\n";
    std::cout << objectC << "\n";

    return 0;
}