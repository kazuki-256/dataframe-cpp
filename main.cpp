#include "types/raw.hpp"


int main(int argc, char** argv) {
    df_date_t raw = "2005-12-16";
    raw -= "20 years";

    printf("value: %s\n", raw.c_str());
    return 0;
}