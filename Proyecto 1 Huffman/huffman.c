
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

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
    wchar_t c,
    int codigo
} Diccionario

typedef struct {
    Diccionario* d,
    unsigned int frecuencia,
    Nodo* izq,
    Nodo* der
} Nodo

//########################################################

Nodo* arbol(int n, Nodo* nodosEmparejar){

    for (int it = 0; it < n-1: ++it){

        Nodo* min1 = NULL;
        int m1 = INT_MAX;
        int pos1 = -1;
        Nodo* min2 = NULL;
        int m2 = INT_MAX;
        int pos2 = -1;
        //pos final a buscar es (n - it)
        finalPos = n - it
        for (int i = 0; i < finalPos; ++i){
            if (nodosEmparejar[i].d.frecuencia <= m1){
                min2 = min1;
                m2 = m1;
                pos2 = pos1;

                min1 = nodosEmparejar[i];
                m1 = nodosEmparejar[i].d.frecuencia;
                pos1 = i;

            } else if (nodosEmparejar[i].d.frecuencia <= m2){
                min2 = nodosEmparejar[i];
                m2 = nodosEmparejar[i].d.frecuencia;
                pos2 = i;
            }
        }

        Nodo* nodo = malloc(sizeof(Nodo));
        nodo.izq = min1;
        nodo.der = min2;
        nodo.frecuencia = m1 + m2;

        if (pos1 == finalPos){
            nodosEmparejar[pos2] = nodosEmparejar[finalPos-1];
            nodosEmparejar[finalPos-1] = nodo;

        } else if (pos2 == finalPos){
            nodosEmparejar[pos1] = nodosEmparejar[finalPos-1];
            nodosEmparejar[finalPos-1] = nodo;

        } else {
            nodosEmparejar[pos1] = nodosEmparejar[finalPos];   //ultimo en pos1
            nodosEmparejar[pos2] = nodosEmparejar[finalPos-1]; //penultimo en pos2
            nodosEmparejar[finalPos-1] = nodo;                 //inserta nodo en penultimo
        }

    }

    return nodosEmparejar[0];
}

void printArbol(Nodo* arbol){

    wprintf(L"[%lc", c);

    //izquierda
    printf("(");
    if (arbol.izq != NULL){
        printArbol(arbol.izq);
    }
    printf(")");

    //derecha
    printf("(");
    if (arbol.der != NULL){
        printArbol(arbol.der);
    }
    printf(")]");
}

//########################################################

int main(){

    setlocale(LC_ALL, "");

    int n = 4; //tamaño alfabeto
    Nodo* nodosEmparejar = malloc(sizeof(Nodo) * n);

    Diccionario* = malloc(sizeof(Diccionario) * n);

    for (int i = 0; i < n; ++i){
        Nodo[i].der = NULL;
        Nodo[i].izq = NULL;
    }

    Diccionario[0].c = "A";
    Diccionario[0].codigo = 0;
    Nodo[0].frecuencia = 24;
    Nodo[0].d = &Diccionario[0];

    Diccionario[1].c = "B";
    Diccionario[1].codigo = 0;
    Nodo[1].frecuencia = 5;
    Nodo[1].d = &Diccionario[1];

    Diccionario[2].c = "C";
    Diccionario[2].codigo = 0;
    Nodo[2].frecuencia = 12;
    Nodo[2].d = &Diccionario[2];

    Diccionario[3].c = "D";
    Diccionario[3].codigo = 0;
    Nodo[3].frecuencia = 3;
    Nodo[3].d = &Diccionario[3];
    

}