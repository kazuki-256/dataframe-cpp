#define DF_DEBUG_LEVEL 3
#include "classes/column.cpp"
#include "vector.cpp"
#include <simpdata.h>

int main(int argc, char** argv) {
    df_column_t days = df_range_datetime("2024", "2025", "1 week");
    df_column_t names = {"kazuki", "hello world", "minecraft", "dataframe-cpp"};

    std::cout << days << "\n";
    std::cout << names << "\n";
    return 0;
}
