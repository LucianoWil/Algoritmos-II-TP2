#ifndef __HEAP__H__
#define __HEAP__H__

#include <stdbool.h>
#include <stdlib.h>
#include "hospital.h"

/**
 * Comparador de elementos. Recibe dos elementos y devuelve 0 en caso de ser
 * iguales, >0 si el segundo elemento es mayor al primero o <0 si el segundo
 * elemento es menor al primero.
 */
typedef int (*heap_comparador)(void*, void*);

typedef struct heap heap_t;

/*Pre_condiciones: El comparador no debe ser nulo y la capacidad inicial debe ser mayor a cero.
  Post_condiciones: Crea un heap con el comparador y la capacidad inicial pasada por parámetro.
                    En caso de error devuelve NULL*/
heap_t* heap_crear(heap_comparador comparador, size_t capacidad_inicial);

/*Pre_condiciones: El heap pasado por parametro no debe ser nulo.
  Post_condiciones: Ingresa un elemento en el heap y realiza un ordenamiento en caso de ser necesario*/
void heap_insertar(heap_t* heap, void* elemento);

/*Pre_condiciones: El heap pasado por parametro no debe ser nulo.
  Post_condiciones: Elimina la raiz del heap y la devuelve, realiza un reordenamiento en caso de ser necesario.
                    Devuelve NULL en caso de error.*/
void* heap_extraer_raiz(heap_t* heap);

/*Pre_condiciones: El heap pasado por parametro no debe ser nulo.
  Post_condiciones: Libera la memoria reservada por el heap pasado por parámetro.*/
void heap_destruir(heap_t* heap);

/*Pre_condiciones: El heap pasado por parametro no debe ser nulo.
  Post_condiciones: Devuelve la cantidad de elementos almacenados en el heap.*/
size_t heap_cantidad(heap_t* heap);

/*Pre_condiciones: El heap pasado por parametro no debe ser nulo.
  Post_condiciones: Devuelve el elemento en la posición especificada. En caso de que la posición no exista o de un
  error se devuelve NULL.*/
void* heap_obtener_posicion(heap_t* heap, int posicion);

/*Pre_condiciones: El heap y la función pasados por parametro no deben ser nulos.
  Post_condiciones: Aplica una función a cada elemento del heap.*/
void heap_a_cada_elemento(heap_t* heap, void (*funcion)(void* elemento));

#endif /* __HEAP__H__ */