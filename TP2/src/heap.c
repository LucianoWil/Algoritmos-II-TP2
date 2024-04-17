#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include "heap.h"

#define PRIMER_ELEMENTO 0
#define PUNTO_REALLOC 0.8

typedef struct heap
{
    void** vector;
    int tope;
    size_t capacidad;
    heap_comparador comparador;
}heap_t;

/// Funciones propias ////////////////////////////////////////////////////////////////////////////////////////////////
/*Pre_condiciones:
  Post_condiciones: Devuelve la posición del padre de un elemento dependiendo de su posición.*/
int pos_padre(int pos_actual)
{
    return (pos_actual - 1)/2;
}

/*Pre_condiciones: El heap no debe ser nulo 
  Post_condiciones: Ordena el heap verificando si el elemento en la ultima posición está en el lugar indicado, de no 
  ser así se cambia la posición del mismo hasta que se ordene.*/
void sift_up(heap_t* heap)
{
    if(!heap)
        return;

    void* aux = NULL;
    bool ordenado = false;
    int pos_actual = heap->tope - 1;
    int padre = pos_padre(pos_actual);

    while (!ordenado)
    {
        if(heap->comparador(heap->vector[padre], heap->vector[pos_actual]) > 0)
        {
            aux = heap->vector[padre];
            heap->vector[padre] = heap->vector[pos_actual];
            heap->vector[pos_actual] = aux;

            pos_actual = padre;
            padre = pos_padre(pos_actual);
        }
        else
            ordenado = true;
    }
}

/*Pre_condiciones:
  Post_condiciones: Devuelve la posición del elemento en la rama izquierda de un elemento dependiendo de su posición.*/
int pos_hijo_izq(int pos_actual)
{
    return ((2 * pos_actual) + 1);
}

/*Pre_condiciones:
  Post_condiciones: Devuelve la posición del elemento en la rama derecha de un elemento dependiendo de su posición.*/
int pos_hijo_der(int pos_actual)
{
    return ((2 * pos_actual) + 2);
}

/*Pre_condiciones:
  Post_condiciones: Ordena el heap verificando si el elemento en la primera posición está en el lugar indicado, de no 
  ser así se cambia la posición del mismo hasta que se ordene.*/
