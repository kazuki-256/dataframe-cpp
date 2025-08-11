#include "DataFrame.hpp"

int main(int argc, char** argv) {
    DfDataFrame df = {
        {"id", DfColumn(DF_OBJTYPE_STRING)}
    };

    DfObject obj = 1;
    int number = obj;
}