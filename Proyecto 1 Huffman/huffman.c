
#define _POSIX_C_SOURCE 200809L
#define _DEFAULT_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdint.h>
#include <limits.h>
#include <assert.h>
#include <time.h>

//#define _XOPEN_SOURCE 700
//#define _DEFAULT_SOURCE
#include <dirent.h>

#include <errno.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/wait.h>
#include <arpa/inet.h> 
#include <fcntl.h>

#include <wchar.h>
#include <locale.h>

#define UNICODE_SCALAR_MAX 0x110000u 
#define DICT_MAX 3000 

#ifndef PATH_MAX
#define PATH_MAX 4096
#endif

#define SEPARADOR 0x110000u

#include "lector.c"
#include "descomprimir.c"

//########################################################

//########################################################

int ipow(int base, int exp) {
    int result = 1;
    for (int i = 0; i < exp; i++) {
        result *= base;
    }
    return result;
}

//########################################################

static inline int write_u32_be(FILE *out, uint32_t v) {
    uint32_t be = htonl(v);                  // convierte a big-endian (network order)
    return (fwrite(&be, 1, sizeof be, out) == sizeof be) ? 0 : -1;
}

static inline uint64_t mask_n(unsigned n) {
    return (n == 64) ? ~0ULL : ((1ULL << n) - 1ULL);
}

static inline void append_bits_msb(uint64_t *w, unsigned int *pos, uint64_t v, unsigned int len) {
    assert(len >= 1 && len <= 64);
    assert(*pos + len <= 64);
    unsigned shift = 64 - (*pos + len);
    *w |= ( (v & mask_n(len)) << shift );
    *pos += len;
}

static inline uint64_t take_bits_msb(uint64_t *w, unsigned *pos, unsigned len) {
    assert(len >= 1 && len <= 64);
    assert(*pos >= len);

    if (len == 64) {
        uint64_t out = *w;
        *w = 0;
        *pos = 0;
        return out;
    }

    uint64_t out = (*w >> (64 - len)) & mask_n(len);
    *w <<= len;
    *pos -= len;
    return out;
}

static inline uint8_t take_byte_msb(uint64_t *w, unsigned *pos) {
    assert(*pos >= 8);
    uint8_t out = (uint8_t)(*w >> 56);
    *w <<= 8;
    *pos -= 8;
    return out;
}

static inline void flush_full_bytes(uint64_t *w, unsigned *pos, FILE *t) {
    while (*pos >= 8) {
        uint8_t b = take_byte_msb(w, pos);
        fwrite(&b, 1, 1, t);
    }
}

static inline void append_code_from_str_msb(uint64_t *w, unsigned *pos,
                                            const char *code, unsigned len,
                                            FILE *t) {
    unsigned i = 0;
    while (i < len) {
        unsigned room = 64 - *pos;
        unsigned chunk_len = len - i;
        if (chunk_len > room) chunk_len = room;

        uint64_t chunk = 0;
        for (unsigned j = 0; j < chunk_len; ++j) {
            chunk = (chunk << 1) | (code[i + j] == '1');
        }

        append_bits_msb(w, pos, chunk, chunk_len);
        flush_full_bytes(w, pos, t);
        i += chunk_len;
    }
}


static inline void append_byte_msb(uint64_t *w, unsigned *pos, unsigned char b, FILE *t) {
    append_bits_msb(w, pos, (uint64_t)b, 8);
    flush_full_bytes(w, pos, t);
}

static inline void flush_pad_zeros(uint64_t *w, unsigned *pos, FILE *t) {
    unsigned rem = *pos % 8;
    if (rem != 0) {
        unsigned pad = 8 - rem;
        append_bits_msb(w, pos, 0, pad);
    }
    flush_full_bytes(w, pos, t);
}

//########################################################

static int str_starts_with(const char *s, const char *pref) {
    size_t lp = strlen(pref);
    return strncmp(s, pref, lp) == 0;
}

static int str_ends_with(const char *s, const char *suf) {
    size_t ls = strlen(s);
    size_t lu = strlen(suf);
    return ls >= lu && strcmp(s + (ls - lu), suf) == 0;
}

static int is_regular_file(const char *path) {
    struct stat st;
    if (stat(path, &st) != 0) return 0;
    return S_ISREG(st.st_mode) ? 1 : 0;
}


