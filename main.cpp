#define DF_DEBUG_LEVEL 0
#include "types/object.hpp"


#include <iostream>

int main(int argc, char** argv) {
<<<<<<< HEAD
    df_object_t object = 100.0;

    object = "hello world";
    std::cout << object << "\n";
=======
    df_date_t raw = "2005-12-16";
    raw -= "20 years";

    printf("value: %s\n", raw.c_str());
>>>>>>> e91bc2cb611defaf90ba1af8ca7646f80da722a7
    return 0;
}