#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include "lista.h"

#define POSICION_INICIAL 0
#define ERROR 0
#define SIN_ELEMENTOS 0 
#define VACIA false


/////Funciones propias//////////////////////////////////////////////////////////////////////
/*Pre_condiciones:
  Post_condiciones: Devuelve true si la lista es inexistente o false en caso contrario.*/
bool lista_inexistente(lista_t* lista)
{   
    if (!lista)
    {
        return true;
    }
    return false;
}

/*Pre_condiciones: El nodo pasado por parametro debe ser no nulo.
  Post_condiciones: Devuelve el nodo en la posición previa a la pasada por parametro.*/
nodo_t* recorrer_lista_hasta_posicion_previa(nodo_t* nodo,size_t posicion)
{
    if (!nodo)
    {
        printf("Error en 'recorrer_lista_hasta_posicion_previa': el nodo pasado por parametro debe ser no nulo.");
    }
    
    if (posicion == POSICION_INICIAL)
    {
        return nodo;
    }

    for (size_t i = POSICION_INICIAL; i < posicion - 1; i++)
        {
            nodo = nodo->siguiente;
        }

    return nodo;
}
////////////////////////////////////////////////////////////////////////////////////////////
lista_t* lista_crear()
{
    lista_t* nueva_lista = calloc(1,sizeof(lista_t));

    if (!nueva_lista)
    {
        printf("Error al asignar memoria a la nueva lista en 'lista_crear'.");
        return NULL;
    }
    
    return nueva_lista;
}

lista_t* lista_insertar(lista_t* lista, void* elemento)
{
    if(lista_inexistente(lista))
    {
        return NULL;
    }

    nodo_t* nuevo_nodo = calloc(1, sizeof(nodo_t));

    if (!nuevo_nodo)
    {
        printf("Error al asignar memoria para un nuevo nodo en 'lista_insertar'.");
        return NULL;
    }

    if(lista->cantidad == SIN_ELEMENTOS)
    {
        nuevo_nodo->elemento = elemento;
        lista->nodo_inicio = nuevo_nodo;
        lista->nodo_fin = nuevo_nodo;
    }
    else
    {
        nuevo_nodo->elemento = elemento;
        lista->nodo_fin->siguiente = nuevo_nodo;
        lista->nodo_fin = nuevo_nodo;
    }

    lista->cantidad ++;
    return lista;
}

lista_t* lista_insertar_en_posicion(lista_t* lista, void* elemento, size_t posicion)
{
    if (lista_inexistente(lista)) 
    {
        return NULL;
    }
    
    nodo_t* nuevo_nodo = calloc(1, sizeof(nodo_t));

    if (!nuevo_nodo)
    {
        printf("Error al asignar memoria para un nuevo nodo en 'lista_insertar_en_posicion'.");
        return NULL;
    }

    nuevo_nodo->elemento = elemento;
    nodo_t* nodo_aux = NULL;

    if((posicion < lista->cantidad) && (posicion != POSICION_INICIAL))//Por si se inserta al medio de la lista
    {    
        nodo_aux = lista->nodo_inicio;
        nodo_aux = recorrer_lista_hasta_posicion_previa(nodo_aux, posicion);  
        nuevo_nodo->siguiente = nodo_aux->siguiente;
        nodo_aux->siguiente = nuevo_nodo;
    }
    else if ((posicion == POSICION_INICIAL) && (lista->cantidad > SIN_ELEMENTOS)) //Por si se inserta al inicio de la lista y esta no es vacia 
    {
        nuevo_nodo->siguiente = lista->nodo_inicio;
        lista->nodo_inicio = nuevo_nodo;        
    }
    else if (lista->cantidad == SIN_ELEMENTOS)//Por si la lista es vacia 
    {
        lista->nodo_inicio = nuevo_nodo;
        lista->nodo_fin = nuevo_nodo;
    }
    else
    {
        lista->nodo_fin->siguiente = nuevo_nodo;
        lista->nodo_fin = nuevo_nodo;
    }
    
    lista->cantidad++;
    return lista;
}

