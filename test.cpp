#include <string>

int main(int argc, char** argv) {
    std::string text = "123";

    printf("debug: %s\n", !text.empty() ? "true" : "false");
}