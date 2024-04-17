#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include "lista.h"

size_t contar_separadores(const char* string, char separador)
{
    size_t largo_string = strlen(string);
    size_t num_separadores = 0;

    for(int i = 0; i < largo_string; i++)
    {
        if(string[i] == separador)
            num_separadores++;
    }
    return num_separadores;
}

size_t buscar_separador(const char* string, char separador)
{
    size_t i = 0;

    while ((string[i]) && (string[i] != separador))
        i++;
    
    return i;
}

char * copiar_subestring(const char * string, size_t longitud_subesting)
{
    char * nuevo_subestring = calloc(longitud_subesting + 1, sizeof(char));
    strncpy(nuevo_subestring, string, longitud_subesting);
    nuevo_subestring[longitud_subesting] = 0;

    return nuevo_subestring;
}

lista_t* split(const char* string, char separador)
{
    if(!string)
    {
        return NULL;
    }

    size_t num_separadores = contar_separadores(string, separador);
    size_t substrings = num_separadores + 1;

    lista_t* lista_substrings = lista_crear();

    if (!lista_substrings)
        return NULL;
    
    for (size_t i = 0; i < substrings; i++)
    {
        size_t longitud_substring = buscar_separador(string, separador);
        char * nuevo_substring = copiar_subestring(string, longitud_substring);
        
        if(!nuevo_substring)
        {
            lista_destruir(lista_substrings);
            return NULL;
        }

        lista_substrings = lista_insertar(lista_substrings, nuevo_substring);
        string += longitud_substring + 1;
    }
    
    return lista_substrings;
}