void* lista_quitar(lista_t* lista)
{
    if (lista_vacia(lista))
    {
        return NULL;
    }

    void* elemento_eliminado = lista->nodo_fin->elemento;
    nodo_t* nodo_aux = lista->nodo_inicio;

    free(lista->nodo_fin);
    lista->cantidad --;

    if(lista->cantidad > SIN_ELEMENTOS)
    {
        for (size_t i = SIN_ELEMENTOS; i < lista->cantidad - 1; i++)
        {   
            nodo_aux = nodo_aux->siguiente;
        }  
        nodo_aux->siguiente = NULL;
        lista->nodo_fin = nodo_aux;
    }
    else
    {
        lista->nodo_fin = NULL;
        lista->nodo_inicio = NULL;
    }

    return elemento_eliminado;
}

void* lista_quitar_de_posicion(lista_t* lista, size_t posicion)
{
    if (lista_vacia(lista))
    {
        return NULL;
    }

    void* elemento_eliminado = NULL;
    nodo_t* ptr_nodo_a_eliminar = NULL;
    nodo_t* ptr_nodo_previo_al_eliminado = lista->nodo_inicio;
    nodo_t* ptr_nodo_posterior_al_eliminado = NULL;
    lista->cantidad--;

    if(lista->cantidad > SIN_ELEMENTOS)//Si la lista posee mas de un elemento
    {
        if((posicion < lista->cantidad) && (posicion != POSICION_INICIAL))//Si el elemento se posiciona en el medio
        {
            ptr_nodo_previo_al_eliminado = recorrer_lista_hasta_posicion_previa(ptr_nodo_previo_al_eliminado, posicion);
            ptr_nodo_a_eliminar = ptr_nodo_previo_al_eliminado->siguiente;
            ptr_nodo_posterior_al_eliminado = ptr_nodo_a_eliminar->siguiente;
            elemento_eliminado = ptr_nodo_a_eliminar->elemento;
            free(ptr_nodo_a_eliminar);
            ptr_nodo_previo_al_eliminado->siguiente = ptr_nodo_posterior_al_eliminado;
        }
        else if (posicion == POSICION_INICIAL)//Si el elemento se posiciona en el inicio
        {
            ptr_nodo_posterior_al_eliminado = lista->nodo_inicio->siguiente;
            ptr_nodo_a_eliminar = lista->nodo_inicio;
            elemento_eliminado = ptr_nodo_a_eliminar->elemento;
            lista->nodo_inicio = ptr_nodo_posterior_al_eliminado;
            free(ptr_nodo_a_eliminar);
        }
        else//Si el elemento se posiciona en el final
        {
            ptr_nodo_previo_al_eliminado = recorrer_lista_hasta_posicion_previa(ptr_nodo_previo_al_eliminado, lista->cantidad);
            ptr_nodo_a_eliminar = ptr_nodo_previo_al_eliminado->siguiente;
            elemento_eliminado = ptr_nodo_a_eliminar->elemento;
            free(ptr_nodo_a_eliminar);
            ptr_nodo_previo_al_eliminado->siguiente = NULL;
            lista->nodo_fin = ptr_nodo_previo_al_eliminado;
        } 
    }
    else //Si la lista posee solo un elemento
    {
        elemento_eliminado = lista->nodo_inicio->elemento;
        free(lista->nodo_inicio);
        lista->nodo_inicio = NULL;
        lista->nodo_fin = NULL;
    }
    
    return elemento_eliminado;
}

void* lista_elemento_en_posicion(lista_t* lista, size_t posicion)
{
    if ((lista_vacia(lista)) || (posicion >= lista->cantidad))
    {
        return NULL;
    }
    
    nodo_t* nodo_aux = lista->nodo_inicio;

    for (size_t i = POSICION_INICIAL; i < posicion; i++)
    {
        nodo_aux = nodo_aux->siguiente;
    }
    
    return nodo_aux->elemento;
}