static int cmp_strings(const void *a, const void *b) {
    const char * const *sa = (const char * const *)a;
    const char * const *sb = (const char * const *)b;
    return strcmp(*sa, *sb);
}

static char* extract_title_from_name(const char *name) {
    const char *pref = "temp_";
    const char *suf  = ".txt";
    if (!str_starts_with(name, pref) || !str_ends_with(name, suf)) return NULL;
    const char *start = name + strlen(pref);
    size_t base_len = strlen(name) - strlen(pref) - strlen(suf);
    char *title = (char*)malloc(base_len + 1);
    if (!title) return NULL;
    memcpy(title, start, base_len);
    title[base_len] = '\0';
    return title;
}

static int is_valid_bitstring(const char *s) {
    if (!s || !*s) return 0;
    for (const char *p = s; *p; ++p) {
        if (*p != '0' && *p != '1') return 0;
    }
    return 1;
}

static void write_ascii_as_bits(uint64_t *w, unsigned *pos, const char *txt, FILE *out) {
    for (const unsigned char *p = (const unsigned char*)txt; *p; ++p) {
        append_byte_msb(w, pos, *p, out);
    }
}

static int write_file_bytes_as_bits(uint64_t *w, unsigned *pos, const char *path, FILE *out) {
    FILE *in = fopen(path, "rb");
    if (!in) {
        fprintf(stderr, "No pude abrir '%s': %s\n", path, strerror(errno));
        return -1;
    }
    unsigned char buf[1 << 16];
    size_t n;
    while ((n = fread(buf, 1, sizeof buf, in)) > 0) {
        for (size_t i = 0; i < n; ++i) {
            append_byte_msb(w, pos, buf[i], out);
        }
    }
    if (ferror(in)) {
        fprintf(stderr, "Error leyendo '%s'\n", path);
        fclose(in);
        return -2;
    }
    fclose(in);
    return 0;
}

//########################################################

static inline void append_uint_bits_msb_stream(uint64_t *w, unsigned *pos,
                                               uint64_t val, unsigned bits,
                                               FILE *t) {
    // Empuja 'bits' desde el MSB de 'val' hacia abajo, respetando el límite de 64-pos
    while (bits > 0) {
        unsigned room = 64 - *pos;
        unsigned chunk = (bits < room) ? bits : room;
        unsigned shift = bits - chunk;                 // bits que quedan después de este chunk
        uint64_t piece = (val >> shift) & mask_n(chunk);
        append_bits_msb(w, pos, piece, chunk);
        flush_full_bytes(w, pos, t);
        bits -= chunk;
    }
}

_Static_assert(sizeof(wchar_t) * CHAR_BIT <= 64, "wchar_t demasiado grande");

static inline void append_uint_bits_msb_stream(uint64_t *w, unsigned *pos,
                                               uint64_t val, unsigned bits,
                                               FILE *t);




                                               
//########################################################

typedef struct{
    wchar_t c;
    char* codigo;
    int len;
} Diccionario;

typedef struct Nodo Nodo;

struct Nodo {
    Diccionario* d;
    unsigned int frecuencia;
    Nodo* izq;
    Nodo* der;
};

//########################################################

Nodo* arbol(int n, Nodo** nodosEmparejar){

    int size = n;
    for (int it = 0; it < n-1; ++it){

        Nodo* min1 = NULL;
        unsigned int m1 = UINT_MAX;
        int pos1 = -1;
        Nodo* min2 = NULL;
        unsigned int m2 = UINT_MAX;
        int pos2 = -1;
        //pos final a buscar es (n - it)
        int last = size - 1;
        for (int i = 0; i <= last; ++i){
            if (nodosEmparejar[i]->frecuencia <= m1){
                min2 = min1;
                m2 = m1;
                pos2 = pos1;

                min1 = nodosEmparejar[i];
                m1 = nodosEmparejar[i]->frecuencia;
                pos1 = i;

            } else if (nodosEmparejar[i]->frecuencia <= m2){
                min2 = nodosEmparejar[i];
                m2 = nodosEmparejar[i]->frecuencia;
                pos2 = i;
            }
        }

        Nodo* nodo = malloc(sizeof(Nodo));
        nodo->izq = min1;
        nodo->der = min2;
        nodo->frecuencia = m1 + m2;
        nodo->d = NULL;

        if (pos1 > pos2){
            int t = pos1;
            pos1 = pos2;
            pos2 = t;
        }

        nodosEmparejar[pos1] = nodo;
        nodosEmparejar[pos2] = nodosEmparejar[last];
        size--;

    }

    return nodosEmparejar[0];
}

