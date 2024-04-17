#include <stdio.h>
#include <string.h>
#include "hospital.h"
#include "split.h"
#include "lista.h"
#include "heap.h"
#include "abb.h"

#define ESPACIO_INICIAL 100
#define SEPARADOR ';'
#define DATOS_ENTRENADOR 2
#define MODO_LECTURA "r"
#define ENTRENADORES_INICIAL 0
#define POKEMONES_INICIAL 0
#define SIN_INVOCACIONES 0
#define CANTIDAD_INICIAL_POKEMONES 10

struct _pkm_t
{
    char* nombre;
    size_t nivel;
};

struct _entrenador_t
{
    size_t id;
    char* nombre;
    heap_t* heap_pokemones;
};

struct _hospital_pkm_t
{
    lista_t* lista_entrenadores;
    size_t cantidad_pokemones;
};

// FUNCIONES PROPIAS /////////////////////////////////////////////////////////////////////////////////////////////////
/*Pre_condiciones: El archivo debe ser abierto previamente.
  Post_condiciones: Devuelve el puntero a un vector con la información extraida de una linea del archivo a leer.*/
char* leer_linea(char * buffer, size_t espacio_buffer, FILE* archivo, size_t largo_lineas_anteriores)
{
    bool linea_copiada = false;

    fseek(archivo, (long)largo_lineas_anteriores, SEEK_SET);
    fgets(buffer, (int)espacio_buffer, archivo);
    
    for (size_t i = 0; i < espacio_buffer; i++)
    {
        if (((buffer[i] == '\n') || (feof(archivo))) && (linea_copiada == false))
        {
            linea_copiada = true;
        }
    }

    while (!linea_copiada)
    {
        buffer = realloc(buffer, espacio_buffer * 2);
        espacio_buffer = (espacio_buffer * 2);

        for (size_t i = 0; i < espacio_buffer; i++)
        {
            buffer[i] = 0;
        }

        fseek(archivo, (long)largo_lineas_anteriores, SEEK_SET);
        fgets(buffer, (int)espacio_buffer, archivo);
    
        for (size_t i = 0; i < espacio_buffer; i++)
        {
            if ((buffer[i] == '\n') || (feof(archivo)))
            {
                linea_copiada = true;
            }
        }
    }
    return buffer;
}

/*Pre_condiciones: Los elementos deben ser del tipo "pokemon_t*" y no deben ser nulos.
  Post_condiciones: Compara los niveles de los pokemones pasados por parámetro y devuelve “1” en caso de que el 
  primero tenga mayor nivel o “-1” en caso de que sea igual o menor.*/
int comparador_niveles(void* elemento_1, void* elemento_2)
{
    int resultado = 0;

    if ((!elemento_1) || (!elemento_2))
        return resultado;
    
    pokemon_t* pokemon_1 = (pokemon_t*)elemento_1;
    pokemon_t* pokemon_2 = (pokemon_t*)elemento_2;
    
    if (pokemon_1->nivel > pokemon_2->nivel)
        resultado = 1;
    else
        resultado = -1;
    
    return resultado;
}

/*Pre_condiciones: Los elementos deben ser del tipo "pokemon_t*" y no deben ser nulos.
  Post_condiciones: Compara los nombres de los dos pokemones pasados por parámetro y devuelve un valor mayor a 0 
  si el primero es mayor que el segundo, menor a 0 si el segundo es mayor al primero o “0” si son iguales.*/
int comparador(void* elemento_1, void* elemento_2)
{
    pokemon_t* pokemon_1 = (pokemon_t*)elemento_1;
    pokemon_t* pokemon_2 = (pokemon_t*)elemento_2;
    return strcmp(pokemon_1->nombre, pokemon_2->nombre);
}

/*Pre_condiciones: La linea extraida del archivo debe seguir el formato "ID;Nombre_entrenador;Nombre_pokemon1;Nivel_pokemon1;Nombre_pokemon2;Nivel_pokemon2;etc...".
  Post:condiciones:Devuelve un puntero al bloque de memoria con la estructura del entrenador.*/