void* lista_primero(lista_t* lista)
{
    if (lista_vacia(lista))
    {
        return NULL;
    }
    
    return lista->nodo_inicio->elemento;
}

void* lista_ultimo(lista_t* lista)
{
    if (lista_vacia(lista))
    {
        return NULL;
    }
    
    return lista->nodo_fin->elemento;
}

bool lista_vacia(lista_t* lista)
{
    if ((lista_inexistente(lista)) || ((lista->cantidad == SIN_ELEMENTOS) && (lista->nodo_inicio == NULL))) //Para la lista vacia compruebo los dos parametros por si en alguna funcion se asigna mal un valor poder detectarlo.
    {
        return true;
    }

    return false;
}

size_t lista_tamanio(lista_t* lista)
{
    if (lista_inexistente(lista))
    {
        return ERROR;
    }

    return lista->cantidad;
}

void lista_destruir(lista_t* lista)
{
    if(lista != NULL)
    {
        nodo_t* ptr_aux_nodo = NULL; 

        for (size_t i = SIN_ELEMENTOS; i < lista->cantidad; i++)
        {
            ptr_aux_nodo = lista->nodo_inicio;
            lista->nodo_inicio = lista->nodo_inicio->siguiente;
            free(ptr_aux_nodo);
        }
        
        free(lista);
    }
}

lista_iterador_t* lista_iterador_crear(lista_t* lista)
{
    if (lista_inexistente(lista))
    {
        return NULL;
    }

    lista_iterador_t* nuevo_iterador_lista = calloc(1, sizeof(lista_iterador_t));

    if (!nuevo_iterador_lista)
    {
        printf("Error: Al asignar memoria para una nueva lista iterador en 'lista_iterador_crear'.");
        return NULL;
    }

    if (!nuevo_iterador_lista)
    {
        return NULL;
    }

    nuevo_iterador_lista->lista = lista;
    nuevo_iterador_lista->corriente = lista->nodo_inicio;

    return nuevo_iterador_lista;
}

bool lista_iterador_tiene_siguiente(lista_iterador_t* iterador)
{
    if ((!iterador) || (!iterador->corriente))
    {
        return false;
    }
    else if (!iterador->corriente->siguiente)
    {
        return true;
    }  
    
    return true;
}

bool lista_iterador_avanzar(lista_iterador_t* iterador)
{
    if (!iterador)
    {
        return false;
    }
   
    else if (iterador->corriente == iterador->lista->nodo_fin)
    {
        iterador->corriente = NULL;
        return false;
    }
    
    iterador->corriente = iterador->corriente->siguiente;
    return true;
}

void* lista_iterador_elemento_actual(lista_iterador_t* iterador)
{
    if ((!iterador) || (!iterador->corriente))
    {
        return NULL;
    }
    
    return iterador->corriente->elemento;
}

void lista_iterador_destruir(lista_iterador_t* iterador)
{
    if(iterador != NULL)
    {
        free(iterador);
    }
}

size_t lista_con_cada_elemento(lista_t* lista, bool (*funcion)(void*, void*), void *contexto)
{
    if (!funcion)
    {
        return ERROR;
    }

    size_t iteraciones = 0;
    bool resultado_funcion = true;
    nodo_t* nodo_aux = NULL;

    if (lista_vacia(lista))
    {
        return iteraciones;
    }
    
    nodo_aux = lista->nodo_inicio;

    while((resultado_funcion) && (iteraciones < lista->cantidad))
    {
        resultado_funcion = funcion(nodo_aux->elemento, contexto);
        iteraciones ++;
        nodo_aux = nodo_aux->siguiente;
    }
    
    return iteraciones;
}
