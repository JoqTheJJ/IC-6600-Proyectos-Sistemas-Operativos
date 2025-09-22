#include <locale.h>
#include <wchar.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

int limitDict = 0;
int limitIndex = 0;

struct {
    wchar_t c;
    unsigned int len;
    
} Dictionary;