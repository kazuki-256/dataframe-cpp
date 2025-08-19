#include "types/raw.hpp"


int main(int argc, char** argv) {
    try {
        df_raw_t<DF_DATETIME> raw_text = "0105-12-16";
        printf("type: %s\n", df_get_type_string(DF_DATETIME));
        printf("value: %s\n", raw_text.c_str());
        printf("value: %ld\n", df_date_t("0000-1-1"));
        printf("sizeof: %ld\n", sizeof(struct tm));
    }
    catch (df_exception_t& e) {
        fprintf(stderr, "info: %s\n", e.what());
    }
    return 0;
}