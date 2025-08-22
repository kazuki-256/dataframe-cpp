#define DF_DEBUG_LEVEL 0
#include "types/object.hpp"


#include <iostream>

int main(int argc, char** argv) {
    df_object_t object = true;
    std::cout << object << "\n";

    object = "3.14";
    printf("%d\n", (uint8_t)object);
    return 0;
}