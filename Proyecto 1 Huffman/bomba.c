#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <dirent.h>
#include <sys/stat.h>

#ifdef _WIN32
#include <direct.h>
#define mkdir(dir, mode) _mkdir(dir)
#define SEPARATOR "\\"
#else
#define SEPARATOR "/"
#endif

#define MAX_BOOKS 100
#define MAX_FILENAME 256
#define MAX_TREE_NODES 512

typedef struct HuffmanNode {
    unsigned char data;
    unsigned freq;
    struct HuffmanNode *left, *right;
} HuffmanNode;

typedef struct {
    unsigned char data;
    unsigned freq;
} FreqEntry;

typedef struct {
    unsigned char code[256];
    int length;
} HuffmanCode;

typedef struct {
    char filename[MAX_FILENAME];
    uint32_t original_size;
    uint32_t compressed_size;
    uint32_t data_offset;
    HuffmanCode codes[256];
} BookMetadata;

HuffmanNode* create_node(unsigned char data, unsigned freq) {
    HuffmanNode* node = (HuffmanNode*)malloc(sizeof(HuffmanNode));
    node->data = data;
    node->freq = freq;
    node->left = node->right = NULL;
    return node;
}

void swap_nodes(HuffmanNode** a, HuffmanNode** b) {
    HuffmanNode* temp = *a;
    *a = *b;
    *b = temp;
}

void min_heapify(HuffmanNode** nodes, int size, int i) {
    int smallest = i;
    int left = 2 * i + 1;
    int right = 2 * i + 2;

    if (left < size && nodes[left]->freq < nodes[smallest]->freq)
        smallest = left;
    if (right < size && nodes[right]->freq < nodes[smallest]->freq)
        smallest = right;

    if (smallest != i) {
        swap_nodes(&nodes[i], &nodes[smallest]);
        min_heapify(nodes, size, smallest);
    }
}

HuffmanNode* extract_min(HuffmanNode** nodes, int* size) {
    HuffmanNode* min = nodes[0];
    nodes[0] = nodes[(*size) - 1];
    (*size)--;
    min_heapify(nodes, *size, 0);
    return min;
}

void insert_node(HuffmanNode** nodes, int* size, HuffmanNode* node) {
    (*size)++;
    int i = (*size) - 1;

    while (i > 0 && node->freq < nodes[(i - 1) / 2]->freq) {
        nodes[i] = nodes[(i - 1) / 2];
        i = (i - 1) / 2;
    }
    nodes[i] = node;
}

void build_min_heap(HuffmanNode** nodes, int size) {
    for (int i = size / 2 - 1; i >= 0; i--)
        min_heapify(nodes, size, i);
}

void generate_codes(HuffmanNode* root, HuffmanCode* codes, unsigned char* code, int depth) {
    if (root->left == NULL && root->right == NULL) {
        if (depth == 0) {                  // Archivo con un único símbolo distinto
            codes[root->data].code[0] = 0; // Asignamos un bit (0)
            codes[root->data].length = 1;  // Longitud 1 para emitir bits
        } else {
            for (int i = 0; i < depth; i++) {
                codes[root->data].code[i] = code[i];
            }
            codes[root->data].length = depth;
        }
        return;
    }

    if (root->left) {
        code[depth] = 0;
        generate_codes(root->left, codes, code, depth + 1);
    }

    if (root->right) {
        code[depth] = 1;
        generate_codes(root->right, codes, code, depth + 1);
    }
}

HuffmanNode* build_huffman_tree(FreqEntry* freq_table, int size) {
    HuffmanNode *left, *right, *top;
    HuffmanNode* nodes[MAX_TREE_NODES];
    int heap_size = 0;

    for (int i = 0; i < size; i++) {
        if (freq_table[i].freq > 0) {
            nodes[heap_size++] = create_node(freq_table[i].data, freq_table[i].freq);
        }
    }

    if (heap_size == 0) return NULL;

    build_min_heap(nodes, heap_size);

    while (heap_size > 1) {
        left = extract_min(nodes, &heap_size);
        right = extract_min(nodes, &heap_size);

        top = create_node(0, left->freq + right->freq);
        top->left = left;
        top->right = right;

        insert_node(nodes, &heap_size, top);
    }

    return extract_min(nodes, &heap_size);
}

