
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <limits.h>

#include <wchar.h>
#include <locale.h>

#define DICT "0123456789abcdefghijklmnopqrstuvwxyz"

int ipow(int base, int exp) {
    int result = 1;
    for (int i = 0; i < exp; i++) {
        result *= base;
    }
    return result;
}

//########################################################

typedef struct{
    wchar_t c;
    int codigo;
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
        unsigned int m1 = INT_MAX;
        int pos1 = -1;
        Nodo* min2 = NULL;
        unsigned int m2 = INT_MAX;
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

    if (arbol->d){
        wprintf(L"[%lc", arbol->d->c);
    } else {
        wprintf(L"[-");
    }

    if (arbol->izq){
        wprintf(L"(");
        printArbol(arbol->izq);
        wprintf(L")");
    }
    if (arbol->der){
        wprintf(L"(");
        printArbol(arbol->der);
        wprintf(L")");
    }

    wprintf(L"]");
}

//########################################################

int main(){

    setlocale(LC_ALL, "");

    int n = 4; //tamaño alfabeto
    Nodo** nodosEmparejar = malloc(sizeof(Nodo*) * n);


    Diccionario* diccionario = malloc(sizeof(Diccionario) * n);

    for (int i = 0; i < n; ++i){
        nodosEmparejar[i] = malloc(sizeof(Nodo));
        nodosEmparejar[i]->der = NULL;
        nodosEmparejar[i]->izq = NULL;
    }

    diccionario[0].c = L'a';
    diccionario[0].codigo = 0;
    nodosEmparejar[0]->frecuencia = 24;
    nodosEmparejar[0]->d = &diccionario[0];

    diccionario[1].c = L'b';
    diccionario[1].codigo = 0;
    nodosEmparejar[1]->frecuencia = 5;
    nodosEmparejar[1]->d = &diccionario[1];

    diccionario[2].c = L'c';
    diccionario[2].codigo = 0;
    nodosEmparejar[2]->frecuencia = 12;
    nodosEmparejar[2]->d = &diccionario[2];

    diccionario[3].c = L'd';
    diccionario[3].codigo = 0;
    nodosEmparejar[3]->frecuencia = 3;
    nodosEmparejar[3]->d = &diccionario[3];
    
    Nodo* a = arbol(n, nodosEmparejar);
    wprintf(L"Holi, termine el arbol\n");

    printArbol(a);
    wprintf(L"\nHoli, termine de imprimir\n");
}



//[-([-(])(])])]