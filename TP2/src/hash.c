#include "hash.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define INEXISTENTE -1
#define CON_ELEMENTOS 1
#define VACIO 0
#define EXITO 0
#define ERROR -1
#define VERDADERO 1
#define FALSO 0
#define CAPACIDAD_MINIMA 3
#define LIMITE_REHASH 0.75

typedef struct casillero casillero_t;

struct casillero
{
    char* clave;
    void* elemento;
    casillero_t* siguiente;
};

struct hash
{
    casillero_t** casilleros;
    size_t cantidad_casilleros;
    float elementos_almacenados;
    hash_destruir_dato_t funcion_destructora;
};

////Funciones propias//////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*Pre_condicones: 
  Post_condiciones: Devuelve '-1'  si el hash es inexistente, '0' si el hash se encuentra vacío y '1' si 
    el hash posee elementos*/
int hash_inexistente_vacio(hash_t* hash)
{
    if (!hash)
        return INEXISTENTE;
    else if (hash->elementos_almacenados == VACIO)
        return VACIO;
    else
        return CON_ELEMENTOS;
}

/*Pre_condiciones: 
  Post_condiciones: Devuelve “true” si la clave es inexistente o “false” en caso contrario.*/  
bool clave_inexistente(const char* clave)
{
    if(!clave)
        return true;
    else
        return false;
}

/*Pre_condiciones: La clave pasada por parámetro no debe ser nula.
  Post_condiciones: Devuelve un número distinto dependiendo la clave pasada por parámetro*/
size_t funcion_hash(const char* clave)
{
    size_t numero = 0;

    while (*clave)
    {
        numero += ((size_t)*(clave));
        clave++;
    }
    
    return numero;
}

/*Pre_condiciones: La clave pasada por parámetro no debe ser nula.
  Post_condiciones: Reserva memoria para crear un casillero y la inicializa con la clave y el elemento pasados 
  por parámetro. En caso de error devuelve NULL*/
casillero_t* crear_casillero(const char* clave, void* elemento)
{
    size_t largo_clave = strlen(clave) + 1;
    casillero_t* casillero_nuevo = malloc(sizeof(casillero_t));

    if (!casillero_nuevo)
        return NULL;
    
    casillero_nuevo->clave = malloc(largo_clave);

    if (!casillero_nuevo->clave)
    {
        free(casillero_nuevo);
        return NULL;
    }
    
    strcpy(casillero_nuevo->clave, clave);
    casillero_nuevo->elemento = elemento;
    casillero_nuevo->siguiente = NULL;

    return casillero_nuevo;
}

/*Pre_condiciones: La clave pasada por parámetro no debe ser nula.
  Post_condiciones: Crea un casillero en la última posición de la cadena (casillero_actual es el primer elemento de ella)
   o actualiza un casillero existente si la clave pasada por parámetro coincide con ese casillero.*/
casillero_t* posicionar_casillero(casillero_t* casillero_actual, const char* clave, void* elemento, hash_destruir_dato_t funcion_destructora, bool* nuevo_casillero, bool* error, bool* actualizacion)
{
    if (!casillero_actual)
    {
        casillero_t* casillero_nuevo = crear_casillero(clave, elemento);
        if(!casillero_nuevo)
        {
            *error = true;
            return NULL;
        }
        *nuevo_casillero = true;
        return casillero_nuevo;
    }
    if (strcmp(casillero_actual->clave, clave) == 0)
    {
        *actualizacion = true;
        
        if (funcion_destructora)
        {
            funcion_destructora(casillero_actual->elemento);
        } 

        casillero_actual->elemento = elemento;
        return casillero_actual;
    }
    
    casillero_actual->siguiente = posicionar_casillero(casillero_actual->siguiente, clave, elemento, funcion_destructora, nuevo_casillero, error, actualizacion);
    
    return casillero_actual;
}

