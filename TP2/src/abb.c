#include "abb.h"
#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>

#define SIN_ELEMENTOS 0
#define SIN_INVOCACIONES 0
////////////// FUNCIONES PROPIAS ////////////////////////////////////////////////////////////////////////////////////////////////

/*Pre_condiciones:
  Post_condiciones: Devuelve "true" si el arbol no existe o "false" en caso contrario.*/
bool arbol_inexistente(abb_t* arbol)
{
  if (!arbol)
  {
    return true;
  }
  
  return false;
}

/*Pre_condiciones: 
  Post_condiciones: Devuelve "true" si el nodo no existe o "false" en caso contrario.*/
bool nodo_inexistente(nodo_abb_t* nodo)
{
  if (!nodo)
  {
    return true;
  }

  return false;
}

/*Pre_condiciones: 
  Post_condiciones: Devuelve un puntero a un nodo que almacena el elemento pasado por parametro. En caso de error
  se devuelve NULL.*/
nodo_abb_t* crear_nodo(void* elemento)
{
    nodo_abb_t* nuevo_nodo = calloc(1, sizeof(nodo_abb_t));

    if (nodo_inexistente(nuevo_nodo))
    {
      return NULL;
    }
    
    nuevo_nodo->elemento = elemento;

    return nuevo_nodo;
}

/*Pre_condiciones: 
  Post_condiciones: Libera la memoria reservada por los nodos que conforman un arbol y le aplica a cada elemento
   un "destructor" pasado por parametro en caso de que este no sea nulo.*/
void destruir_nodos(nodo_abb_t* nodo, void destructor(void* elemento))
{
    if (!nodo_inexistente(nodo))
    {
        destruir_nodos(nodo->izquierda, destructor);
        destruir_nodos(nodo->derecha, destructor);
        
        if (destructor != NULL)
        {
          destructor(nodo->elemento);         
        }

        free(nodo);
    }
}

/*Pre_condiciones: 
  Post_condiciones: Inserta en un arbol un elemento pasado por parametro, teniendo en cuenta el orden dado por el 
   comparador pasado por parametro.*/
nodo_abb_t* insertar_nodo(nodo_abb_t* nodo, void* elemento, abb_comparador comparador)
{
    if (nodo_inexistente(nodo))
    {
      nodo = crear_nodo(elemento);
    }
    else if (comparador(nodo->elemento, elemento) > 0)
    {
      nodo->izquierda = insertar_nodo(nodo->izquierda, elemento, comparador);
    }
    else if (comparador(nodo->elemento, elemento) < 0)
    {
      nodo->derecha = insertar_nodo(nodo->derecha, elemento, comparador);
    }
    else
    {
      nodo_abb_t* nuevo_nodo = crear_nodo(elemento);

      nuevo_nodo->izquierda = nodo->izquierda;

      nodo->izquierda = nuevo_nodo;
    }

    return nodo;
}

/*Pre_condiciones: 
  Post_condiciones: Devuelve un puntero al elemento buscado alojado en el arbol, en caso de no existir se
   devuelve un puntero a NULL.*/
void* buscar_elemento(nodo_abb_t* nodo, void* elemento, abb_comparador comparador)
{
    if (nodo == NULL)
    {
        return NULL;
    }

    int comparacion = comparador(nodo->elemento, elemento);
    void* elemento_buscado = nodo->elemento;

    if (comparacion > 0)
    {
        elemento_buscado = buscar_elemento(nodo->izquierda, elemento, comparador);
    }
    else if (comparacion < 0)
    {
        elemento_buscado = buscar_elemento(nodo->derecha, elemento, comparador);
    }
    
    return elemento_buscado;
}

/*Pre_condiciones: 
  Post_condiciones: Devuelve un puntero a el predecesor inorden del nodo pasado por parametro.*/
nodo_abb_t* extraer_predecesor_inorder(nodo_abb_t* nodo)
{
   if (nodo->derecha)
   {
     nodo = extraer_predecesor_inorder(nodo->derecha);
   }
   return nodo;
}

/*Pre_condiciones: 
  Post_condiciones: Devuelve "true" si el elemento del nodo y el segundo elemento 
   pasado por parametro son iguales.*/
bool elemento_repetido(nodo_abb_t*  nodo, void* elemento, abb_comparador comparador)
{
  if (!nodo->izquierda)
  {
    return false;
  }
  else if (comparador(nodo->izquierda->elemento, elemento) != 0)
  {
    return false;
  }
  return true;
}

