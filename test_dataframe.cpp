#define DF_DEBUG_LEVEL 7
#include "classes/dataframe.cpp"
#include "vector.cpp"

int main(int argc, char** argv) {
    // 初期化：2列のデータフレーム
    df_dataframe_t df = {
        {"A", df_range_int32(0, 30)},
        {"B", df_range_int32(0, 30)},
        {"C", df_range_int32(0, 30)},
        {"D", df_range_int32(0, 30)},
        {"E", df_range_int32(0, 30)},
    };

    std::cout << df.range_rows();
    return 0;
}