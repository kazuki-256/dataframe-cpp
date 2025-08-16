#define DF_DEBUG_LEVEL 6
#include "types/data_frame.hpp"

int main(int argc, char** argv) {
    df_column<df_string> numbers = {"hello world", "yoshihara kazuki"};

    df_data_frame df = {
        {"id", df_column<int>{1, 2, 3, 4, 5}},
        {"name", df_column<df_string>{"1", "2", "3", "4", "5"}}
    };

    // df_data_frame still not completed
    

    numbers.print();
    return 0;
    
}