/*Pre_condiciones: 
  Post_condiciones: Elimina del arbol un elemento pasado por parametro y lo almacena en el puntero "elemento_quitado".*/
nodo_abb_t* eliminar_elemento(nodo_abb_t* nodo, void* elemento, abb_comparador comparador, void** elemento_quitado)
{
    nodo_abb_t * hijo = NULL;
    int comparacion = comparador(nodo->elemento, elemento);
    if (!nodo) 
    {
        *elemento_quitado = NULL;
        return NULL;
    }
    if ((comparacion == 0) && (!elemento_repetido(nodo, elemento, comparador)))
    {
      if (nodo->derecha && nodo->izquierda) //CASO DOS HIJOS
      {
          nodo_abb_t* predecesor = extraer_predecesor_inorder(nodo->izquierda);
          void* elemento_predecesor = predecesor->elemento;
          eliminar_elemento(nodo, predecesor->elemento, comparador, elemento_quitado);
          *elemento_quitado = nodo->elemento;
          nodo->elemento = elemento_predecesor;
          return nodo;
      }
      else if (nodo->derecha || nodo->izquierda) //Caso un Hijo
      {
          hijo = nodo->derecha?nodo->derecha:nodo->izquierda;
          *elemento_quitado = nodo->elemento;
          free(nodo);
          return hijo;
      }
      else //Caso sin hijos
      {
          *elemento_quitado = nodo->elemento;
          free(nodo);
          return NULL;
      }  
    }
    else if ((comparacion > 0) || (elemento_repetido(nodo, elemento, comparador)))
    {
      nodo->izquierda = eliminar_elemento(nodo->izquierda, elemento, comparador, elemento_quitado);
    }
    else
    {
      nodo->derecha = eliminar_elemento(nodo->derecha, elemento, comparador, elemento_quitado);
    }

    return nodo;
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
abb_t* abb_crear(abb_comparador comparador)
{
  if (!comparador)
  {
    return NULL;
  }
  
  abb_t* nuevo_arbol = calloc(1, sizeof(abb_t));

  if (arbol_inexistente(nuevo_arbol))
  {
    return NULL;
  }

  nuevo_arbol->comparador = comparador;

  return nuevo_arbol;
}

abb_t* abb_insertar(abb_t* arbol, void* elemento)
{
  if (arbol_inexistente(arbol))
  {
    return NULL;
  }
  else if (arbol->comparador == NULL)
  {
    return NULL;
  }

  arbol->nodo_raiz = insertar_nodo(arbol->nodo_raiz, elemento, arbol->comparador);

  arbol->tamanio++;
  return arbol;
}

void* abb_quitar(abb_t* arbol, void *elemento)
{
  if (abb_vacio(arbol))
  {
    return NULL;
  }

  void* elemento_quitado;
  
  arbol->nodo_raiz = eliminar_elemento(arbol->nodo_raiz, elemento, arbol->comparador, &elemento_quitado);

  arbol->tamanio --;
  return elemento_quitado;
}

void* abb_buscar(abb_t* arbol, void* elemento)
{
  if (arbol_inexistente(arbol))
  {
    return NULL;
  }
  else if (arbol->comparador == NULL)
  {
    return NULL;
  }
  
  nodo_abb_t* nodo_buscado = buscar_elemento(arbol->nodo_raiz, elemento, arbol->comparador);
  
  return nodo_buscado;
}

bool abb_vacio(abb_t* arbol)
{
  if(arbol_inexistente(arbol))
  {
    return true;
  } 
  else if ((arbol->tamanio == SIN_ELEMENTOS) && (nodo_inexistente(arbol->nodo_raiz))) //Se consultan existencia y tamaño para verificar que ambos valores fueron actualizados.
  {  
    return true;
  }
  else
  {
      return false;
  }
}

size_t abb_tamanio(abb_t *arbol)
{
  if (arbol_inexistente(arbol))
  {
    return SIN_ELEMENTOS;
  }
  
  return arbol->tamanio;
}

void abb_destruir(abb_t *arbol)
{
  abb_destruir_todo(arbol, NULL);
}

void abb_destruir_todo(abb_t *arbol, void (*destructor)(void *))
{
  if (!arbol_inexistente(arbol))
    {
        destruir_nodos(arbol->nodo_raiz, destructor);
        free(arbol);
    }
}

/*Pre_condiciones: El puntero al nodo y el puntero a la función no deben ser NULL. "resultado" debe apuntar a un bool en estado "true".
  Post_condiciones: Recorre el arbol con recorrido inorden e invoca la funcion con cada elemento almacenado en el mismo
  como primer parámetro. El puntero aux se pasa como segundo parámetro a la función. Si la función devuelve false, se
  finaliza el recorrido aun si quedan elementos por recorrer. Si devuelve true se sigue recorriendo mientras queden
  elementos. Devuelve la cantidad de veces que fue invocada la función.*/
size_t abb_con_cada_elemento_inorden(nodo_abb_t* nodo, bool (*funcion)(void *, void *), void *aux, bool *resultado)
{
  size_t contador = SIN_INVOCACIONES;

  if (!nodo)
  {
    return contador;
  }

  if (*resultado)
  {
    contador += abb_con_cada_elemento_inorden(nodo->izquierda, funcion, aux, resultado);
  }
  if (*resultado)
  {
    *resultado = funcion(nodo->elemento, aux);
    contador ++;
  }
  if (*resultado)
  {
    contador += abb_con_cada_elemento_inorden(nodo->derecha, funcion, aux, resultado);
  }
  
  return contador;
}

/*Pre_condiciones: El puntero al nodo y el puntero a la función no deben ser NULL. "resultado" debe apuntar a un bool en estado "true".
  Post_condiciones: Recorre el arbol con recorrido preorden e invoca la funcion con cada elemento almacenado en el mismo
  como primer parámetro. El puntero aux se pasa como segundo parámetro a la función. Si la función devuelve false, se
  finaliza el recorrido aun si quedan elementos por recorrer. Si devuelve true se sigue recorriendo mientras queden
  elementos. Devuelve la cantidad de veces que fue invocada la función.*/
size_t abb_con_cada_elemento_preorden(nodo_abb_t* nodo, bool (*funcion)(void *, void *), void *aux, bool *resultado)
{
  size_t contador = SIN_INVOCACIONES;

  if (!nodo)
  {
    return contador;
  }

  if (*resultado)
  {
    *resultado = funcion(nodo->elemento, aux);
    contador ++;
  }
  if (*resultado)
  {
    contador += abb_con_cada_elemento_preorden(nodo->izquierda, funcion, aux, resultado);
  }
  if (*resultado)
  {
    contador += abb_con_cada_elemento_preorden(nodo->derecha, funcion, aux, resultado);
  }
  
  return contador;
}

/*Pre_condiciones: El puntero al nodo y el puntero a la función no deben ser NULL. "resultado" debe apuntar a un bool en estado "true".
  Post_condiciones: Recorre el arbol con recorrido postorden e invoca la funcion con cada elemento almacenado en el mismo
  como primer parámetro. El puntero aux se pasa como segundo parámetro a la función. Si la función devuelve false, se 
  finaliza el recorrido aun si quedan elementos por recorrer. Si devuelve true se sigue recorriendo mientras queden
  elementos. Devuelve la cantidad de veces que fue invocada la función.*/
size_t abb_con_cada_elemento_postorden(nodo_abb_t* nodo, bool (*funcion)(void *, void *), void *aux, bool *resultado)
{
  size_t contador = SIN_INVOCACIONES;

  if (!nodo)
  {
    return contador; 
  }

  if (*resultado)
  {
    contador += abb_con_cada_elemento_postorden(nodo->izquierda, funcion, aux, resultado);
  }
  if (*resultado)
  {
    contador += abb_con_cada_elemento_postorden(nodo->derecha, funcion, aux, resultado);
  }
  if (*resultado)
  {
    *resultado = funcion(nodo->elemento, aux);
    contador ++;
  }
  
  return contador;
}

size_t abb_con_cada_elemento(abb_t *arbol, abb_recorrido recorrido, bool (*funcion)(void *, void *), void *aux)
{
  if ((arbol_inexistente(arbol)) || (!funcion))
  {
    return SIN_INVOCACIONES;
  }

  bool resultado = true;

  if (recorrido == INORDEN)
  {
    return abb_con_cada_elemento_inorden(arbol->nodo_raiz, funcion, aux, &resultado);
  }
  else if (recorrido == PREORDEN)
  {
    return abb_con_cada_elemento_preorden(arbol->nodo_raiz, funcion, aux, &resultado);
  }
  else if (recorrido == POSTORDEN)
  {
    return abb_con_cada_elemento_postorden(arbol->nodo_raiz, funcion, aux, &resultado);
  }
  
  return SIN_INVOCACIONES;
}

/*Pre_condiciones: El puntero al nodo y el puntero a la función no deben ser NULL. "elementos_almacenados" debe apuntar a un entero inicializado en 0.
  Post_condiciones: Recorre el arbol con recorrido inorden y va almacenando los elementos en el vector pasado por 
  parametro hasta completar el recorrido o quedarse sin espacio en el mismo.*/
void abb_recorrer_inorden(nodo_abb_t* nodo, abb_recorrido recorrido, void** array, size_t tamanio_array, size_t *elementos_almacenados)
{
  if (!nodo)
  {
    return;
  }

  if (*elementos_almacenados < tamanio_array)
  {
    abb_recorrer_inorden( nodo->izquierda, recorrido, array, tamanio_array, elementos_almacenados);
  }
  if (*elementos_almacenados < tamanio_array)
  {
    array[*elementos_almacenados] = nodo->elemento;
    *elementos_almacenados += 1;
  }
  if (*elementos_almacenados < tamanio_array)
  {
    abb_recorrer_inorden( nodo->derecha, recorrido, array, tamanio_array, elementos_almacenados);
  }
}

/*Pre_condiciones: El puntero al nodo y el puntero a la función no deben ser NULL. "elementos_almacenados" debe apuntar a un entero inicializado en 0.
  Post_condiciones: Recorre el arbol con recorrido preorden y va almacenando los elementos en el vector pasado por 
  parametro hasta completar el recorrido o quedarse sin espacio en el mismo.*/
void abb_recorrer_preorden(nodo_abb_t* nodo, abb_recorrido recorrido, void** array, size_t tamanio_array, size_t *elementos_almacenados)
{
  if (!nodo)
  {
    return;
  }

  if (*elementos_almacenados < tamanio_array)
  {
    array[*elementos_almacenados] = nodo->elemento;
    *elementos_almacenados += 1;
  }
  if (*elementos_almacenados < tamanio_array)
  {
    abb_recorrer_preorden( nodo->izquierda, recorrido, array, tamanio_array, elementos_almacenados);
  }
  if (*elementos_almacenados < tamanio_array)
  {
    abb_recorrer_preorden( nodo->derecha, recorrido, array, tamanio_array, elementos_almacenados);
  }
}

/*Pre_condiciones: El puntero al nodo y el puntero a la función no deben ser NULL. "elementos_almacenados" debe apuntar a un entero inicializado en 0.
  Post_condiciones: Recorre el arbol con recorrido postorden y va almacenando los elementos en el vector pasado por 
  parametro hasta completar el recorrido o quedarse sin espacio en el mismo.*/
void abb_recorrer_postorden(nodo_abb_t* nodo, abb_recorrido recorrido, void** array, size_t tamanio_array, size_t *elementos_almacenados)
{
  if (!nodo)
  {
    return;
  }

  if (*elementos_almacenados < tamanio_array)
  {
   abb_recorrer_postorden( nodo->izquierda, recorrido, array, tamanio_array, elementos_almacenados);
  }
  if (*elementos_almacenados < tamanio_array)
  {
   abb_recorrer_postorden( nodo->derecha, recorrido, array, tamanio_array, elementos_almacenados);
  }
  if (*elementos_almacenados < tamanio_array)
  {
    array[*elementos_almacenados] = nodo->elemento;
    *elementos_almacenados += 1;
  }
}

size_t abb_recorrer(abb_t* arbol, abb_recorrido recorrido, void** array, size_t tamanio_array)
{
  size_t elementos_almacenados = SIN_ELEMENTOS;

  if ((arbol_inexistente(arbol)) || (!array))
  {
    return elementos_almacenados;
  }

  if (recorrido == INORDEN)
  {
    abb_recorrer_inorden(arbol->nodo_raiz, recorrido, array, tamanio_array, &elementos_almacenados);
  }
  else if (recorrido == PREORDEN)
  {
    abb_recorrer_preorden(arbol->nodo_raiz, recorrido, array, tamanio_array, &elementos_almacenados);
  }
  else if (recorrido == POSTORDEN)
  {
    abb_recorrer_postorden(arbol->nodo_raiz, recorrido, array, tamanio_array, &elementos_almacenados);
  }
  
  return elementos_almacenados;
}
