#include "int.h"

int main() {
    SAVE_START_TIME();
    int32_t v = 0;
    for (int32_t i = 0; i <= 10000; i++) { v += i; }
    printf("%d ", v);
    PRINT_TIME_FROM_START();
}