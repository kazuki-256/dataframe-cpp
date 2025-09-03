#include "types/column.hpp"
#include <iostream>

int main(int argc, char** argv) {
    df_column_t column = df_column_date_t::range("2005-1-1", "2010-1-1", df_interval_t("1 years"));

    df_column_t merged_column = column.merge_to(column);

    std::cout << column << "\n";
    std::cout << merged_column << "\n";
    return 0;
}