unsigned int contarArbol(Nodo* arbol) {
    if(!arbol) return 0;
    return contarArbol(arbol->izq) + 1 + contarArbol(arbol->der);
}

void printArbol(Nodo* arbol){
    if (!arbol) return;

    wprintf(L"[");

    if (arbol->d){ //hoja

        wprintf(L"%lc:(%ld)", arbol->d->c, arbol->d->len);

        for (int i = 0; i < arbol->d->len; i++){
            wprintf(L"%c", arbol->d->codigo[i]);
        }

    } else { //nodo intermedio
        wprintf(L"-");
    }

    if (arbol->izq){ //hijo izquierda
        wprintf(L"(");
        printArbol(arbol->izq);
        wprintf(L")");
    }
    if (arbol->der){ //hijo derecho
        wprintf(L"(");
        printArbol(arbol->der);
        wprintf(L")");
    }

    wprintf(L"]");
}

void printArbolSizes(Nodo* arbol){
    if (!arbol) return;


    if (arbol->d){ //hoja

        wprintf(L"%lc:(%ld)\n", arbol->d->c, arbol->d->len);

        /*
        for (int i = 0; i < arbol->d->len; i++){
            wprintf(L"%c", arbol->d->codigo[i]);
        }
        */

    }

    if (arbol->izq){ //hijo izquierda
        printArbolSizes(arbol->izq);
    }
    if (arbol->der){ //hijo derecho
        printArbolSizes(arbol->der);
    }
}

void asignarCodigos(Nodo* arbol, char* codigo, int len){

    if (arbol->d != NULL){
        char* cod = malloc(sizeof(char)*len);
        memcpy(cod, codigo, (size_t)len);

        arbol->d->codigo = cod;
        arbol->d->len = len;
        return;
    }


    if (arbol->izq != NULL){
        char* nuevoCodigo = malloc(sizeof(char)*len + 1);
        memcpy(nuevoCodigo, codigo, (size_t)len);
        nuevoCodigo[len] = '0';
        asignarCodigos(arbol->izq, nuevoCodigo, len+1);
        free(nuevoCodigo);
    }

    if (arbol->der != NULL){
        char* nuevoCodigo = malloc(sizeof(char)*len + 1);
        memcpy(nuevoCodigo, codigo, (size_t)len);
        nuevoCodigo[len] = '1';
        asignarCodigos(arbol->der, nuevoCodigo, len+1);
        free(nuevoCodigo);
    }
}

//########################################################

void comprimirArchivo(Diccionario* diccionario, int lenDir, char* path, char* temp){

    FILE* f = fopen(path, "rb");
    if (!f) { //Error
        perror("fopen");
        _exit(1);
    }

    FILE* t = fopen(temp, "wb");
    if (!t) { //Error
        perror("fopen");
        _exit(1);
    }

    uint64_t buffer = 0;
    unsigned int pos = 0;
    wint_t wc;
    while ((wc = fgetwc(f)) != WEOF) {
        if ((unsigned)wc < UNICODE_SCALAR_MAX &&
            !((wc >= 0xD800 && wc <= 0xDFFF))) {
            
            int index = -1;
            for (int i = 0; i < lenDir; ++i){
                if ((wint_t)diccionario[i].c == wc){
                    index = i;
                    break;
                }
            }

            if (index == -1){
                append_bits_msb(&buffer, &pos, 0, 8);

                uint8_t byte;
                while (pos >= 8){ //more than 8 bits

                    byte = take_byte_msb(&buffer, &pos);
                    fwrite(&byte, sizeof(uint8_t), 1, t);
                    //pos -= 8; se hace en take byte
                }
                continue;
            }

            unsigned int len = diccionario[index].len;
            char* codigo = diccionario[index].codigo;

            append_code_from_str_msb(&buffer, &pos, codigo, len, t);
        }
    }

    if (pos > 0) {
        unsigned pad = (8 - (pos % 8)) % 8;        // 0..7
        if (pad) append_bits_msb(&buffer, &pos, 0, pad);
        while (pos >= 8) {
            uint8_t byte = take_byte_msb(&buffer, &pos);
            fwrite(&byte, sizeof(uint8_t), 1, t);
        }
    }

    fclose(f);
    fclose(t);
}