void sift_down(heap_t* heap)
{
    int pos_actual = PRIMER_ELEMENTO;
    bool ordenado = false;
    void* aux = NULL;
    int hijo_izq = pos_hijo_izq(pos_actual);
    int hijo_der = pos_hijo_der(pos_actual);

    if ((heap->tope - 1) < hijo_izq)
        return; //no tiene hijos

    int hijo_mayor = (heap->comparador(heap->vector[hijo_izq], heap->vector[hijo_der]) > 0) ? hijo_der : hijo_izq;    
    
    while (!ordenado)
    {
        if (heap->comparador(heap->vector[hijo_mayor], heap->vector[pos_actual]) < 0)
        {
            aux = heap->vector[hijo_mayor];
            heap->vector[hijo_mayor] = heap->vector[pos_actual];
            heap->vector[pos_actual] = aux;
            pos_actual = hijo_mayor;
            hijo_izq = pos_hijo_izq(pos_actual);
            hijo_der = pos_hijo_der(pos_actual);
            if (hijo_der <= heap->tope - 1)
            {
                hijo_mayor = (heap->comparador(heap->vector[hijo_izq], heap->vector[hijo_der]) > 0) ? hijo_der : hijo_izq;
                if ((heap->tope - 1) < hijo_izq)
                    ordenado = true;
            }
            else if (hijo_izq <= heap->tope - 1)
                hijo_mayor = hijo_izq;
            else
                ordenado = true;
        }
        else
            ordenado = true;
    }
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
heap_t* heap_crear(heap_comparador comparador, size_t capacidad_inicial)
{
    if ((!comparador) || (capacidad_inicial == 0))
    {
        printf("\nError al crear el heap\n");
        return NULL;
    }

    heap_t* heap = malloc(sizeof(heap_t));

    heap->comparador = comparador;
    heap->vector = calloc(capacidad_inicial, sizeof(void*));
    heap->tope = 0;
    heap->capacidad = capacidad_inicial;

    return heap;
}

void heap_insertar(heap_t* heap, void* elemento)
{
    if(!heap)
        return;

    heap->vector[heap->tope] = elemento;
    heap->tope++;
    if(heap->tope > 1) //Para que no se analice el primer caso 
        sift_up(heap);

    if ((heap->tope / (int)heap->capacidad) >= PUNTO_REALLOC)
    {
        heap->capacidad *= 2;
        heap->vector =  realloc(heap->vector, heap->capacidad * sizeof(void*));
    }
}

void* heap_extraer_raiz(heap_t* heap)
{
    if((!heap) || (heap->tope == 0))
        return NULL;

    void* aux = heap->vector[0];
    heap->vector[0] = heap->vector[heap->tope - 1];
    heap->tope--;
    sift_down(heap);

    return aux;
}

void heap_destruir(heap_t* heap)
{
    if(!heap)
        return;
    free(heap->vector);
    free(heap);
}

/*
size_t heap_a_cada_pokemon(heap_t* heap, bool (*funcion)(pokemon_t* pokemon))
{
    bool resultado = true;
    size_t contador = 0;

    if((!heap) || (!funcion))
        return contador;
        
    while ((contador < heap->tope) && (resultado))
    {
        resultado = funcion((pokemon_t*)heap_extraer_raiz(heap));
        contador++;
    }
    
    return contador;
}*/

void heap_a_cada_elemento(heap_t* heap, void (*funcion)(void* elemento))
{
    size_t contador = 0;

    if((!heap) || (!funcion))
        return;

    while (contador < heap->tope)
    {
        funcion(heap->vector[contador]);
        contador++;
    }
}

size_t heap_cantidad(heap_t* heap)
{
    if (!heap)
        return 0;

    return (size_t)heap->tope;
}

void* heap_obtener_posicion(heap_t* heap, int posicion)
{
    if(!heap)
        return NULL;

    if(heap->vector[posicion])
        return heap->vector[posicion];
    else
        return NULL;
}
/*
int main()
{

     int ejemplo_comparador(void* elemento_1, void* elemento_2)
{
    int primer_elemento = *(int*)elemento_1;
    int segundo_elemento = *(int*)elemento_2;
    return segundo_elemento - primer_elemento;
}
    //Pruebas heap
    heap_t* heap = heap_crear(ejemplo_comparador, 5);

    int numero_1 = 1;
    int numero_2 = 2;
    int numero_3 = 3;
    int numero_4 = 4;
    int numero_5 = 5;
    int numero_6 = 6;
    int numero_7 = 7;
    int numero_8 = 8;
    int numero_9 = 9;
    int numero_10 = 10;

    int* elemento_1 = calloc(1, sizeof(int));
    int* elemento_2 = calloc(1, sizeof(int));
    int* elemento_3 = calloc(1, sizeof(int));
    int* elemento_4 = calloc(1, sizeof(int));
    int* elemento_5 = calloc(1, sizeof(int));
    int* elemento_6 = calloc(1, sizeof(int));
    int* elemento_7 = calloc(1, sizeof(int));
    int* elemento_8 = calloc(1, sizeof(int));
    int* elemento_9 = calloc(1, sizeof(int));
    int* elemento_10 = calloc(1, sizeof(int));

    *elemento_1 = numero_1;
    *elemento_2 = numero_2;
    *elemento_3 = numero_3;
    *elemento_4 = numero_4;
    *elemento_5 = numero_5;
    *elemento_6 = numero_6;
    *elemento_7 = numero_7;
    *elemento_8 = numero_8;
    *elemento_9 = numero_9;
    *elemento_10 = numero_10;

    heap_insertar(heap, elemento_1);
    heap_insertar(heap, elemento_2);
    heap_insertar(heap, elemento_3);
    heap_insertar(heap, elemento_4);
    heap_insertar(heap, elemento_5);
    heap_insertar(heap, elemento_6);
    heap_insertar(heap, elemento_7);
    heap_insertar(heap, elemento_8);
    heap_insertar(heap, elemento_9);
    heap_insertar(heap, elemento_10);

    void* raiz_extraida = heap_extraer_raiz(heap);
    void* raiz_extraida_2 = heap_extraer_raiz(heap);

    for (size_t i = 0; i < heap->tope; i++)
    {
        printf(" %i ,", *(int*)heap->vector[i]);
    }   
 
    heap_destruir(heap);
    free(raiz_extraida);
    free(raiz_extraida_2);
    return 0; 
}
*/