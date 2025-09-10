#define DF_DEBUG_LEVEL 10
#include "classes/dataframe.cpp"
#include "vector.cpp"

int main(int argc, char** argv) {
    // 初期化：2列のデータフレーム
    df_dataframe_t df = {
        {"A", df_range_int32(0, 50000)},
        {"B", df_range_int32(0, 50000)},
        {"C", df_range_int32(0, 50000)},
        {"D", df_range_int32(0, 50000)},
        {"E", df_range_int32(0, 50000)},
    };

    timespec start, end;
    clock_gettime(CLOCK_MONOTONIC, &start);
    long total = 0;
    for (df_row_t& row : df.range_rows(2000, 40000)) {
        total = (long)row["B"] + (long)row["C"];
    }
    clock_gettime(CLOCK_MONOTONIC, &end);

    std::cout << "total: " << total << "\n";
    std::cout << "usetime: " << (end.tv_sec - start.tv_sec) * 1000000000 + end.tv_nsec - start.tv_nsec << " ns\n";
    return 0;
}