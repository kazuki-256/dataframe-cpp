#define DF_DEBUG_LEVEL 0
#include "types/object.hpp"


#include <iostream>

int main(int argc, char** argv) {
    df_object_t object = 100.0;

    object = "hello world";
    std::cout << object << "\n";
    return 0;
}