#define DF_DEBUG_LEVEL 7
#include "classes/object.cpp"
#include <iostream>

int main(int argc, char** argv) {
    // df_object_t objectD;
    df_object_t objectA = 100;              // int
    df_object_t objectB = "hello world";    // text
    df_object_t objectC = DF_NULL;           // float

    std::cout << objectA << "\n";
    std::cout << objectB << "\n";
    std::cout << objectC << "\n";

    objectA << DF_NULL;

    // object copy
    objectB << objectC;


    std::cout << objectA << "\n";
    std::cout << objectB << "\n";
    std::cout << objectC << "\n";

    return 0;
}