void comprimirSerial(Diccionario* diccionario, int lenDir){

    DIR *dir = opendir("Libros");
    if (!dir) {
        perror("opendir");
        return;
    }

    struct dirent *entry;
    struct stat sb;
    char path[1024];
    char temp[1024];

    while ((entry = readdir(dir)) != NULL) {
        const char *name = entry->d_name;

        // omitir especiales y temporales
        if (strcmp(name, ".") == 0 || strcmp(name, "..") == 0) continue;
        if (strncmp(name, "temp_", 5) == 0) continue;

        // ruta al archivo real
        snprintf(path, sizeof(path), "Libros/%s", name);
        if (stat(path, &sb) == -1) {
            fprintf(stderr, "stat(%s): %s\n", path, strerror(errno));
            continue;
        }
        if (!S_ISREG(sb.st_mode)) continue; // solo archivos regulares

        // ruta al archivo temporal
        snprintf(temp, sizeof(temp), "Libros/temp_%s", name);

        comprimirArchivo(diccionario, lenDir, path, temp);
    }

    closedir(dir);
}

void comprimirProcesos(Diccionario* diccionario, int lenDir) {

    DIR *dir = opendir("Libros");
    if (!dir) {
        perror("opendir");
        return;
    }

    int fd[2];
    if (pipe(fd) == -1) {
        perror("pipe");
        closedir(dir);
        return;
    }

    long nproc = sysconf(_SC_NPROCESSORS_ONLN);
    int MAX_CHILDREN = (nproc > 0) ? (int)nproc : 4;
    int active = 0;

    struct dirent *entry;
    struct stat sb;
    char path[1024];
    char temp[1024];

    while ((entry = readdir(dir)) != NULL) {
        const char *name = entry->d_name;

        // omitir especiales y temporales
        if (strcmp(name, ".") == 0 || strcmp(name, "..") == 0) continue;
        if (strncmp(name, "temp_", 5) == 0) continue;

        snprintf(path, sizeof(path), "Libros/%s", name);
        if (stat(path, &sb) == -1) continue;
        if (!S_ISREG(sb.st_mode)) continue;

        snprintf(temp, sizeof(temp), "Libros/temp_%s", name);

        pid_t pid = fork();
        if (pid == -1) {
            perror("fork");
            continue;
        }
        if (pid == 0) { // Hijo
            close(fd[0]); // cierra lectura del pipe

            comprimirArchivo(diccionario, lenDir, path, temp);

            
        } else { // Padre
            active++;
            while (active >= MAX_CHILDREN) {
                int status;
                if (wait(&status) > 0) active--;
                else break;
            }
        }
    }

    closedir(dir);

    close(fd[1]);

    char buf[512];
    ssize_t nread;
    while ((nread = read(fd[0], buf, sizeof(buf)-1)) > 0) {
        buf[nread] = '\0';
        fputs(buf, stdout);
    }
    close(fd[0]);

    int status;
    while (wait(&status) > 0) { /* notin */ }
}

//########################################################
//########################################################
//########################################################

static void guardarArbol_preorden_char_o_cero(const Nodo *n,
                                              uint64_t *w, unsigned *pos, FILE *out)
{
    if (!n) return;

    const unsigned BITS_WC = (unsigned)(sizeof(wchar_t) * CHAR_BIT);

    uint64_t v = 0;
    if (n->d != NULL) {
        // hoja: escribir el wchar_t completo
        v = (uint64_t)( (unsigned)n->d->c );
    } else {
        // interno: escribir 0
        v = 0;
    }
    append_uint_bits_msb_stream(w, pos, v, BITS_WC, out);

    guardarArbol_preorden_char_o_cero(n->izq, w, pos, out);
    guardarArbol_preorden_char_o_cero(n->der, w, pos, out);
}

