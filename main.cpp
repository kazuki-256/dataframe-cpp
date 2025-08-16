#define DF_DEBUG_LEVEL 6
#include "types/data_frame.hpp"

int main(int argc, char** argv) {
    const df_data_frame df = {
        {"id", df_column<int>{1, 2, 3, 4, 5}},
        {"name", df_column<df_string>{"a", "b", "c", "d", "e"}}
    };

    // df_data_frame still not completed
    

    std::cout << df["name"][2].c_str(DF_TYPE_STRING) << "\n";
    return 0;
    
}