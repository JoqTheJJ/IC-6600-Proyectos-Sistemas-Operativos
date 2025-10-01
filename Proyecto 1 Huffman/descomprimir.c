#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <inttypes.h>
#include <stdbool.h>
#include <ctype.h>
#include <limits.h>

#define SEPARADOR2 0x110000u

#ifndef NAME_MAX
#define NAME_MAX 255 
#endif

#ifndef PATH_MAX
#define PATH_MAX 4096       
#endif


typedef struct {
    // buffer crudo del archivo
    uint8_t *file_buf;
    size_t   file_len;

    // Seccion 0: Indice
    uint32_t dict_end_bit;   

    // Seccion 1: Diccionario
    uint32_t *dict_chars;    
    size_t    dict_count;    

    // Seccion 2: Libros
    uint8_t  *books_bits;        
    size_t    books_bits_count;  
} KirbyFile;

// helpers big-endian
static inline uint32_t rd_u32_be(const uint8_t *p) {
    return ((uint32_t)p[0] << 24) | ((uint32_t)p[1] << 16) | ((uint32_t)p[2] << 8) | (uint32_t)p[3];
}

// Lee un bit global del buffer
static inline uint8_t get_bit_msb(const uint8_t *buf, size_t bit_index) {
    size_t byte_i = bit_index >> 3;          // /8
    int bit_in_byte = 7 - (int)(bit_index & 7); // MSB-first
    return (uint8_t)((buf[byte_i] >> bit_in_byte) & 1u);
}

// IO archivo
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

    // finDiccionario
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

    // Diccionario: bits [32 .. dict_end_bit) ——  cada nodo = 32 bits (u32 BE)
    uint32_t dict_bits = out->dict_end_bit - 32u;
    out->dict_count = dict_bits / 32u;
    uint32_t dict_resto = dict_bits % 32u;
    if (dict_resto != 0u) {
        fprintf(stderr, "Aviso: el diccionario no está alineado a 32 bits (%" PRIu32 " bits extra ignorados)\n", dict_resto);
    }

    if (out->dict_count > 0) {
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

    // Libros: bits [dict_end_bit .. EOF)
    uint64_t books_bits_u64 = file_bits - (uint64_t)out->dict_end_bit;
    if (books_bits_u64 > (uint64_t)SIZE_MAX) {
        fprintf(stderr, "Demasiados bits en Libros para esta plataforma (%" PRIu64 ")\n", books_bits_u64);
        return 0;
    }
    out->books_bits_count = (size_t)books_bits_u64;
    if (out->books_bits_count > 0) {
        out->books_bits = (uint8_t*)malloc(out->books_bits_count);
        if (!out->books_bits) { fprintf(stderr, "Sin memoria para array de bits de Libros\n"); return 0; }

        size_t start_bit = (size_t)out->dict_end_bit;
        for (size_t k = 0; k < out->books_bits_count; ++k) {
            out->books_bits[k] = get_bit_msb(buf, start_bit + k);
        }
    }

    return 1;
}

// --- muestra tamaños y primeros elementos ---
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

static char *dup_cstr(const char *s) {
    if (!s) return NULL;
    size_t n = strlen(s) + 1;
    char *p = (char*)malloc(n);
    if (!p) return NULL;
    memcpy(p, s, n);
    return p;
}

// -------------------------------------- //

// ----- árbol -----
typedef struct Arbol {
    uint32_t c;            // 0 = interno, !=0 = hoja 
    struct Arbol *izq, *der;
} Arbol;

typedef struct Letter {
    uint32_t c;
    struct Letter *nxt;
} Letter;

typedef struct Book {
    Letter *first;
    Letter *last;
} Book;

typedef struct BookNode {
    Book *title;
    Book *book;
    struct BookNode *nxt;
} BookNode;

typedef struct Library {
    BookNode *first;
    BookNode *last;
}Library;

static void *xmalloc(size_t n) {
    void *p = malloc(n);
    if(!p){ perror("malloc"); exit(1); }
    return p;
}