int escribir_header_arbol_char0(const char *outfile, const Nodo *raiz, uint32_t len)
{
    if (!outfile || !raiz) {
        fprintf(stderr, "[header_arbol] parámetros inválidos\n");
        return 1;
    }
    FILE *out = fopen(outfile, "wb");
    if (!out) {
        fprintf(stderr, "No pude crear '%s': %s\n", outfile, strerror(errno));
        return 2;
    }

    if (write_u32_be(out, len) != 0) {
        fprintf(stderr, "No pude escribir len del árbol\n");
        fclose(out);
        return 3;
    }


    uint64_t w = 0; unsigned pos = 0;

    // PREORDEN: wchar_t completo en hojas, 0 en internos
    guardarArbol_preorden_char_o_cero(raiz, &w, &pos, out);

    // Alinear a byte para que luego puedas hacer append limpio
    flush_pad_zeros(&w, &pos, out);
    fclose(out);
    return 0;
}

//########################################################
//########################################################
//########################################################





















// Fusiona: TITULO -> SEP(bits) -> DATA -> SEP(bits)
// - Título = entre "temp_" y ".txt" si aplica (strip_txt_suffix=1 quita ".txt").
// - sep_bits = char* con '0' y '1' (terminado en '\0').
// - delete_temps: 1 para borrar los temp_* tras éxito.
int fusionar_temporales_con_separador_bits_del(const char *dirpath,
                                               const char *outfile,
                                               const char *sep_bits,
                                               int strip_txt_suffix,
                                               int delete_temps)
{
    if (!dirpath || !outfile || !sep_bits) {
        if (!dirpath)  fprintf(stderr, "[fusionar] dirpath NULL\n");
        if (!outfile)  fprintf(stderr, "[fusionar] outfile NULL\n");
        if (!sep_bits) fprintf(stderr, "[fusionar] sep_bits NULL\n");
        fprintf(stderr, "Parámetros inválidos\n");
        return 1;
    }
    if (!is_valid_bitstring(sep_bits)) {
        fprintf(stderr, "El separador debe contener solo '0' y '1' y no estar vacío\n");
        return 2;
    }

    DIR *d = opendir(dirpath);
    if (!d) {
        fprintf(stderr, "No pude abrir directorio '%s': %s\n", dirpath, strerror(errno));
        return 3;
    }

    size_t cap = 16, n = 0;
    char **paths = (char**)malloc(cap * sizeof(char*));
    if (!paths) { closedir(d); return 4; }

    struct dirent *ent;
    char path[PATH_MAX];

    // Evitar incluir el outfile si cae en el mismo dir
    char out_abs[PATH_MAX];
    if (!realpath(outfile, out_abs)) {
        if (outfile[0] == '/') {
            strncpy(out_abs, outfile, sizeof out_abs - 1);
            out_abs[sizeof out_abs - 1] = '\0';
        } else {
            char cwd[PATH_MAX];
            if (!getcwd(cwd, sizeof cwd)) cwd[0] = '\0';
            snprintf(out_abs, sizeof out_abs, "%s/%s", cwd, outfile);
        }
    }

    while ((ent = readdir(d)) != NULL) {
        const char *name = ent->d_name;
        if (strcmp(name, ".") == 0 || strcmp(name, "..") == 0) continue;
        if (strncmp(name, "temp_", 5) != 0) continue; // solo temporales

        int w = snprintf(path, sizeof path, "%s/%s", dirpath, name);
        if (w < 0 || (size_t)w >= sizeof path) continue;
        if (!is_regular_file(path)) continue;

        char abs[PATH_MAX];
        if (!realpath(path, abs)) strncpy(abs, path, sizeof abs - 1);
        abs[sizeof abs - 1] = '\0';
        if (strcmp(abs, out_abs) == 0) continue; // nunca incluir outfile

        if (n == cap) {
            cap *= 2;
            char **tmp = (char**)realloc(paths, cap * sizeof(char*));
            if (!tmp) { closedir(d); for (size_t i=0;i<n;i++) free(paths[i]); free(paths); return 5; }
            paths = tmp;
        }
        paths[n] = strdup(path);
        if (!paths[n]) { closedir(d); for (size_t i=0;i<n;i++) free(paths[i]); free(paths); return 6; }
        n++;
    }
    closedir(d);

    if (n == 0) {
        fprintf(stderr, "No hay archivos 'temp_*' en '%s'\n", dirpath);
        free(paths);
        return 7;
    }

    qsort(paths, n, sizeof(char*), cmp_strings);

    FILE *out = fopen(outfile, "ab");
    if (!out) {
        fprintf(stderr, "No pude crear '%s': %s\n", outfile, strerror(errno));
        for (size_t i=0;i<n;i++) free(paths[i]);
        free(paths);
        return 8;
    }

    uint64_t w = 0;
    unsigned pos = 0;
    unsigned sep_len = (unsigned)strlen(sep_bits);

    size_t processed = 0;
    for (size_t i = 0; i < n; ++i) {
        const char *full = paths[i];
        const char *name = strrchr(full, '/');
        name = name ? name + 1 : full; // "temp_...."

        // Título: quitar "temp_" y, si corresponde, ".txt"
        const char *after_pref = name + 5; // salta "temp_"
        size_t tlen = strlen(after_pref);
        if (strip_txt_suffix && tlen >= 4) {
            if (strcmp(after_pref + (tlen - 4), ".txt") == 0) tlen -= 4;
        }

        char *title = (char*)malloc(tlen + 1);
        if (!title) {
            fclose(out);
            for (size_t j=0;j<n;j++) free(paths[j]);
            free(paths);
            return 9;
        }
        memcpy(title, after_pref, tlen);
        title[tlen] = '\0';

        // SEPARADOR
        append_code_from_str_msb(&w, &pos, sep_bits, sep_len, out);
        append_code_from_str_msb(&w, &pos, sep_bits, sep_len, out);
        // Title
        //write_ascii_as_bits(&w, &pos, title, out);
        // Data
        if (write_file_bytes_as_bits(&w, &pos, full, out) != 0) {
            fprintf(stderr, "Fallo procesando '%s'\n", full);
            free(title);
            fclose(out);
            for (size_t j=0;j<n;j++) free(paths[j]);
            free(paths);
            return 10;
        }
        // SEPARADOR
        append_code_from_str_msb(&w, &pos, sep_bits, sep_len, out);

        free(title);
        processed++;
    }

    // Alinear a byte y cerrar
    flush_pad_zeros(&w, &pos, out);
    fflush(out);
    // fsync para robustez (opcional):
    // int fd = fileno(out); if (fd >= 0) fsync(fd);
    fclose(out);

    // Borrar temporales solo si todo salió OK
    if (delete_temps) {
        for (size_t i = 0; i < processed; ++i) {
            if (unlink(paths[i]) != 0) {
                fprintf(stderr, "Aviso: no pude borrar '%s': %s\n", paths[i], strerror(errno));
            }
        }
    }

    for (size_t i=0;i<n;i++) free(paths[i]);
    free(paths);

    fprintf(stderr, "Fusión OK: %zu archivos %s\n",
            processed, delete_temps ? "(y temporales borrados)" : "");
    return 0;
}



