#include "types/column.hpp"
#include <time.h>
#include <iostream>

int main(int argc, char** argv) {
    struct timespec start, end;
	clock_gettime(CLOCK_MONOTONIC, &start);
    df_column_t column = df_column_t::make_date_range("2005-1-1", "2006-1-1", df_interval_t("30 seconds"));
    clock_gettime(CLOCK_MONOTONIC, &end);

    printf("usetime: %lf s\n", end.tv_sec - start.tv_sec + ((end.tv_nsec - start.tv_nsec) % 1000000 / 1.0e6));
    return 0;
}