/*Pre_condiciones: El casillero pasado por parámetro no debe ser nulo.
  Post_condiciones: Libera el espacio reservado por la cadena (casillero_actual es el primer elemento de ella).*/
void destruir_cadena(casillero_t* casillero, hash_destruir_dato_t funcion_destructora)
{
    if (!casillero->siguiente)
    {
        if (funcion_destructora)
        {
            funcion_destructora(casillero->elemento);
        } 
        free(casillero->clave);
        free(casillero);

        return;
    }
    else
    {
        destruir_cadena(casillero->siguiente, funcion_destructora);
    }

    if (funcion_destructora)
    {
        funcion_destructora(casillero->elemento);
    } 
    free(casillero->clave); 
    free(casillero);
}

/*Pre_condiciones: 
  Post_condiciones: Devuelve el casillero que posee la clave pasada por parámetro de una cadena 
  (casillero_actual es el primer elemento de ella).*/
casillero_t* obtener_casillero(casillero_t* casillero_actual, const char* clave)
{
    if (!casillero_actual)
        return NULL;
    if (casillero_actual->clave)
    {
        if (strcmp(casillero_actual->clave, clave) == 0)
            return casillero_actual;
    }
    else
        return NULL;
    
    return obtener_casillero(casillero_actual->siguiente, clave);
}

/*Pre_condiciones: 
  Post_condiciones: Devuelve el casillero anterior al que posee la clave pasada por parámetro de una cadena 
  (casillero_actual es el primer elemento de ella).*/
casillero_t* obtener_casillero_anterior(casillero_t* casillero_actual, const char* clave)
{
    if (!casillero_actual)
        return NULL;
   
    if (casillero_actual->siguiente)
    {
        if (strcmp(casillero_actual->siguiente->clave, clave) == 0)
        return casillero_actual;
    }
    else
        return NULL;
    
    return obtener_casillero_anterior(casillero_actual->siguiente, clave);
}

/*Pre_condiciones: 
  Post_condiciones: Devuelve el casillero que posee la clave pasada por parámetro de un hash.*/
casillero_t* buscar_casillero(hash_t* hash, const char* clave)
{
    if ((hash_inexistente_vacio(hash) != CON_ELEMENTOS) || (clave_inexistente(clave)))
        return NULL;

    int posicion = ((int)(funcion_hash(clave) % hash->cantidad_casilleros));
    
    casillero_t* buscado = obtener_casillero(hash->casilleros[posicion], clave);
    
    return buscado;
}

/*Pre_condiciones: 
  Post_condiciones: Libera la memoria reservada por un casillero y aplica la función destructora al elemento
   en caso de no ser nula. Si el casillero es nulo devuelve error (-1)*/
int eliminar_casillero(casillero_t* casillero, hash_destruir_dato_t funcion_destructora)
{
    if (!casillero)
        return ERROR;
    
    free(casillero->clave);
    casillero->clave = NULL;

    if (funcion_destructora)
    {
        funcion_destructora(casillero->elemento);
        casillero->elemento = NULL;
    }

    free(casillero);
    return EXITO;
}

/*Pre_condiciones: 
  Post_condiciones: Devuelve el factor de carga del hash pasado por parámetro. Si el hash es nulo devuelve error (-1).*/
int calcular_factor_carga(hash_t* hash)
{
    if (!hash)
        return ERROR;
    
    return ((int)(hash->elementos_almacenados / (float)hash->cantidad_casilleros));
}

/*Pre_condiciones: El hash pasado por parámetro no debe ser nulo.
  Post_condiciones: Copia los elementos de una cadena a un hash pasado por parámetro. 
  (casillero_actual es el primer elemento de la cadena)*/
void copiar_cadena(casillero_t* casillero_actual, hash_t* nuevo_hash)
{
    if(!casillero_actual)
        return;
    
    copiar_cadena(casillero_actual->siguiente, nuevo_hash);

    hash_insertar(nuevo_hash, casillero_actual->clave, casillero_actual->elemento);
    free(casillero_actual->clave);
    free(casillero_actual);
}