//########################################################
//########################################################
//########################################################

Diccionario* makeDictionary(uint64_t* frequences, Nodo** nodosEmparejar, int* len){

    Diccionario* diccionario = malloc(sizeof(Diccionario) * DICT_MAX);
    *len = 0;

    for (unsigned cp = 0; cp < UNICODE_SCALAR_MAX; ++cp) {
        if (cp >= 0xD800 && cp <= 0xDFFF) continue;   // omite surrogates
        if (frequences[cp] == 0) continue;                  // imprime solo los presentes

        int i = *len;

        diccionario[i].c = (wchar_t) cp;
        diccionario[i].len = 0;
        diccionario[i].codigo = NULL;

        nodosEmparejar[i] = malloc(sizeof(Nodo));
        nodosEmparejar[i]->der = NULL;
        nodosEmparejar[i]->izq = NULL;

        nodosEmparejar[i]->d = &diccionario[i];

        *len += 1;
    }

    free(frequences);
    return diccionario;

}


//########################################################

int testViejo(){

    setlocale(LC_ALL, "");

    //uint64_t* frequences = runFrequences();

    int dictLength = 4;
    Diccionario* diccionario = malloc(sizeof(Diccionario) * dictLength);

    for (int i = 0; i < dictLength; ++i){
        diccionario[i].len = 0;
    }

    diccionario[0].c = L'a';
    diccionario[0].codigo = malloc(sizeof(char) * 1);
    diccionario[0].len = 1;
    diccionario[0].codigo[0] = '0';

    diccionario[1].c = L'b';
    diccionario[1].codigo = malloc(sizeof(char) * 2);
    diccionario[1].len = 2;
    diccionario[1].codigo[0] = '1';
    diccionario[1].codigo[1] = '0';

    diccionario[2].c = L'c';
    diccionario[2].codigo = malloc(sizeof(char) * 3);
    diccionario[2].len = 3;
    diccionario[2].codigo[0] = '1';
    diccionario[2].codigo[1] = '1';
    diccionario[2].codigo[2] = '0';
    

    diccionario[3].c = L'd';
    diccionario[3].codigo = calloc(78, sizeof(char));
    diccionario[3].len = 2502;
    diccionario[3].codigo[0] = '1';
    diccionario[3].codigo[1] = '0';
    diccionario[3].codigo[2] = '1';
    diccionario[3].codigo[75] = '1';
    diccionario[3].codigo[76] = '0';
    diccionario[3].codigo[77] = '1';


    comprimirArchivo(diccionario, dictLength, "prueba0.txt", "salida.bin");

    //comprimirSerial(diccionario, dictLength);
    //comprimirProcesos(diccionario, dictLength);

    return 0;
}