entrenador_t* cargar_entrenador(lista_t* linea_entrenador, size_t tope_pokemones)
{
    entrenador_t* entrenador = calloc(1, sizeof(entrenador_t));
    entrenador->nombre = calloc(strlen(linea_entrenador->nodo_inicio->siguiente->elemento) + 1, sizeof(char));
    entrenador->heap_pokemones = heap_crear(comparador_niveles, CANTIDAD_INICIAL_POKEMONES);
    entrenador->id = (size_t)atoi(linea_entrenador->nodo_inicio->elemento);
    strcpy(entrenador->nombre, linea_entrenador->nodo_inicio->siguiente->elemento); //Segunda posicion en la lista almacena el nombre

    nodo_t* linea_aux = linea_entrenador->nodo_inicio->siguiente->siguiente; //Este nodo empieza los pokemones de la lista
    for (size_t i = 0; i < tope_pokemones; i++)
    {
        pokemon_t* pokemon_aux =  calloc(1, sizeof(pokemon_t));
        pokemon_aux->nombre = calloc(strlen(linea_aux->elemento) + 1, sizeof(char));
        strcpy(pokemon_aux->nombre, linea_aux->elemento);
        linea_aux = linea_aux->siguiente;
        pokemon_aux->nivel = (size_t)atoi(linea_aux->elemento); //Almaceno nivel
        heap_insertar(entrenador->heap_pokemones, pokemon_aux);

        linea_aux = linea_aux->siguiente; //Paso al siguiente 
    }

    return entrenador;
}

/*Pre_condiciones: 
  Post_condiciones: Devuelve true si un hospital está vacío o es nulo, o false en caso contrario.*/
bool hospital_invalido(hospital_t* hospital)
{
    if (hospital)
        if (hospital_cantidad_pokemon(hospital) > 0)
            return false;

    return true;
}

/*Pre_condiciones: El elemento debe ser del tipo “pokemon_t*”
  Post_condiciones: Libera la memoria reservada por un pokemon.*/
void destruir_pokemon(void* elemento)
{
    if(!elemento)
        return;

    pokemon_t* pokemon_actual = (pokemon_t*)elemento;
    free(pokemon_actual->nombre);
    free(pokemon_actual);
}

/*Pre_condiciones: El elemento pasado por parámetro no debe ser nulo.
  Post_condiciones: Libera la memoria reservada por un entrenador. En caso de ingresar un 
  elemento nulo solo se devuelve true.*/
bool destruir_entrenador(void* elemento, void* contexto)
{   
    if(!elemento)
        return true;

    entrenador_t* entrenador_actual = (entrenador_t*)elemento;
    heap_a_cada_elemento(entrenador_actual->heap_pokemones, destruir_pokemon);
    heap_destruir(entrenador_actual->heap_pokemones);
    free(entrenador_actual->nombre);
    free(entrenador_actual);
    return true;
}

/*Pre_condiciones: El puntero al nodo y el puntero a la función no deben ser NULL. "resultado" debe apuntar a un bool en estado "true".
  Post_condiciones: Recorre el arbol con recorrido inorden e invoca la funcion con cada elemento almacenado en el mismo
  como primer parámetro. El puntero aux se pasa como segundo parámetro a la función. Si la función devuelve false, se
  finaliza el recorrido aun si quedan elementos por recorrer. Si devuelve true se sigue recorriendo mientras queden
  elementos. Devuelve la cantidad de veces que fue invocada la función.*/
size_t aplicar_funcion_pokemones_arbol(nodo_abb_t* nodo, bool (*funcion)(pokemon_t* p), bool *resultado)
{
  size_t contador = SIN_INVOCACIONES;

  if (!nodo)
  {
    return contador;
  }

  if (*resultado)
  {
    contador += aplicar_funcion_pokemones_arbol(nodo->izquierda, funcion, resultado);
  }
  if (*resultado)
  {
    *resultado = funcion(nodo->elemento);
    contador ++;
  }
  if (*resultado)
  {
    contador += aplicar_funcion_pokemones_arbol(nodo->derecha, funcion, resultado);
  }
  
  return contador;
}

/*Pre_condiciones: El entrenador y el abb no deben ser nulos.
  Post_condiciones: Copia los pokemones del entrenador a el abb pasado por parámetro.*/
void copiar_entrenador(entrenador_t* entrenador, abb_t* abb_pokemones)
{
    if((!entrenador) && (!abb_pokemones))
        return;
    
    for (int i = 0; i < heap_cantidad(entrenador->heap_pokemones); i++)
    {
        abb_insertar(abb_pokemones, heap_obtener_posicion(entrenador->heap_pokemones, i));
    }
    
}

/*Pre_condiciones: El elemento pasado por parámetro no debe ser nulo.
  Post_condiciones: Libera el espacio reservado por un elemento de la lista y devuelve true. En caso de ingresar un 
  elemento nulo solo se devuelve true.*/
