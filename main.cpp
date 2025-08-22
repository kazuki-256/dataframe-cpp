#define DF_DEBUG_LEVEL 3
#include "types/column.hpp"


#include <iostream>

int main(int argc, char** argv) {
    std::pair<std::string, df_column_t> column = {"values", {1, 2, 3, "123", 5}};

    std::cout << "Column length: " << column.second.get_length() << std::endl;
    std::cout << "Column type: " << df_type_get_string(column.second.get_data_type()) << "\n\n";

    std::cout << column << "\n";
    return 0;
}