static Arbol *nuevo(uint32_t c) {
    Arbol *n = xmalloc(sizeof *n);
    n->c = c; n->izq = n->der = NULL;
    return n;
}

Arbol *build_full_preorder_u32(const uint32_t *dict_chars, size_t *i, size_t n) {
    if (*i >= n){
        fprintf(stderr, "Entrada insuficiente (i=%zu, n=%zu)\n", *i, n);
        return NULL;
    }
    uint32_t t = dict_chars[(*i)++];
    Arbol *node = nuevo(t);

    if (t == 0){
        node->izq = build_full_preorder_u32(dict_chars, i, n);
        node->der = build_full_preorder_u32(dict_chars, i, n);
    }
    return node;
}

static void print_preorder_compacto(const Arbol *x) {
    if (!x) return;  
    if (x->c == 0){
        printf("0 ");
        print_preorder_compacto(x->izq);
        print_preorder_compacto(x->der);
    } else {
        printf("U+%04X ", x->c);
    }
}

static void free_tree(Arbol *x) {
    if(!x) return;
    free_tree(x->izq);
    free_tree(x->der);
    free(x);
}

static inline void book_init(Book *b) {
    b->first = b->last = NULL;
}

static bool book_push_back(Book *b, uint32_t c) {
    Letter *node = (Letter*)malloc(sizeof *node);
    if (!node) return false;
    node->c = c;
    node->nxt = NULL;

    if (!b->first) {            
        b->first = b->last = node;
    } else {
        b->last->nxt = node;    
        b->last = node;         
    }
    return true;
}

static void free_book(Book *b) {
    if (!b) return;
    for (Letter *p = b->first; p; ) {
        Letter *next = p->nxt;
        free(p);
        p = next;
    }
    free(b);
}


static Book *book_new(void) {
    Book *b = (Book*)calloc(1, sizeof *b);
    if (!b) return NULL;
    book_init(b);
    return b;
}

static inline void library_init(Library *l) {
    l->first = l->last = NULL;
}

static Library *library_new(void) {
    Library *l = (Library*)calloc(1, sizeof *l);
    if (!l) return NULL;
    library_init(l);
    return l;
}

int library_push(Library *l, Book *title, Book *book) {
    BookNode *node = (BookNode*)malloc(sizeof *node);
    if (!node) return -1;

    node->title = title;
    node->book = book;   
    node->nxt  = NULL;

    if (l->last) {
        l->last->nxt = node;   
    } else {
        l->first = node;        
    }
    l->last = node;
    return 0;
}

void free_library(Library *l) {
    BookNode *p = l->first;
    while (p) {
        BookNode *next = p->nxt;
        if (p->book) free_book(p->book);
        if (p->title) free_book(p->title);
        free(p);
        p = next;
    }
    l->first = l->last = NULL;
}

Arbol* moveArbol(short code, Arbol *pos) {
    if (!pos) return NULL;
    if (code == 0) {    // 0 -> Izq
        if (!pos->izq) return NULL;
        return pos->izq;
    } else {            // 1 -> Der
        if (!pos->der) return NULL;
        return pos->der;
    }
}

uint32_t getChar(Arbol* pos) {
    if (!pos) return 0;
    return pos->c;
}

void printBookAux(Letter *l){
    if(!l) 
        printf("\n");
    else {
        printf("U+%d | ", l->c);
        printBookAux(l->nxt);
    }
}

void printBook(Book *b) {
    printBookAux(b->first);
}

Book* convertBitsToChars(uint8_t* bits, size_t size, Arbol *root) {
    Book *book = book_new();
    if (!book || !root || !bits) return NULL;

    Arbol *pos = root;
    for (size_t i = 0; i < size; ++i) {
        pos = moveArbol(bits[i], pos);
        uint32_t c = getChar(pos);
        if (c != 0) {
            book_push_back(book, c);
            pos = root;
        }
    }

    return book;
}