void calculate_frequency(const char* filename, uint32_t* freq_table) {
    FILE* file = fopen(filename, "rb");
    if (!file) {
        printf("Error abriendo archivo: %s\n", filename);
        return;
    }

    for (int i = 0; i < 256; i++) {
        freq_table[i] = 0;
    }

    unsigned char buffer[1024];
    size_t bytes_read;

    while ((bytes_read = fread(buffer, 1, sizeof(buffer), file)) > 0) {
        for (size_t i = 0; i < bytes_read; i++) {
            freq_table[buffer[i]]++;
        }
    }

    fclose(file);
}

void compress_file(const char* input_file, const char* output_file,
                   BookMetadata* metadata, HuffmanCode* codes) {
    FILE* input = fopen(input_file, "rb");
    FILE* output = fopen(output_file, "ab"); // abrimos en append por cada libro

    if (!input || !output) {
        printf("Error abriendo archivos\n");
        if (input) fclose(input);
        if (output) fclose(output);
        return;
    }

    // Posición de inicio de los datos comprimidos de este archivo
    metadata->data_offset = ftell(output);

    unsigned char current_byte = 0;
    int bit_count = 0;
    unsigned char buffer[1024];
    size_t bytes_read;
    uint32_t compressed_bits = 0;

    while ((bytes_read = fread(buffer, 1, sizeof(buffer), input)) > 0) {
        for (size_t i = 0; i < bytes_read; i++) {
            unsigned char c = buffer[i];
            for (int j = 0; j < codes[c].length; j++) {
                current_byte = (current_byte << 1) | (codes[c].code[j] & 1);
                bit_count++;

                if (bit_count == 8) {
                    fwrite(&current_byte, 1, 1, output);
                    current_byte = 0;
                    bit_count = 0;
                    compressed_bits += 8;
                }
            }
        }
    }

    if (bit_count > 0) {
        current_byte <<= (8 - bit_count); // padding con ceros a la izquierda
        fwrite(&current_byte, 1, 1, output);
        compressed_bits += bit_count;
    }

    metadata->compressed_size = (compressed_bits + 7) / 8;

    fseek(input, 0, SEEK_END);
    metadata->original_size = (uint32_t)ftell(input);

    fclose(input);
    fclose(output);
}

void decompress_file(const char* input_file, const char* output_file,
                     BookMetadata* metadata, HuffmanNode* root) {
    FILE* input = fopen(input_file, "rb");
    FILE* output = fopen(output_file, "wb");

    if (!input || !output) {
        printf("Error abriendo archivos\n");
        if (input) fclose(input);
        if (output) fclose(output);
        return;
    }

    fseek(input, metadata->data_offset, SEEK_SET);

    HuffmanNode* current = root;
    unsigned char byte;
    uint32_t bytes_out = 0;

    // Leemos exactamente compressed_size bytes de flujo comprimido
    for (uint32_t b = 0; b < metadata->compressed_size; b++) {
        if (fread(&byte, 1, 1, input) != 1) break;

        for (int bit_pos = 7; bit_pos >= 0; bit_pos--) {
            int bit = (byte >> bit_pos) & 1;
            current = bit ? current->right : current->left;

            if (current == NULL) { // stream/códigos corruptos
                printf("  ERROR - flujo comprimido/códigos inconsistentes\n");
                fclose(input); fclose(output);
                return;
            }

            if (current->left == NULL && current->right == NULL) {
                fwrite(&current->data, 1, 1, output);
                bytes_out++;
                if (bytes_out >= metadata->original_size) { // Parar exactamente al recuperar original_size bytes
                    fclose(input);
                    fclose(output);
                    return;
                }
                current = root;
            }
        }
    }

    fclose(input);
    fclose(output);
}

void free_tree(HuffmanNode* root) {
    if (root == NULL) return;
    free_tree(root->left);
    free_tree(root->right);
    free(root);
}

