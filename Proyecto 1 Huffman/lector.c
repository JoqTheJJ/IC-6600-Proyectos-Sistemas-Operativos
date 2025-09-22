// contar_freq_utf8.c
#include <locale.h>
#include <wchar.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

#define UNICODE_SCALAR_MAX 0x110000u  // U+0000 .. U+10FFFF (1,114,112 entradas)

int contar_archivo(const char *ruta, uint64_t *freq) {
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

int main(int argc, char **argv) {
    if (!setlocale(LC_ALL, "")) {
        fprintf(stderr, "Advertencia: no se pudo establecer locale; asegúrate de usar UTF-8.\n");
    }

    if (argc < 2) {
        fprintf(stderr, "Uso: %s archivo1.txt [archivo2.txt ...]\n", argv[0]);
        return 1;
    }

    // Reserva en heap para no reventar la pila
    uint64_t *freq = calloc(UNICODE_SCALAR_MAX, sizeof(uint64_t));

    if (!freq) {
        fprintf(stderr, "Sin memoria para tabla de frecuencias.\n");
        return 1;
    }

    int status = 0;
    for (int i = 1; i < argc; i++) {
        if (contar_archivo(argv[i], freq) != 0) status = 1;
    }


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

    free(freq);
    return status;
}
