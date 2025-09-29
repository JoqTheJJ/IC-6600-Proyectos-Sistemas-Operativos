#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <inttypes.h>
#include <limits.h>

typedef struct {
    // buffer crudo del archivo (útil si luego quieres re-leer algo)
    uint8_t *file_buf;
    size_t   file_len;

    // header v
    uint32_t dict_end_bit;   // bit absoluto (límite derecho EXCLUSIVO del diccionario)

    // —— Sección 1: Diccionario ——
    uint32_t *dict_chars;    // arreglo de code points U+XXXX (wchar lógico)
    size_t    dict_count;    // cantidad de entradas

    // —— Sección 2: Libros ——
    // Array de bits lineal (0/1) con TODOS los bits desde dict_end_bit hasta EOF.
    // books_bits[k] es el bit absoluto (dict_end_bit + k).
    uint8_t  *books_bits;        // cada elemento vale 0 o 1
    size_t    books_bits_count;  // cantidad de bits en 'books_bits'
} KirbyFile;

// ---- helpers big-endian ----
static inline uint32_t rd_u32_be(const uint8_t *p) {
    return ((uint32_t)p[0] << 24) | ((uint32_t)p[1] << 16) | ((uint32_t)p[2] << 8) | (uint32_t)p[3];
}

// Lee un bit global (MSB-first en cada byte) del buffer
static inline uint8_t get_bit_msb(const uint8_t *buf, size_t bit_index) {
    size_t byte_i = bit_index >> 3;          // /8
    int bit_in_byte = 7 - (int)(bit_index & 7); // MSB-first
    return (uint8_t)((buf[byte_i] >> bit_in_byte) & 1u);
}

// ---- IO archivo ----
static int load_file(const char *path, uint8_t **out_buf, size_t *out_len) {
    *out_buf = NULL; *out_len = 0;
    FILE *f = fopen(path, "rb");
    if (!f) { fprintf(stderr, "No pude abrir %s: %s\n", path, strerror(errno)); return 0; }
    if (fseek(f, 0, SEEK_END) != 0) { fclose(f); return 0; }
    long sz = ftell(f);
    if (sz < 0) { fclose(f); return 0; }
    if (fseek(f, 0, SEEK_SET) != 0) { fclose(f); return 0; }

    uint8_t *buf = (uint8_t*)malloc((size_t)sz);
    if (!buf) { fclose(f); return 0; }

    size_t n = fread(buf, 1, (size_t)sz, f);
    fclose(f);
    if (n != (size_t)sz) { free(buf); return 0; }

    *out_buf = buf; *out_len = n;
    return 1;
}

static void free_kirby(KirbyFile *kf) {
    if (!kf) return;
    free(kf->dict_chars);
    free(kf->books_bits);
    free(kf->file_buf);
    memset(kf, 0, sizeof(*kf));
}


