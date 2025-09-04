#define DF_DEBUG_LEVEL 3
#include "types/column.hpp"

int main(int argc, char** argv) {
    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC, &start);
    df_column_t column = df_column_t::range_date("100", "2025", "1 month");
    clock_gettime(CLOCK_MONOTONIC, &end);

    printf("usetime: %lu ns\n", (end.tv_sec - start.tv_sec) * 1000000000 + (end.tv_nsec - start.tv_nsec));


    std::cout << "first: " << column[0] << " " << column[1] << " " << column[2] << "\n";
    std::cout << "last: " << column[-3] << " " << column[-2] << " " << column[-1] << "\n";

    return 0;
}