Library* divideBooks(Book *books) {
    Library *result = library_new();
    Book *title = book_new();
    Book *book = book_new();
    short separadores = 0;

    Letter *l = books->first;
    while(l) {
        if (l->c == SEPARADOR2) {
            ++separadores;
            if (separadores > 3) {
                separadores = 1;
                library_push(result, title, book);
                title = book_new();
                book = book_new();
            }
        } else {
            if (separadores == 3) {
                book_push_back(book, l->c);
            } else {
                book_push_back(title, l->c);
            }
        }
       l = l->nxt; 
    }

    if (title || book) {
        library_push(result, title, book);
    } else {
        free_book(title);
        free_book(book);
    }

    return result;
}

static int u32_to_utf8(uint32_t cp, char out[4]) {
    // Reemplaza inválidos por U+FFFD
    if (cp > 0x10FFFF || (cp >= 0xD800 && cp <= 0xDFFF)) {
        out[0] = (char)0xEF; out[1] = (char)0xBF; out[2] = (char)0xBD;
        return 3;
    }
    if (cp <= 0x7F) { out[0] = (char)cp; return 1; }
    if (cp <= 0x7FF) {
        out[0] = (char)(0xC0 | (cp >> 6));
        out[1] = (char)(0x80 | (cp & 0x3F));
        return 2;
    }
    if (cp <= 0xFFFF) {
        out[0] = (char)(0xE0 | (cp >> 12));
        out[1] = (char)(0x80 | ((cp >> 6) & 0x3F));
        out[2] = (char)(0x80 | (cp & 0x3F));
        return 3;
    }
    out[0] = (char)(0xF0 | (cp >> 18));
    out[1] = (char)(0x80 | ((cp >> 12) & 0x3F));
    out[2] = (char)(0x80 | ((cp >> 6) & 0x3F));
    out[3] = (char)(0x80 | (cp & 0x3F));
    return 4;
}

static char *sanitize_filename(const char *utf8_in) {
    // Permitimos UTF-8, pero quitamos '/', controles y trims. Colapsamos espacios.
    size_t n = utf8_in ? strlen(utf8_in) : 0;
    // buffer generoso
    char *tmp = (char*)malloc(n + 1);
    if (!tmp) return NULL;

    // 1) filtra
    size_t w = 0;
    int last_space = 1; // para trim inicial
    for (size_t i = 0; i < n; ++i) {
        unsigned char c = (unsigned char)utf8_in[i];
        if (c == '/') { c = '_'; }                 // separador prohibido
        if (c == '\n' || c == '\r' || c == '\t') c = ' ';
        if (c < 0x20) continue;                    // drop controles
        if (c == ' ') {
            if (last_space) continue;              // colapsa espacios
            last_space = 1;
        } else {
            last_space = 0;
        }
        tmp[w++] = (char)c;
    }
    // trim final
    while (w > 0 && tmp[w-1] == ' ') --w;
    tmp[w] = '\0';

    // 2) evita "." o ".." y vacío
    if (w == 0 || (strcmp(tmp, ".") == 0) || (strcmp(tmp, "..") == 0)) {
        free(tmp);
        return dup_cstr("documento");
    }

    // 3) recorta para mantener bajo ~240 chars (dejando margen para " (n).txt")
    const size_t MAX_BASE = 240;
    if (w > MAX_BASE) {
        tmp[MAX_BASE] = '\0';
        w = MAX_BASE;
    }
    return tmp;
}

static int join_path(char *out, size_t cap, const char *dir, const char *name) {
    if (!dir || dir[0] == '\0') {
        return snprintf(out, cap, "%s", name) < (int)cap;
    } else {
        size_t dl = strlen(dir);
        int has_sep = (dl > 0 && (dir[dl-1] == '/'));
        if (has_sep) return snprintf(out, cap, "%s%s", dir, name) < (int)cap;
        return snprintf(out, cap, "%s/%s", dir, name) < (int)cap;
    }
}

