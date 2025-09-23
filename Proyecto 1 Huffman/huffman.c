
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdint.h>
#include <limits.h>
#include <assert.h>

#define _DEFAULT_SOURCE
#include <dirent.h>

#include <sys/stat.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>

#include <wchar.h>
#include <locale.h>

#define UNICODE_SCALAR_MAX 0x110000u 



#include "lector.c"






int ipow(int base, int exp) {
    int result = 1;
    for (int i = 0; i < exp; i++) {
        result *= base;
    }
    return result;
}

//########################################################

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

    FILE* f = fopen(path, "r");
    if (!f) { //Error
        perror("fopen");
        _exit(1);
    }

    FILE* t = fopen(temp, "w");
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

            unsigned int len = diccionario[index].len;
            char* codigo = diccionario[index].codigo;

            uint64_t cod = 0;
            for (unsigned int index = 0; index < len; ++index){
                if (codigo[index] == '1'){
                    cod += ipow(2, index);
                }
            }
            append_bits_msb(&buffer, &pos, cod, len);


            uint8_t byte;
            while (pos >= 8){ //more than 8 bits

                byte = take_byte_msb(&buffer, &pos);
                fwrite(&byte, sizeof(uint8_t), 1, t);

                //pos -= 8; se hace en take byte
            }

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


void comprimir(Diccionario* diccionario, int lenDir, DIR* directorio) {
    (void)directorio; // sin uso, mantenemos la firma original

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
        if (pid == 0) {
            // ===== Hijo =====
            close(fd[0]); // cierra lectura del pipe

            // Comprensión individual con tu función (¡intocable!)
            comprimirArchivo(diccionario, lenDir, path, temp);

            // Renombrado atómico: sustituye original por el comprimido
            if (rename(temp, path) == -1) {
                char msg[512];
                int n = snprintf(msg, sizeof(msg),
                                 "Hijo %d ERROR renombrando %s -> %s: %s\n",
                                 getpid(), temp, path, strerror(errno));
                if (n > 0) write(fd[1], msg, (size_t)n);
                close(fd[1]);
                _exit(1);
            } else {
                char msg[512];
                int n = snprintf(msg, sizeof(msg),
                                 "Hijo %d OK: %s comprimido\n", getpid(), name);
                if (n > 0) write(fd[1], msg, (size_t)n);
                close(fd[1]);
                _exit(0);
            }
        } else {
            // ===== Padre =====
            active++;
            // Limitar nº de hijos simultáneos
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

int testOpenDir2() {
    DIR *dir = opendir("Libros");
    struct dirent *entry;
    struct stat sb;
    char path[1024];
    char temp[1024];

    if (!dir) {
        perror("opendir");
        return 1;
    }

    int fd[2];
    if (pipe(fd) == -1) {
        perror("pipe");
        return 1;
    }

    while ((entry = readdir(dir)) != NULL) {
        snprintf(path, sizeof(path), "Libros/%s", entry->d_name);
        snprintf(temp, sizeof(temp), "Libros/temp_%s", entry->d_name);
        if (stat(path, &sb) == -1) continue;

        if (S_ISREG(sb.st_mode)) {
            pid_t pid = fork();
            if (pid == 0) {
                // Hijo
                close(fd[0]); // cierra lectura

                FILE *f = fopen(path, "r");
                if (!f) {
                    perror("fopen");
                    _exit(1);
                }

                char buffer[256];
                if (fgets(buffer, sizeof(buffer), f)) {
                    char msg[512];
                    snprintf(msg, sizeof(msg), "Hijo %d: %s -> %s",
                             getpid(), entry->d_name, buffer);
                    write(fd[1], msg, strlen(msg));
                }
                fclose(f);

                close(fd[1]); // cerrar escritura
                _exit(0);
            }
            // Padre: sigue iterando
        }
    }

    closedir(dir);
    close(fd[1]); // padre cierra escritura

    // Leer mensajes del pipe
    char buf[512];
    ssize_t n;
    while ((n = read(fd[0], buf, sizeof(buf)-1)) > 0) {
        buf[n] = '\0';
        printf("Padre recibió: %s\n", buf);
    }
    close(fd[0]);

    // Espera a los hijos
    int status;
    while (wait(&status) > 0);

    return 0;
}

int testOpenDir() {
    DIR *dir = opendir("Libros");
    struct dirent *entry;
    struct stat sb;
    char path[1024];

    if (!dir) {
        perror("opendir");
        return 1;
    }

    while ((entry = readdir(dir)) != NULL) {
        snprintf(path, sizeof(path), "Libros/%s", entry->d_name);
        if (stat(path, &sb) == -1) continue;

        if (S_ISREG(sb.st_mode)) {
            pid_t pid = fork();
            if (pid == 0) {
                // Hijo: procesa el archivo
                printf("Hijo %d procesando %s\n", getpid(), entry->d_name);
                // ... abrir archivo, crear temp, etc ...
                _exit(0);  // importante: salir del hijo
            }
            // Padre: sigue con el loop, no hace nada aquí
        }
    }

    closedir(dir);

    // Padre espera a todos los hijos
    int status;
    pid_t wpid;
    while ((wpid = wait(&status)) > 0) {
        if (WIFEXITED(status)) {
            //printf("Hijo %d terminó con %d\n", wpid, WEXITSTATUS(status));
        }
    }
}

//########################################################

int main(){

    //uint64_t* frequences = runFrequences();

    int dictLength = 4;
    Diccionario* diccionario = malloc(sizeof(Diccionario) * dictLength);

    for (int i = 0; i < dictLength; ++i){
        diccionario[i].len = 0;
    }

    diccionario[0].c = L'a';
    diccionario[0].codigo = malloc(sizeof(char) * 1);
    diccionario[0].codigo[0] = '0';

    diccionario[1].c = L'b';
    diccionario[1].codigo = malloc(sizeof(char) * 2);
    diccionario[1].codigo[0] = '1';
    diccionario[1].codigo[1] = '0';

    diccionario[2].c = L'c';
    diccionario[2].codigo = malloc(sizeof(char) * 3);
    diccionario[2].codigo[0] = '1';
    diccionario[2].codigo[1] = '1';
    diccionario[2].codigo[2] = '0';
    

    diccionario[3].c = L'd';
    diccionario[3].codigo = malloc(sizeof(char) * 3);
    diccionario[3].codigo[0] = '1';
    diccionario[3].codigo[1] = '1';
    diccionario[3].codigo[2] = '1';



    comprimir(diccionario, dictLength, NULL);

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
