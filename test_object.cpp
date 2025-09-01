#include "types/object.hpp"
#include <iostream>

int main(int argc, char** argv) {
    df_object_t objectA = 100;              // int
    df_object_t objectB = "hello world";    // text
    df_object_t objectC = 3.1416;           // float

    std::cout << objectA << "\n";
    std::cout << objectB << "\n";
    std::cout << objectC << "\n";

    // object type would not chnage after created, objectA would be 3
    objectA = 3.1416;

    // object copy
    objectB = objectC;


    std::cout << objectA << "\n";
    std::cout << objectB << "\n";
    std::cout << objectC << "\n";

    return 0;
}