int is_regular_file(const char* path) {
    struct stat path_stat;
    if (stat(path, &path_stat) != 0) {
        return 0;
    }
    return S_ISREG(path_stat.st_mode);
}

void create_directory(const char* dir) {
    struct stat st = {0};
    if (stat(dir, &st) == -1) {
#ifdef _WIN32
        _mkdir(dir);
#else
        mkdir(dir, 0755);
#endif
    }
}

int get_books_from_directory(const char* dir_path, char book_files[][MAX_FILENAME]) {
    DIR *dir;
    struct dirent *entry;
    int count = 0;

    printf("Buscando archivos en la carpeta: %s\n", dir_path);

    dir = opendir(dir_path);
    if (dir == NULL) {
        printf("Error: No se pudo abrir la carpeta '%s'\n", dir_path);
        return 0;
    }

    while ((entry = readdir(dir)) != NULL && count < MAX_BOOKS) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }

        char full_path[MAX_FILENAME * 2];
        snprintf(full_path, sizeof(full_path), "%s%s%s", dir_path, SEPARATOR, entry->d_name);

        if (is_regular_file(full_path)) {
            strncpy(book_files[count], full_path, MAX_FILENAME - 1);
            book_files[count][MAX_FILENAME - 1] = '\0'; // asegurar terminador
            count++;
            printf("  Encontrado: %s\n", entry->d_name);
        }
    }

    closedir(dir);

    if (count == 0) {
        printf("No se encontraron archivos en la carpeta '%s'\n", dir_path);
    } else {
        printf("Total de archivos encontrados: %d\n", count);
    }

    return count;
}

// Reconstruye el árbol EXACTO usando los códigos guardados
HuffmanNode* rebuild_tree_from_codes(HuffmanCode* codes) {
    HuffmanNode* root = create_node(0, 0);

    for (int i = 0; i < 256; i++) {
        if (codes[i].length > 0) {
            HuffmanNode* current = root;
            for (int j = 0; j < codes[i].length; j++) {
                if (codes[i].code[j] == 0) {
                    if (!current->left) current->left = create_node(0, 0);
                    current = current->left;
                } else {
                    if (!current->right) current->right = create_node(0, 0);
                    current = current->right;
                }
            }
            current->data = (unsigned char)i; // asignar el carácter a la hoja
        }
    }
    return root;
}

void compress_books(const char* input_dir, const char* output_file) {
    char book_files[MAX_BOOKS][MAX_FILENAME];
    int num_books = get_books_from_directory(input_dir, book_files);

    if (num_books == 0) {
        printf("No hay archivos para comprimir.\n");
        return;
    }

    BookMetadata metadata[MAX_BOOKS] = {0};

    // 1) Crear el archivo y escribir encabezado vacío; cerrar inmediatamente
    FILE* output = fopen(output_file, "wb");
    if (!output) {
        printf("Error creando archivo de salida: %s\n", output_file);
        return;
    }
    fwrite(metadata, sizeof(BookMetadata), MAX_BOOKS, output);
    fclose(output);

    printf("\nIniciando compresión...\n");

    uint32_t total_original = 0;
    uint32_t total_compressed = 0;

    for (int i = 0; i < num_books; i++) {
        printf("Comprimiendo %d/%d: %s\n", i + 1, num_books, book_files[i]);

        const char* filename_only = strrchr(book_files[i], SEPARATOR[0]);
        if (filename_only) {
            strncpy(metadata[i].filename, filename_only + 1, MAX_FILENAME - 1);
        } else {
            strncpy(metadata[i].filename, book_files[i], MAX_FILENAME - 1);
        }
        metadata[i].filename[MAX_FILENAME - 1] = '\0';

        // Calcular frecuencias en un arreglo local
        uint32_t freq[256] = {0};
        calculate_frequency(book_files[i], freq);

        // Convertir a FreqEntry para construir el árbol
        FreqEntry freq_table[256];
        for (int j = 0; j < 256; j++) {
            freq_table[j].data = (unsigned char)j;
            freq_table[j].freq = freq[j];
        }

        HuffmanNode* root = build_huffman_tree(freq_table, 256);
        if (!root) {
            printf("Error: No se pudo comprimir %s (archivo vacío o error)\n", book_files[i]);
            continue;
        }

        // Generar códigos y guardarlos en metadata
        unsigned char temp_code[256];
        memset(metadata[i].codes, 0, sizeof(metadata[i].codes));
        generate_codes(root, metadata[i].codes, temp_code, 0);

        // Comprimir archivo (esta función setea data_offset y tamaños)
        compress_file(book_files[i], output_file, &metadata[i], metadata[i].codes);

        printf("  Comprimido: %u -> %u bytes\n", metadata[i].original_size, metadata[i].compressed_size);

        total_original += metadata[i].original_size;
        total_compressed += metadata[i].compressed_size;

        free_tree(root);
    }

    // 2) Reabrir el archivo y actualizar el encabezado con la metadata correcta
    output = fopen(output_file, "rb+");
    if (!output) {
        printf("Error reabriendo archivo de salida para actualizar metadata: %s\n", output_file);
        return;
    }
    fseek(output, 0, SEEK_SET);
    fwrite(metadata, sizeof(BookMetadata), MAX_BOOKS, output);
    fclose(output);

    printf("\n=== COMPRESIÓN COMPLETADA ===\n");
    printf("Archivos: %d\n", num_books);
    printf("Tamaño original: %u bytes\n", total_original);
    printf("Tamaño comprimido: %u bytes\n", total_compressed);
    printf("Archivo de salida: %s\n", output_file);
}