int testNuevo(){

    setlocale(LC_ALL, "");

    uint64_t* frequences = frequenceSerial();
    Nodo** nodosEmparejar = malloc(sizeof(Nodo*) * DICT_MAX);

    int dictLength = 0;
    Diccionario* diccionario = makeDictionary(frequences, nodosEmparejar, &dictLength);

    //Separador SEPARADOR
    int separadorId = dictLength;
    diccionario[dictLength].c = (wchar_t) SEPARADOR;
    diccionario[dictLength].codigo = NULL;
    diccionario[dictLength].len = 0;

    nodosEmparejar[dictLength] = malloc(sizeof(Nodo));
    nodosEmparejar[dictLength]->d = &diccionario[dictLength];
    nodosEmparejar[dictLength]->frecuencia = 400;
    nodosEmparejar[dictLength]->izq = NULL;
    nodosEmparejar[dictLength]->der = NULL;
    dictLength++;

    //Basura 0
    diccionario[dictLength].c = (wchar_t) ' ';
    diccionario[dictLength].codigo = NULL;
    diccionario[dictLength].len = 0;

    nodosEmparejar[dictLength] = malloc(sizeof(Nodo));
    nodosEmparejar[dictLength]->d = &diccionario[dictLength];
    nodosEmparejar[dictLength]->frecuencia = 1000000000;
    nodosEmparejar[dictLength]->izq = NULL;
    nodosEmparejar[dictLength]->der = NULL;
    dictLength++;

    //Basura 1
    diccionario[dictLength].c = (wchar_t) ' ';
    diccionario[dictLength].codigo = NULL;
    diccionario[dictLength].len = 0;

    nodosEmparejar[dictLength] = malloc(sizeof(Nodo));
    nodosEmparejar[dictLength]->d = &diccionario[dictLength];
    nodosEmparejar[dictLength]->frecuencia = 1000000000;
    nodosEmparejar[dictLength]->izq = NULL;
    nodosEmparejar[dictLength]->der = NULL;
    dictLength++;




    

    Nodo* a = arbol(dictLength, nodosEmparejar);
    wprintf(L"Holi, termine el arbol\n");

    char* codigo = malloc(sizeof(char)*1);
    codigo[0] = 0;
    asignarCodigos(a, codigo, 0);
    wprintf(L"Holi, termine los codigos\n");

    unsigned int len = contarArbol(a);
    len++;
    len = len*32;
    //printf("Len: %d\n", len);

    /*
    int max = -1;
    for (int i = 0; i < dictLength; ++i){

        if(diccionario[i].len > max){
            max = diccionario[i].len;
        }
    }
    wprintf(L"\nMax size: %d\n", max);    
    */



    //printArbolSizes(a);

    




    //comprimirArchivo(diccionario, dictLength, "prueba0.txt", "salida.bin");

    comprimirSerial(diccionario, dictLength);
    //comprimirProcesos(diccionario, dictLength);
    wprintf(L"\nHoli, termine de comprimir\n");

    int L = diccionario[separadorId].len;
    char *SEPSTR = malloc((size_t)L + 1);
    memcpy(SEPSTR, diccionario[separadorId].codigo, (size_t)L);
    SEPSTR[L] = '\0';



    if (escribir_header_arbol_char0("Comprimido.bin", a, (uint32_t)len) != 0) {
        fprintf(stderr, "No se pudo escribir header del árbol\n");
        return 1;
    }


    //diccionario[dictLength].codigo es el codigo de separador
    int rc = fusionar_temporales_con_separador_bits_del("Libros",
                                                    "Comprimido.bin",
                                                    SEPSTR,
                                                    1,
                                                    1);

    if (rc != 0) fprintf(stderr, "Fusión fallo :c con codigo %d\n", rc);
    
    wprintf(L"\nBye, termine de fusionar\n");


    return 0;
}