bool destruir_linea(void* elemento, void* contexto)
{
    if(!elemento)
        return true;

    free(elemento);
    return true;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

hospital_t* hospital_crear()
{
    hospital_t* nuevo_hospital = calloc(1, sizeof(hospital_t));
    
    if(!nuevo_hospital)
        return NULL;
    
    nuevo_hospital->lista_entrenadores = lista_crear();

    return nuevo_hospital;
}

bool hospital_leer_archivo(hospital_t* hospital, const char* nombre_archivo)
{
     FILE* archivo = fopen(nombre_archivo, MODO_LECTURA);

    if (archivo == NULL)
    {
        perror("\nError al abrir el archivo");
        return false;
    }
    
    entrenador_t* entrenador_resultado;
    size_t tope_pokemones = 0;

    lista_t* linea_entrenador;

    char* buffer;
    size_t espacio_buffer = 0;
    size_t largo_lineas_anteriores = 0;
    
    while (!feof(archivo))
    {
        espacio_buffer = ESPACIO_INICIAL;
        buffer = calloc(1 ,espacio_buffer);

        if (!buffer)
            return false;
        
        buffer = leer_linea(buffer, espacio_buffer, archivo, largo_lineas_anteriores);

        if (strlen(buffer) == 0)
        {
            free(buffer);
            fclose(archivo);
            return true;
        }

        linea_entrenador = split(buffer, SEPARADOR); 
        tope_pokemones = (linea_entrenador->cantidad - DATOS_ENTRENADOR)/2;
        entrenador_resultado = cargar_entrenador(linea_entrenador, tope_pokemones);
        hospital->lista_entrenadores = lista_insertar(hospital->lista_entrenadores, entrenador_resultado);
        hospital->cantidad_pokemones += heap_cantidad(entrenador_resultado->heap_pokemones);
        largo_lineas_anteriores = (largo_lineas_anteriores + (strlen(buffer)));

        free(buffer);
        lista_con_cada_elemento(linea_entrenador, destruir_linea, NULL);
        lista_destruir(linea_entrenador);
    }

    fclose(archivo);
    return true;
}

size_t hospital_cantidad_pokemon(hospital_t* hospital)
{
    if (!hospital)
        return 0;

    return hospital->cantidad_pokemones;
}

size_t hospital_cantidad_entrenadores(hospital_t* hospital)
{
    if (!hospital)
        return 0;
    
    return hospital->lista_entrenadores->cantidad;
}

size_t hospital_a_cada_pokemon(hospital_t* hospital, bool (*funcion)(pokemon_t* p))
{
    size_t contador = 0;
    bool resultado_funcion = true;
    
    if ((hospital_invalido(hospital)) || (!funcion))
        return contador;

    nodo_t* nodo_actual = hospital->lista_entrenadores->nodo_inicio;
    entrenador_t* entrenador_aux = (entrenador_t*)nodo_actual->elemento;
    abb_t* abb_pokemones = abb_crear(comparador);

    for (size_t i = 0; i < hospital->lista_entrenadores->cantidad; i++)
    {
        copiar_entrenador(entrenador_aux, abb_pokemones);

        if (nodo_actual->siguiente)
        {
            nodo_actual = nodo_actual->siguiente;
            entrenador_aux = (entrenador_t*)nodo_actual->elemento;
        }  
    }
    
    contador = aplicar_funcion_pokemones_arbol(abb_pokemones->nodo_raiz, funcion, &resultado_funcion);
    
    abb_destruir(abb_pokemones);
    return contador;
}

/*
size_t hospital_a_cada_pokemon(hospital_t* hospital, bool (*funcion)(pokemon_t* p))
{
    size_t contador = 0;
    
    if ((hospital_invalido(hospital)) || (!funcion))
    {
        return contador;
    }

    nodo_t* nodo_actual = hospital->lista_entrenadores->nodo_inicio;
    entrenador_t* entrenador_aux = (entrenador_t*)nodo_actual->elemento;
    heap_t* heap_pokemones = heap_crear(comparador, CANTIDAD_INICIAL_POKEMONES);

    for (size_t i = 0; i < hospital->lista_entrenadores->cantidad; i++)
    {
        copiar_entrenador(entrenador_aux, heap_pokemones);

        if (nodo_actual->siguiente)
        {
            nodo_actual = nodo_actual->siguiente;
            entrenador_aux = (entrenador_t*)nodo_actual->elemento;
        }  
    }
    
    contador = heap_a_cada_pokemon(heap_pokemones, funcion);

    heap_destruir(heap_pokemones);
    return contador;
}
*/


void hospital_destruir(hospital_t* hospital)
{
    if (!hospital)
        return;
    
    lista_con_cada_elemento(hospital->lista_entrenadores, destruir_entrenador, NULL);
    lista_destruir(hospital->lista_entrenadores);
    free(hospital);
}

size_t pokemon_nivel(pokemon_t* pokemon)
{
    if (!pokemon)
        return 0;
    
    return pokemon->nivel;
}

const char* pokemon_nombre(pokemon_t* pokemon)
{
    if (!pokemon)
        return NULL;

    return pokemon->nombre;
}   