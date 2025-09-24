// kirbyparse_be.c — Lector .kirby (BIG-ENDIAN fijo) — Diccionario = u32 por nodo
// Layout (offset base 0):
// [0..3]  u32 fin_dicc
// [4..7]  u32 tam_dicc (informativo)
// [8..11] u32 ini_indices
// [12..fin_dicc) diccionario: entradas contiguas de 4 bytes (u32 code point)
// [fin_dicc..ini_indices) libros: bytes crudos ('0' y '1')
// [ini_indices..ini_indices+400) 100 * u32 offsets absolutos (BE) para cada libro

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <inttypes.h>

#define KIRBY_HEADER_SIZE  12u
#define KIRBY_DICT_START   12u
#define KIRBY_BOOKS_COUNT  100u

// ---- lectura big-endian segura ----
static inline uint32_t rd_u32_be(const uint8_t *p) {
    return ((uint32_t)p[0] << 24) | ((uint32_t)p[1] << 16) | ((uint32_t)p[2] << 8) | (uint32_t)p[3];
}

// (opcional) convertir U+codepoint a UTF-8 (útil para TXT más adelante)
static int u32_to_utf8(uint32_t cp, char out[4]) {
    if (cp <= 0x7F)               { out[0]=(char)cp; return 1; }
    else if (cp <= 0x7FF)         { out[0]=0xC0|(cp>>6); out[1]=0x80|(cp&0x3F); return 2; }
    else if (cp <= 0xFFFF)        { out[0]=0xE0|(cp>>12); out[1]=0x80|((cp>>6)&0x3F); out[2]=0x80|(cp&0x3F); return 3; }
    else if (cp <= 0x10FFFF)      { out[0]=0xF0|(cp>>18); out[1]=0x80|((cp>>12)&0x3F); out[2]=0x80|((cp>>6)&0x3F); out[3]=0x80|(cp&0x3F); return 4; }
    return 0;
}

typedef struct {
    // buffer completo del archivo
    uint8_t *file_buf;
    size_t   file_len;

    // punteros del header (BE)
    uint32_t dict_end;       // fin de diccionario (offset absoluto)
    uint32_t dict_size_raw;  // informativo (puede ser bytes o #entradas, según tu writer)
    uint32_t indices_start;  // inicio de la tabla de índices (offset absoluto)

    // diccionario
    uint32_t *dict_chars;    // arreglo de code points U+XXXX
    size_t    dict_count;    // cantidad de entradas del diccionario

    // libros (crudo)
    const uint8_t *books_data;
    size_t         books_len;

    // índices
    uint32_t book_index_abs[KIRBY_BOOKS_COUNT];
    uint32_t book_index_rel[KIRBY_BOOKS_COUNT]; // relativo a books_data o UINT32_MAX si inválido
} KirbyFile;

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
    free(kf->file_buf);
    memset(kf, 0, sizeof(*kf));
}

// ---- parseo principal (BIG-ENDIAN fijo) ----
static int parse_kirby_be(const char *path, KirbyFile *out) {
    memset(out, 0, sizeof(*out));
    if (!load_file(path, &out->file_buf, &out->file_len)) return 0;
    const uint8_t *buf = out->file_buf;
    size_t len = out->file_len;

    if (len < KIRBY_HEADER_SIZE) {
        fprintf(stderr, "Archivo demasiado corto (%zu bytes)\n", len);
        return 0;
    }

    // Sección 1: header
    out->dict_end      = rd_u32_be(buf + 0);
    out->dict_size_raw = rd_u32_be(buf + 4); // informativo
    out->indices_start = rd_u32_be(buf + 8);

    // Validaciones básicas de rango
    if (out->dict_end < KIRBY_DICT_START || out->dict_end > out->indices_start || out->indices_start > len) {
        fprintf(stderr, "Header inválido (BE): fin_dicc=%" PRIu32 ", ini_indices=%" PRIu32 ", file_len=%zu\n",
                out->dict_end, out->indices_start, len);
        return 0;
    }

    // Sección 2: Diccionario (u32 contiguos)
    size_t dict_bytes = (size_t)out->dict_end - (size_t)KIRBY_DICT_START;
    if (dict_bytes % 4u != 0u) {
        fprintf(stderr, "Advertencia: diccionario %zu bytes no múltiplo de 4; truncando.\n", dict_bytes);
    }
    out->dict_count = dict_bytes / 4u;
    out->dict_chars = (uint32_t*)calloc(out->dict_count, sizeof(uint32_t));
    if (!out->dict_chars && out->dict_count) { fprintf(stderr, "Sin memoria para diccionario\n"); return 0; }

    for (size_t i = 0; i < out->dict_count; ++i) {
        size_t off = (size_t)KIRBY_DICT_START + i*4u;
        out->dict_chars[i] = rd_u32_be(buf + off);
    }

    // Sección 3: Libros (crudo)
    out->books_data = buf + out->dict_end;
    out->books_len  = (size_t)out->indices_start - (size_t)out->dict_end;

    // Sección 4: Índices (100 * u32 BE)
    size_t need_idx_bytes = (size_t)KIRBY_BOOKS_COUNT * 4u;
    if ((size_t)out->indices_start + need_idx_bytes > len) {
        fprintf(stderr, "Archivo corto para %u índices\n", KIRBY_BOOKS_COUNT);
        return 0;
    }
    for (size_t i = 0; i < KIRBY_BOOKS_COUNT; ++i) {
        uint32_t abs = rd_u32_be(buf + out->indices_start + i*4u);
        out->book_index_abs[i] = abs;
        if (abs < out->dict_end || abs > out->indices_start) {
            out->book_index_rel[i] = UINT32_MAX;     // fuera de región Libros
        } else {
            out->book_index_rel[i] = abs - out->dict_end;
        }
    }

    return 1;
}

// Devuelve el slice [ptr, ptr+len) del libro i dentro de books_data.
// Usa el siguiente índice válido o ini_indices como fin.
static int get_book_slice(const KirbyFile *kf, size_t i, const uint8_t **out_ptr, size_t *out_len) {
    if (i >= KIRBY_BOOKS_COUNT) return 0;
    uint32_t rel = kf->book_index_rel[i];
    if (rel == UINT32_MAX) return 0;

    uint32_t start_abs = kf->book_index_abs[i];
    uint32_t end_abs   = kf->indices_start;

    for (size_t j = i + 1; j < KIRBY_BOOKS_COUNT; ++j) {
        if (kf->book_index_rel[j] != UINT32_MAX) {
            uint32_t next_abs = kf->book_index_abs[j];
            if (next_abs > start_abs && next_abs <= kf->indices_start) {
                end_abs = next_abs;
                break;
            }
        }
    }
    if (end_abs < start_abs) return 0;

    *out_ptr = kf->file_buf + start_abs;
    *out_len = (size_t)end_abs - (size_t)start_abs;
    return 1;
}

// --- main mínimo de ejemplo (borralo si lo integras como librería) ---
int main() {
const char *path = "comprimido.bin";

    KirbyFile kf;
    
    if (!parse_kirby_be(path, &kf)) {
        fprintf(stderr, "Fallo al parsear el archivo.\n");
        return -1;
    }

    // Crear Arbol

    // Crear funcion que va navegando el arbol (y puede volver a la raiz)

    // Convertir los 100 libros en texto y guardarlos

    free_kirby(&kf);
    return 0;
}

