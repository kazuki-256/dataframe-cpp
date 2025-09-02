#include "types/column.hpp"
#include <iostream>

int main(int argc, char** argv) {
    df_column_date_t column = {"2005"};

    std::cout << column << "\n";
    return 0;
}