static int parse_kirby_be(const char *path, KirbyFile *out) {
    memset(out, 0, sizeof(*out));
    if (!load_file(path, &out->file_buf, &out->file_len)) return 0;

    const uint8_t *buf = out->file_buf;
    size_t len = out->file_len;
    uint64_t file_bits = (uint64_t)len * 8ull;

    if (len < 4) {
        fprintf(stderr, "Archivo demasiado corto (%zu bytes)\n", len);
        return 0;
    }

    // Sección 0: finDiccionario (u32 BE, bit absoluto EXCLUSIVO)
    out->dict_end_bit = rd_u32_be(buf + 0);

    if (out->dict_end_bit < 32) {
        fprintf(stderr, "finDiccionario=%" PRIu32 " < 32 bits (el header ocupa 32 bits)\n", out->dict_end_bit);
        return 0;
    }
    if ((uint64_t)out->dict_end_bit > file_bits) {
        fprintf(stderr, "finDiccionario=%" PRIu32 " rebasa el tamaño del archivo (%zu bytes = %" PRIu64 " bits)\n",
                out->dict_end_bit, len, file_bits);
        return 0;
    }

    // —— Diccionario: bits [32 .. dict_end_bit) ——  cada nodo = 32 bits (u32 BE)
    uint32_t dict_bits = out->dict_end_bit - 32u;
    out->dict_count = dict_bits / 32u;
    uint32_t dict_resto = dict_bits % 32u; // si no es 0, habrá basura que ignoramos
    if (dict_resto != 0u) {
        // No abortamos: simplemente ignoramos los bits sobrantes.
        fprintf(stderr, "Aviso: el diccionario no está alineado a 32 bits (%" PRIu32 " bits extra ignorados)\n", dict_resto);
    }

    if (out->dict_count > 0) {
        // Como el diccionario empieza en el byte 4, está byte-alineado para leer u32 BE.
        size_t dict_byte_start = 4;
        size_t need_dict_bytes = (size_t)out->dict_count * 4u;
        if (dict_byte_start + need_dict_bytes > len) {
            fprintf(stderr, "Bytes insuficientes para %" PRIu64 " entradas del diccionario\n",
                    (uint64_t)out->dict_count);
            return 0;
        }
        out->dict_chars = (uint32_t*)calloc(out->dict_count, sizeof(uint32_t));
        if (!out->dict_chars) { fprintf(stderr, "Sin memoria para diccionario\n"); return 0; }

        for (size_t i = 0; i < out->dict_count; ++i) {
            size_t off = dict_byte_start + i*4u;
            out->dict_chars[i] = rd_u32_be(buf + off);
        }
    }

    // —— Libros: bits [dict_end_bit .. EOF) ——  volcamos a un array de 0/1
    uint64_t books_bits_u64 = file_bits - (uint64_t)out->dict_end_bit;
    if (books_bits_u64 > (uint64_t)SIZE_MAX) {
        fprintf(stderr, "Demasiados bits en Libros para esta plataforma (%" PRIu64 ")\n", books_bits_u64);
        return 0;
    }
    out->books_bits_count = (size_t)books_bits_u64;
    if (out->books_bits_count > 0) {
        out->books_bits = (uint8_t*)malloc(out->books_bits_count);
        if (!out->books_bits) { fprintf(stderr, "Sin memoria para array de bits de Libros\n"); return 0; }

        // Cargamos cada bit en orden, MSB-first por byte
        // k recorre 0..books_bits_count-1  ⇒ bit absoluto = dict_end_bit + k
        size_t start_bit = (size_t)out->dict_end_bit;
        for (size_t k = 0; k < out->books_bits_count; ++k) {
            out->books_bits[k] = get_bit_msb(buf, start_bit + k);
        }
    }

    return 1;
}

// --- (opcional) demo mínima: muestra tamaños y primeros elementos ---
static void dump_demo(const KirbyFile *kf) {
    printf("Archivo: %zu bytes\n", kf->file_len);
    printf("finDiccionario (bit, exclusivo) = %" PRIu32 "\n", kf->dict_end_bit);
    printf("Diccionario: %zu entradas\n", kf->dict_count);
    if (kf->dict_count) {
        size_t show = kf->dict_count < 10 ? kf->dict_count : 10;
        for (size_t i = 0; i < show; ++i) {
            printf("  dict[%zu] = U+%04" PRIX32 "\n", i, kf->dict_chars[i]);
        }
        if (kf->dict_count > show) printf("  ... (%zu más)\n", kf->dict_count - show);
    }
    printf("Libros: %zu bits\n", kf->books_bits_count);
    if (kf->books_bits_count) {
        size_t showb = kf->books_bits_count < 64 ? kf->books_bits_count : 64;
        printf("  primeros %zu bits: ", showb);
        for (size_t i = 0; i < showb; ++i) putchar(kf->books_bits[i] ? '1' : '0');
        if (kf->books_bits_count > showb) printf("...");
        putchar('\n');
    }
}

int main() {
    const char *path = "comprimido.kirby";

    KirbyFile kf;
    if (!parse_kirby_be(path, &kf)) return 2;

    //dump_demo(&kf);


    free_kirby(&kf);
    return 0;
}
