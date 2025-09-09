#define DF_DEBUG_LEVEL 3
#include "classes/dataframe.cpp"

int main(int argc, char** argv) {
    // 初期化：2列のデータフレーム
    df_dataframe_t df = {
        {"id", df_column_int32_t{1, 2, 3, 4, 5}},
        {"name", df_column_text_t{"kazuki", "B", "C", "D", "E"}}
    };

    // 列の追加（新しい列 age）
    df.add_column("age", df_column_int32_t{29, 35, 22, 41, 30});

    // 列の更新（name列の3番目を "Charlie" に変更）
    df["name"][2] << "Charlie";

    // フィルタリング：年齢が30以上の行だけ抽出（仮の filter 関数）
    // auto filtered_df = df.filter([](const df_row_t& row) {
    //     return std::stoi(row["age"]) >= 30;
    // });
    // df_dataframe_t filtered = df.select("age").where("age >= 30");

    // 表示
    std::cout << "=== Original DataFrame ===\n";
    std::cout << df << "\n";

    for (df_row_t&  row : df) {
        std::cout << row["name"] << " " << row["age"] << "\n";
    }
    std::cout << df << "\n";

    // std::cout << "=== Filtered (age >= 30) ===\n";
    // std::cout << filtered_df << "\n";

    return 0;
}