#define DF_DEBUG_LEVEL 4
#include "Types/DataFrame.hpp"

int main(int argc, char** argv) {
    DfDataFrame df = {
        {"id", {1, 2, 3}},
        {"name", {"1", "2", "3"}}
    };

    printf("end\n");
}