#include "util.h"

void print(char* s) {
    write(STDOUT_FILENO, s, sizeof(s));
}
