// contar_freq_utf8.c
#include <locale.h>
#include <wchar.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <limits.h>
#include <glob.h>
#include <linux/limits.h>

#define UNICODE_SCALAR_MAX 0x110000u  // U+0000 .. U+10FFFF (1,114,112 entradas)

char rutas[100][PATH_MAX];

int getFreqAux(const char *ruta, uint64_t *freq) {
    FILE *f = fopen(ruta, "r");
    if (!f) {
        fprintf(stderr, "No pude abrir %s: %s\n", ruta, strerror(errno));
        return -1;
    }

    wint_t wc;
    while ((wc = fgetwc(f)) != WEOF) {
        // Filtra valores fuera de Unicode válido (por robustez)
        if ((unsigned)wc < UNICODE_SCALAR_MAX &&
            !((wc >= 0xD800 && wc <= 0xDFFF))) { // evita surrogates
            freq[(unsigned)wc]++;                // cuenta el punto de código
        }
    }

    if (ferror(f)) {
        fprintf(stderr, "Error de lectura en %s\n", ruta);
        fclose(f);
        return -1;
    }

    fclose(f);
    return 0;
}

uint64_t* getFreq(char* book) {
    uint64_t *freq = calloc(UNICODE_SCALAR_MAX, sizeof(uint64_t));

    if (!freq) {
        fprintf(stderr, "Sin memoria para tabla de frecuencias.\n");
        return NULL;
    }

    if (getFreqAux(book, freq) == -1) return NULL;

    // Imprime el array y el total (Solo para debug)
    /*
    unsigned int contador = 0;

    for (unsigned cp = 0; cp < UNICODE_SCALAR_MAX; ++cp) {
        if (cp >= 0xD800 && cp <= 0xDFFF) continue;   // omite surrogates
        if (freq[cp] == 0) continue;                  // imprime solo los presentes

        contador++;
        // Imprime: "Letra: x | Freq: N"
        if (cp == L'\n')
            wprintf(L"Letra: \\n | Freq: %llu\n", (unsigned long long)freq[cp]);
        else
            wprintf(L"Letra: %lc | Freq: %llu\n", (wint_t)cp, (unsigned long long)freq[cp]);

    }

    wprintf(L"\nHola, hay %d cantidad de digitos distintos\n", contador);
    */

    return freq;
}

uint64_t* sumFreq(uint64_t* a, uint64_t* b) {
    if (!a || !b) return NULL;
    for (unsigned c = 0; c < UNICODE_SCALAR_MAX; ++c) {
        a[c] += b[c];
    }
    free(b);
    return a;
}

void setBooks(char* pattern) {
    glob_t g = {0};

    int rc = glob(pattern, 0, NULL, &g);
    // Bloque de errores
    if (rc != 0) {
        if (rc == GLOB_NOMATCH) {
            fprintf(stderr, "No se encontraron archivos que coincidan con %s\n", pattern);
        } else if (rc == GLOB_NOSPACE) {
            fprintf(stderr, "Memoria insuficiente durante glob()\n");
        } else {
            fprintf(stderr, "Error de glob() (código %d)\n", rc);
        }
        return;
    }

    if (g.gl_pathc < 100) {
        fprintf(stderr, "Advertencia: se esperaban 100 .txt, se encontraron %zu\n", g.gl_pathc);
    }

    size_t n = g.gl_pathc < 100 ? g.gl_pathc : 100;

    for (size_t i = 0; i < n; ++i) {
        // Copia segura con terminación
        strncpy(rutas[i], g.gl_pathv[i], PATH_MAX - 1);
        rutas[i][PATH_MAX - 1] = '\0';
    }

    // Print dir (Solo para debug)
    /*
    for (size_t i = 0; i < n; ++i) {
        puts(rutas[i]);         // p.ej., "Libros/DonQuijote.txt"
    }
    */

    globfree(&g);
    return;
}



uint64_t* getAllFreqSerial() {
    uint64_t *totalFreq = calloc(UNICODE_SCALAR_MAX, sizeof(uint64_t)); //Padre
    if (!totalFreq) return NULL;

    for (int book = 0; book < 100; ++book) {
        uint64_t *bookFreq = getFreq(rutas[book]);
        sumFreq(totalFreq, bookFreq);
    }

    return totalFreq;
}




uint64_t* frequenceSerial() {

    setBooks("Libros/*.txt");

    uint64_t* freq = getAllFreqSerial();

    if(!freq) return NULL;

    return freq;
}

/*
int main() {
    if (!setlocale(LC_ALL, "")) {
        fprintf(stderr, "Advertencia: no se pudo establecer locale; asegúrate de usar UTF-8.\n");
    }

    setBooks("Libros/*.txt");

    uint64_t* freq = getAllFreqSerial();

    if(!freq) return -1;

    return 0;
}*/
