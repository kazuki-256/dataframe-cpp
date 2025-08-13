#include <stdio.h>
#include <time.h>


int main(int argc, char** argv) {
    long value;
    sscanf("+0800", "%05lu", &value);
    printf("%d\n", value);
    return 0;
}