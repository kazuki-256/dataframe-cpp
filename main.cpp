#include "classes/column_new.cpp"


int main(int argc, char** argv) {
    df_column_t column = {1, 2, 3, 4, 5};

    std::cout << column << "\n";
    return 0;
}