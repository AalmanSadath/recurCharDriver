#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <int1> <int2> ... <intN>\n", argv[0]);
        return 1;
    }

    for (int i = 1; i < argc; i++) {
        uint64_t value = strtoull(argv[i], NULL, 10);
        
        fwrite(&value, sizeof(uint64_t), 1, stdout);
    }

    return 0;
}