/*Pre_condiciones: 
  Post_condiciones: Expande el tamaño del hash pasado por parametro aumentando la cantidad de casilleros que posee.
   Adicionalmente reubica los elementos en sus nuevas posiciones. En caso de error devuelve NULL.*/
hash_t* rehash(hash_t* hash_original)
{
    if (!hash_original)
        return NULL;    
    
    hash_t* nuevo_hash = hash_crear(hash_original->funcion_destructora, (hash_original->cantidad_casilleros * 2));

    if (!nuevo_hash)
        return NULL;

    for (size_t i = 0; i < hash_original->cantidad_casilleros; i++)
    {
        copiar_cadena(hash_original->casilleros[i], nuevo_hash);
    }
    
    return nuevo_hash;
}

/*Pre_condiciones: El hash, el casillero y la función pasados por parámetro no deben ser nulos.
  Post_condiciones: Recorre la cadena hasta el final  y empieza a aplicar la función pasada por parámetro a cada
   elemento desde el ultimo hasta el primero.*/
size_t iterar_cadena(hash_t* hash, casillero_t* casillero, bool (*funcion)(hash_t* hash, const char* clave, void* aux), void* aux, bool * resultado_funcion)
{
    size_t iteraciones = 0;
    if (!casillero->siguiente)
    {
        *resultado_funcion = funcion(hash, casillero->clave, aux);
        return 1;
    }
    else
    {
        iteraciones = iterar_cadena(hash, casillero->siguiente, funcion, aux, resultado_funcion);
        if (!(*resultado_funcion))
        {
            *resultado_funcion = funcion(hash, casillero->clave, aux);
            iteraciones ++;
        }

        return iteraciones;
    }
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
hash_t* hash_crear(hash_destruir_dato_t destruir_elemento, size_t capacidad_inicial)
{
    hash_t* nuevo_hash = malloc(sizeof(hash_t));
    
    if (!nuevo_hash)
        return NULL;

    if (capacidad_inicial < CAPACIDAD_MINIMA)
        capacidad_inicial = CAPACIDAD_MINIMA;
    
    nuevo_hash->elementos_almacenados = 0;
    nuevo_hash->funcion_destructora = destruir_elemento;
    nuevo_hash->cantidad_casilleros = capacidad_inicial;
    nuevo_hash->casilleros = calloc(capacidad_inicial, sizeof(casillero_t));

    if (!nuevo_hash->casilleros)
    {
        free(nuevo_hash);
        return NULL;
    }

    return nuevo_hash;
}

int hash_insertar(hash_t* hash, const char* clave, void* elemento)
{
    if ((hash_inexistente_vacio(hash) == INEXISTENTE) || clave_inexistente(clave))
        return ERROR;

    bool nuevo_casillero = false;
    bool error = false;
    bool actualizacion = false;
    hash_t* nuevo_hash = NULL;
    int factor_carga = 0;

    size_t posicion = (funcion_hash(clave) % hash->cantidad_casilleros);
    
    hash->casilleros[posicion] = posicionar_casillero(hash->casilleros[posicion], clave, elemento, hash->funcion_destructora, &nuevo_casillero, &error, &actualizacion);

    if (error)
        return ERROR;
    if(!actualizacion)
        hash->elementos_almacenados ++;

    factor_carga = calcular_factor_carga(hash);

    if (factor_carga >= LIMITE_REHASH)
    {
        nuevo_hash = rehash(hash);
        if (!nuevo_hash)
            return ERROR;

        free(hash->casilleros);
        hash->cantidad_casilleros = nuevo_hash->cantidad_casilleros;
        hash->casilleros = nuevo_hash->casilleros;

        free(nuevo_hash);
    } 
    
    return EXITO;  
}

size_t hash_cantidad(hash_t* hash)
{
    if (hash_inexistente_vacio(hash) == INEXISTENTE)
        return VACIO;

    return (size_t)hash->elementos_almacenados;
}

void hash_destruir(hash_t* hash)
{
    if (hash_inexistente_vacio(hash) == INEXISTENTE)
        return;
    
    if (hash_inexistente_vacio(hash) == CON_ELEMENTOS)
    {
        for (size_t i = 0; i < hash->cantidad_casilleros; i++)
        {
            if (hash->casilleros[i])
            {
                destruir_cadena(hash->casilleros[i], hash->funcion_destructora);
            }
        }
    }
    free(hash->casilleros);
    free(hash);
}

void* hash_obtener(hash_t* hash, const char* clave)
{
    casillero_t* buscado = buscar_casillero(hash, clave);

    if (!buscado)
        return NULL;
    else
        return buscado->elemento;
}

bool hash_contiene(hash_t* hash, const char* clave)
{
    casillero_t* buscado = buscar_casillero(hash, clave);

    if (!buscado)
        return false;
    else 
        return true;
}

/*Pre_condiciones:
  Post_condiciones: Devuelve “VERDADERO” (1) si la clave del casillero y la segunda clave pasadas por parámetro 
  coinciden, sino devuelve “FALSO” (0). En caso de error devuelve “ERROR” (-1).*/
int comprobar_clave_casillero(casillero_t* casillero, const char* clave)
{
    if ((!casillero) || (!clave))
        return ERROR;
    else if (!casillero->clave)
        return ERROR;
    else if (strcmp(casillero->clave, clave) == 0)
        return VERDADERO;
    else
        return FALSO;
}

int hash_quitar(hash_t* hash, const char* clave)
{
    int resultado = ERROR;
    casillero_t* aux = NULL;
    
    if ((hash_inexistente_vacio(hash) != CON_ELEMENTOS) || clave_inexistente(clave))
        return resultado;

    int posicion = ((int)(funcion_hash(clave) % hash->cantidad_casilleros));

    if (hash->casilleros[posicion])
    {
        if (comprobar_clave_casillero(hash->casilleros[posicion], clave) == VERDADERO)
        {
            if (hash->casilleros[posicion]->siguiente)
                aux = hash->casilleros[posicion]->siguiente;

            resultado = eliminar_casillero(hash->casilleros[posicion], hash->funcion_destructora);
            hash->casilleros[posicion] = aux;

            if (resultado == EXITO)
                hash->elementos_almacenados --;
                    
            return resultado;
        }

        casillero_t* anterior_a_eliminar = obtener_casillero_anterior(hash->casilleros[posicion], clave); //Si elimina un casillero conectado
        
        if (anterior_a_eliminar)
        {
            casillero_t* casillero_a_eliminar = anterior_a_eliminar->siguiente;
            anterior_a_eliminar->siguiente = casillero_a_eliminar->siguiente;
            resultado = eliminar_casillero(casillero_a_eliminar, hash->funcion_destructora);
            if(resultado == EXITO)
                hash->elementos_almacenados --;
                
            return resultado;
        }
    }
    return resultado;
}

size_t hash_con_cada_clave(hash_t* hash, bool (*funcion)(hash_t* hash, const char* clave, void* aux), void* aux)
{
    size_t posicion_actual = 0;
    size_t iteraciones = 0;
    bool resultado_funcion = false;

    if ((hash_inexistente_vacio(hash) == INEXISTENTE) || !funcion)
    {
        return iteraciones;
    }

    while((posicion_actual < hash->cantidad_casilleros) && (!resultado_funcion))
    {   
        if (hash->casilleros[posicion_actual])
        {
            iteraciones += iterar_cadena(hash, hash->casilleros[posicion_actual], funcion, aux, &resultado_funcion);
        }
        posicion_actual ++;
    }
    
    return iteraciones;
}