static int leer_opcion(void) {
    for (;;) {
        printf("\n=== Menú ===\n");
        printf("  1) Comprimir .\\Libros\\* \n");
        printf("  2) Descomprimir .\\Comprimido.kirby \n");
        printf("> ");
        fflush(stdout);

        char buf[64];
        if (!fgets(buf, sizeof buf, stdin)) {
            return -1; // EOF o error de lectura
        }

        char *end = NULL;
        errno = 0;
        long v = strtol(buf, &end, 10);
        if (errno == 0 && end != buf && (v == 1 || v == 2)) {
            return (int)v;
        }
        printf("Entrada inválida. Escribe 1 o 2.\n");
    }
}

static inline uint64_t now_ms(void) {
    struct timespec ts;
    if (clock_gettime(CLOCK_MONOTONIC, &ts) != 0) {
        // en sistemas muy antiguos podía requerir -lrt (no en Debian moderno)
        perror("clock_gettime");
        return 0;
    }
    return (uint64_t)ts.tv_sec * 1000ULL + (uint64_t)ts.tv_nsec / 1000000ULL;
}


int main(){
    int opcion = leer_opcion();
    if (opcion == -1) {
        fprintf(stderr, "No se pudo leer la entrada (EOF).\n");
        return 1;
    }

    uint64_t t0, t1;

    switch (opcion) {
        case 1:
            printf("Elegiste la opción Comprimir Libros\n");
            t0 = now_ms();
            testNuevo();
            t1 = now_ms();
            break;

        case 2:
            printf("Elegiste la opción Descomprimir Libros\n");
            t0 = now_ms();
            descomprimir();
            t1 = now_ms();
            break;
    }

    printf("Tardó %.3f ms\n", (double)(t1 - t0));

    return 0;
}

void testTree(){

    setlocale(LC_ALL, "");

    int n = 4; //tamaño alfabeto
    Nodo** nodosEmparejar = malloc(sizeof(Nodo*) * n);


    Diccionario* diccionario = malloc(sizeof(Diccionario) * n);

    for (int i = 0; i < n; ++i){
        diccionario[i].codigo = NULL;
        diccionario[i].len = 0;
    }

    for (int i = 0; i < n; ++i){
        nodosEmparejar[i] = malloc(sizeof(Nodo));
        nodosEmparejar[i]->der = NULL;
        nodosEmparejar[i]->izq = NULL;
    }

    diccionario[0].c = L'a';
    nodosEmparejar[0]->frecuencia = 24;
    nodosEmparejar[0]->d = &diccionario[0];

    diccionario[1].c = L'b';
    nodosEmparejar[1]->frecuencia = 5;
    nodosEmparejar[1]->d = &diccionario[1];

    diccionario[2].c = L'c';
    nodosEmparejar[2]->frecuencia = 12;
    nodosEmparejar[2]->d = &diccionario[2];

    diccionario[3].c = L'd';
    nodosEmparejar[3]->frecuencia = 3;
    nodosEmparejar[3]->d = &diccionario[3];
    
    Nodo* a = arbol(n, nodosEmparejar);
    wprintf(L"Holi, termine el arbol\n");

    char* codigo = malloc(sizeof(char)*1);
    codigo[0] = 0;
    asignarCodigos(a, codigo, 0);
    wprintf(L"Holi, termine los codigos\n");

    printArbol(a);
    wprintf(L"\nHoli, termine de imprimir\n");
}