static char *book_to_utf8_string(const Book *b, size_t *out_len) {
    size_t cap = 256, len = 0;
    char *buf = (char*)malloc(cap);
    if (!buf) return NULL;

    for (const Letter *p = b && b->first ? b->first : NULL; p; p = p->nxt) {
        char tmp[4];
        int n = u32_to_utf8(p->c, tmp);
        if (len + (size_t)n + 1 > cap) { // +1 para '\0'
            size_t newcap = cap * 2;
            while (len + (size_t)n + 1 > newcap) newcap *= 2;
            char *nb = (char*)realloc(buf, newcap);
            if (!nb) { free(buf); return NULL; }
            cap = newcap; buf = nb;
        }
        memcpy(buf + len, tmp, (size_t)n);
        len += (size_t)n;
    }
    buf[len] = '\0';
    if (out_len) *out_len = len;
    return buf;
}

static FILE *open_unique_txt(const char *dir, const char *base_no_ext, char *out_path, size_t out_cap) {
    // intenta base.txt, base (2).txt, base (3).txt, ...
    for (int i = 1; i <= 9999; ++i) {
        char name[NAME_MAX + 64];
        if (i == 1) snprintf(name, sizeof(name), "%s.txt", base_no_ext);
        else        snprintf(name, sizeof(name), "%s (%d).txt", base_no_ext, i);

        if (!join_path(out_path, out_cap, dir, name)) continue;

        // "wx" (fail si existe). Si tu libc no soporta "x", puedes simular con open(O_CREAT|O_EXCL).
        FILE *fp = fopen(out_path, "wx");
        if (fp) return fp;
        if (errno != EEXIST) {
            // si falló por otro motivo, devolvemos NULL
            return NULL;
        }
        // si existía, probamos con el siguiente sufijo
    }
    errno = EEXIST;
    return NULL;
}

static int write_book_utf8(const Book *book, FILE *fp) {
    for (const Letter *p = book && book->first ? book->first : NULL; p; p = p->nxt) {
        char tmp[4];
        int n = u32_to_utf8(p->c, tmp);
        if ((size_t)fwrite(tmp, 1, (size_t)n, fp) != (size_t)n) return 0;
    }
    return 1;
}

int library_export_to_txts(const Library *lib, const char *output_dir, size_t *out_files_written) {
    size_t written = 0;
    char path[PATH_MAX];

    for (const BookNode *node = lib ? lib->first : NULL; node; node = node->nxt) {
        // 1) título como UTF-8
        size_t tlen = 0;
        char *title_utf8 = book_to_utf8_string(node->title, &tlen);
        if (!title_utf8) return 0;

        // 2) base de nombre
        char *base = NULL;
        if (tlen == 0) {
            base = dup_cstr("documento");
        } else {
            base = sanitize_filename(title_utf8);
        }
        free(title_utf8);
        if (!base) return 0;

        // 3) abrir archivo único con ".txt"
        FILE *fp = open_unique_txt(output_dir, base, path, sizeof(path));
        if (!fp) { free(base); return 0; }

        // 4) escribir el libro (contenido)
        int ok = write_book_utf8(node->book, fp);
        int cerr = fclose(fp);
        if (!ok || cerr != 0) { free(base); return 0; }

        // (opcional) log:
        // fprintf(stderr, "Escrito: %s\n", path);

        ++written;
        free(base);
    }

    if (out_files_written) *out_files_written = written;
    return 1;
}

int descomprimir() {
    const char *path = "comprimido.kirby";

    KirbyFile kf;
    if (!parse_kirby_be(path, &kf)) return 2;

    //dump_demo(&kf);

    size_t i = 0;
    Arbol *root = build_full_preorder_u32(kf.dict_chars, &i, kf.dict_count);

    Book *book = convertBitsToChars(kf.books_bits, kf.books_bits_count, root);

    //printBook(book);

    Library *books = divideBooks(book);

    size_t archivos = 0;
    if (!library_export_to_txts(books, "./salida", &archivos)) {
        perror("exportando txts");
    } else {
        printf("Exportados %zu archivos .txt\n", archivos);
    }

    free_kirby(&kf);
    free_tree(root);
    free_book(book);
    free_library(books);
    return 0;
}

/*
int main() {
    descomprimir();
    return 0;
}
*/
