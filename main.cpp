#include "Column.hpp"

int main(int argc, char** argv) {
    DfColumn numbers = {1, 2, 3, 4, 5};
    DfColumn doubleNumbers = numbers * 3.5;

    doubleNumbers.print();
    printf("end\n");
}