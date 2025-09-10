#include "classes/dataframe.cpp"
#include "vector.cpp"


int main(int argc, char** argv) {
    df_column_t column = df_range_int32(1, 5000000, 1);

    std::cout << column << "\n";
    return 0;
}