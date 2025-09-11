#include "app.h"
#include <stdio.h>

size_t app_sum(const int *arr, size_t n) {
    size_t s = 0;
    for (size_t i = 0; i < n; ++i) s += (size_t)arr[i];
    return s;
}

void app_run(void) {
    int data[] = {1, 2, 3, 4, 5};
    size_t n = sizeof(data) / sizeof(data[0]);
    size_t s = app_sum(data, n);
    printf("[proj1] example run ok. sum=%zu\n", s);
}