void decompress_books(const char* input_file, const char* output_dir) {
    FILE* input = fopen(input_file, "rb");
    if (!input) {
        printf("Error abriendo archivo comprimido: %s\n", input_file);
        return;
    }

    create_directory(output_dir);

    BookMetadata metadata[MAX_BOOKS];
    if (fread(metadata, sizeof(BookMetadata), MAX_BOOKS, input) != MAX_BOOKS) {
        printf("Error leyendo metadatos\n");
        fclose(input);
        return;
    }
    fclose(input);

    printf("Descomprimiendo archivos...\n");

    int books_processed = 0;

    for (int i = 0; i < MAX_BOOKS; i++) {
        if (metadata[i].original_size == 0) continue;

        char output_path[MAX_FILENAME * 2];
        snprintf(output_path, sizeof(output_path), "%s%s%s", output_dir, SEPARATOR, metadata[i].filename);

        printf("Descomprimiendo: %s\n", metadata[i].filename);

        // Reconstruir el árbol EXACTO usando los códigos guardados
        HuffmanNode* root = rebuild_tree_from_codes(metadata[i].codes);
        if (root) {
            decompress_file(input_file, output_path, &metadata[i], root);
            free_tree(root);
            printf("  OK - %u bytes recuperados\n", metadata[i].original_size);
        } else {
            printf("  ERROR - No se pudo reconstruir el árbol\n");
        }

        books_processed++;
    }

    printf("\n=== DESCOMPRESIÓN COMPLETADA ===\n");
    printf("Archivos descomprimidos: %d\n", books_processed);
    printf("Directorio de salida: %s\n", output_dir);
}

void print_usage(const char* program_name) {
    printf("Compresor Huffman para Libros\n");
    printf("=============================\n\n");
    printf("Uso:\n");
    printf("  %s -c <archivo_comprimido>          Comprime todos los archivos de la carpeta 'libros'\n", program_name);
    printf("  %s -d <archivo_comprimido>          Descomprime a la carpeta 'libros_descomprimidos'\n", program_name);
    printf("\nEjemplos:\n");
    printf("  %s -c libros.bin\n", program_name);
    printf("  %s -d libros.bin\n", program_name);
}

int main(int argc, char* argv[]) {
    printf("Compresor Huffman para Libros\n");
    printf("=============================\n\n");

    if (argc == 3 && strcmp(argv[1], "-c") == 0) {
        compress_books("libros", argv[2]);
    } else if (argc == 3 && strcmp(argv[1], "-d") == 0) {
        decompress_books(argv[2], "libros_descomprimidos");
    } else {
        print_usage(argv[0]);
        return 1;
    }

    return 0;
}