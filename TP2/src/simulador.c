#include "simulador.h"
#include "lista.h"
#include "hash.h"
#include "heap.h"
#include <stdio.h>
#include <string.h>

#define NULO 0;
#define CANTIDAD_INICIAL_POKEMONES 10
#define EXITO 0
#define ERROR -1

//Dificultades
#define PUNTOS_PERFECTOS 3
#define PUNTOS_INTERMEDIOS 2
#define PUNTOS_BASICOS 1
#define PUNTOS_NULOS 0
#define FACIL 0
#define NORMAL 1
#define DIFICIL 2
#define DIFICULTADES_ESTANDAR 3


typedef struct dificultad
{
    char* nombre;
    int id;
    bool en_uso;
    unsigned (*calcular_puntaje)(unsigned cantidad_intentos);
    int (*verificar_nivel)(unsigned nivel_adivinado, unsigned nivel_pokemon);
    const char* (*verificacion_a_string)(int resultado_verificacion);
} dificultad_t;

typedef struct paciente
{
    pokemon_t* pokemon;
    char* nombre_entrenador;
} paciente_t;

typedef struct opciones_dificultades
{
    dificultad_t* dificultad_actual;
    hash_t* hash_dificultades;
    int ultimo_id; 
} opciones_dificultades_t;

typedef struct consultorio
{
    heap_t* pacientes_espera;
    paciente_t* paciente_actual;
    unsigned intentos;
} consultorio_t;

struct _simulador_t
{
    size_t indice_entrenador_actual;
    hospital_t* hospital;
    opciones_dificultades_t opciones_dificultades;
    consultorio_t consultorio;
    EstadisticasSimulacion estadisticas;
    bool en_ejecucion;
};

struct _hospital_pkm_t
{
    lista_t* lista_entrenadores;
    size_t cantidad_pokemones;
};

struct _entrenador_t
{
    size_t id;
    char* nombre;
    heap_t* heap_pokemones;
};

//Funciones Propias////////////////////////////////////////////////////////////////////////////////////////////////////
/*Pre_condiciones: La cantidad de intentos debe ser mayor a 0.
  Post_condiciones: Devuelve la cantidad de puntos ganados por acertar dada la cantidad de intentos requeridos 
  en la dificultad "facil"*/
unsigned calcular_puntaje_facil(unsigned cantidad_intentos)
{
    if (cantidad_intentos <= 10)
        return PUNTOS_PERFECTOS;
    else if (cantidad_intentos <= 20)
        return PUNTOS_INTERMEDIOS;
    else
        return PUNTOS_BASICOS;
}

/*Pre_condiciones: La cantidad de intentos debe ser mayor a 0.
  Post_condiciones: Devuelve la cantidad de puntos ganados por acertar dada la cantidad de intentos requeridos 
  en la dificultad "normal"*/
unsigned calcular_puntaje_normal(unsigned cantidad_intentos)
{
    if (cantidad_intentos <= 5)
        return PUNTOS_PERFECTOS;
    else if (cantidad_intentos <= 10)
        return PUNTOS_INTERMEDIOS;
    else if (cantidad_intentos <= 15)
        return PUNTOS_BASICOS;
    else
        return PUNTOS_NULOS;
}

/*Pre_condiciones: La cantidad de intentos debe ser mayor a 0.
  Post_condiciones: Devuelve la cantidad de puntos ganados por acertar dada la cantidad de intentos requeridos 
  en la dificultad "difícil"*/
unsigned calcular_puntaje_dificil(unsigned cantidad_intentos)
{
    if (cantidad_intentos == 1)
        return PUNTOS_PERFECTOS;
    else if (cantidad_intentos <= 10)
        return PUNTOS_BASICOS;
    else
        return PUNTOS_NULOS;
}

/*Pre_condiciones:
  Post_condiciones: Recibe el numero adivinado y el correcto y devuelve 0 en caso de acertar 
  (y cualquier otro valor en otro caso)*/
int verificar_nivel(unsigned nivel_adivinado, unsigned nivel_pokemon)
{
    return ((int)(nivel_adivinado - nivel_pokemon));
}

/*Pre_condiciones: 
  Post_condiciones: Recibe el numero resultado de la función verificar_nivel y lo transforma en un string que 
  representa el resultado en forma de texto. (dificultad facil)*/
const char* verificacion_a_string_facil(int resultado_verificacion)
{
    if (resultado_verificacion < 0)
        resultado_verificacion *= -1;

    if (resultado_verificacion == EXITO)
        return "EXITO";
    else if (resultado_verificacion >= 30)
        return "A 30 niveles o mas de distancia";
    else if (resultado_verificacion >= 20)
        return "Entre 20 y 30 niveles de distancia";
    else if (resultado_verificacion >= 10)
        return "Entre 10 y 20 niveles de distancia";
    else
        return "A 10 niveles o menos de distancia";
}

/*Pre_condiciones: 
  Post_condiciones: Recibe el numero resultado de la función verificar_nivel y lo transforma en un string que 
  representa el resultado en forma de texto. (dificultad normal)*/
const char* verificacion_a_string_normal(int resultado_verificacion)
{
    if (resultado_verificacion < 0)
        resultado_verificacion *= -1;

    if (resultado_verificacion == EXITO)
        return "EXITO";
    else if (resultado_verificacion >= 35)
        return "Muy Lejos";
    else if (resultado_verificacion >= 25)
        return "Lejos";
    else if (resultado_verificacion >= 15)
        return "Cerca";
    else
        return "Muy cerca";
}

/*Pre_condiciones: 
  Post_condiciones: Recibe el numero resultado de la función verificar_nivel y lo transforma en un string que 
  representa el resultado en forma de texto. (dificultad difícil)*/
const char* verificacion_a_string_dificil(int resultado_verificacion)
{
  /*  if (resultado_verificacion < 0)
        resultado_verificacion *= -1;*/

    if (resultado_verificacion == EXITO)
        return "EXITO";
    else
        return "Respuesta invalida";
    
}

/*Pre_condiciones: El elemento pasado por parámetro no debe ser nulo.
  Post_condiciones: Libera la memoria reservada por los elementos almacenados en un hash.*/
void destruir_elementos_hash(void* elemento)
{
    if (elemento)
        free(elemento);
}

/*Pre_condiciones:
  Post_condiciones: Verifica si el simulador finalizo su ejecución y en caso afirmativo imprime por pantalla un 
  mensaje informando al usuario*/
bool simulacion_finalizada(bool estado)
{
    if (!estado)
    {
        printf("\nLa simulacion ha finalizado, no es posible ejecutar mas eventos.\n");
        return true;
    }
    
    return false;
}

/*Pre_condiciones: Los elementos pasados por parámetro no deben ser nulos y deben ser del tipo paciente_t.
  Post_condiciones: Compara dos pacientes pasados por parámetro y devuelve “1” en caso de que el primero posea 
  mayor nivel que el segundo o “-1” en caso contrario. */
int comparador_pacientes(void* elemento_1, void* elemento_2)
{
    int resultado = 0;

    if ((!elemento_1) || (!elemento_2))
        return resultado;
    
    paciente_t* paciente = (paciente_t*)elemento_1;
    paciente_t* paciente_2 = (paciente_t*)elemento_2;
    
    if (pokemon_nivel(paciente->pokemon) > pokemon_nivel(paciente_2->pokemon))
        resultado = 1;
    else
        resultado = -1;
    
    return resultado;
}

/*Pre_condiciones: El elemento pasado por parámetro no debe ser nulo y debe ser del tipo paciente_t.
  Post_condiciones: Libera la memoria reservada por el paciente pasado por parámetro*/
void destruir_pacientes(void* elemento)
{
    if(!elemento)
        return;

    paciente_t* paciente = (paciente_t*)elemento;
    free(paciente->nombre_entrenador);
    free(paciente);
}

/*Pre_condiciones: El hash no debe ser nulo y debe contener elementos del tipo "dificultad_t".
  Post_condiciones:  Libera la memoria de la dificultad almacenada en el hash con el id pasado por parámetro.
  Devuelve false en caso de error.*/
bool destruir_dificultad(hash_t* hash, const char* id, void* aux)
{
    if(!hash)
        return true;

    dificultad_t* dificultad = (dificultad_t*)hash_obtener(hash, id);
    if(dificultad)
    {
        free(dificultad->nombre);
        return false;
    }
    return false;
}

/*Pre_condiciones:
  Post_condiciones: Reserva memoria para una nueva dificultad y la inicializa con todos los elementos pasados 
  por parámetro. Devuelve NULL en caso de error.*/
dificultad_t* crear_dificultad(char* nombre, int id, bool en_uso, unsigned (*calcular_puntaje)(unsigned cantidad_intentos), const char* (*verificacion_a_string)(int resultado_verificacion))
{   
    dificultad_t* dificultad = malloc(sizeof(dificultad_t));

    if(!dificultad)
        return NULL;

    dificultad->id = id;
    dificultad->nombre = malloc(strlen(nombre) + 1);
    strcpy(dificultad->nombre, nombre);
    dificultad->en_uso = en_uso;
    dificultad->calcular_puntaje = calcular_puntaje;
    dificultad->verificar_nivel = verificar_nivel;
    dificultad->verificacion_a_string = verificacion_a_string;

    return dificultad;
}

/*Pre_condiciones: El simulador no debe ser nulo.
  Post_condiciones:  Crea el hash de dificultades del simulador y le introduce las 3 dificultades que vienen por 
  defecto con el mismo (fácil, normal, difícil). Devuelve NULL en caso de error.*/
hash_t* dificultades_iniciales(simulador_t* simulador)
{
    int resultado = EXITO;

    if(!simulador)
        return NULL;

    hash_t* hash_dificultades = hash_crear(destruir_elementos_hash, DIFICULTADES_ESTANDAR);

    dificultad_t* facil = crear_dificultad("FACIL", FACIL, false, calcular_puntaje_facil, verificacion_a_string_facil);

    if(!facil)
        return NULL;

    char id_clave[2];
    resultado = sprintf(id_clave, "%i", FACIL);
    if(resultado == ERROR)
        return NULL;
     
    resultado = hash_insertar(hash_dificultades, id_clave, facil);
    if(resultado != EXITO)
        return NULL;

    if(resultado == ERROR)
        return NULL;

    dificultad_t* normal = crear_dificultad("NORMAL", NORMAL, true, calcular_puntaje_normal, verificacion_a_string_normal);

    if(!normal)
        return NULL;

    char id_clave_2[2];
    resultado = sprintf(id_clave_2, "%i", NORMAL);
    if(resultado == ERROR)
        return NULL;

    resultado = hash_insertar(hash_dificultades, id_clave_2, normal);
    if(resultado != EXITO)
        return NULL;

    dificultad_t* dificil = crear_dificultad("DIFICIL", DIFICIL, false, calcular_puntaje_dificil, verificacion_a_string_dificil);

    if(!dificil)
        return NULL;

    char id_clave_3[2];
    resultado = sprintf(id_clave_3, "%i", DIFICIL);
    if(resultado == ERROR)
        return NULL;

    resultado = hash_insertar(hash_dificultades, id_clave_3, dificil);
    if(resultado != EXITO)
        return NULL;

    simulador->opciones_dificultades.ultimo_id = DIFICIL;
    simulador->opciones_dificultades.dificultad_actual = normal;
    return hash_dificultades;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
simulador_t* simulador_crear(hospital_t* hospital)
{
    if (!hospital)
        return NULL;    

    simulador_t* simulador = malloc(sizeof(simulador_t));
    simulador->en_ejecucion = true;
    simulador->indice_entrenador_actual = NULO;
    simulador->hospital = hospital;
    simulador->consultorio.intentos = 0;

    simulador->estadisticas.cantidad_eventos_simulados = NULO;
    simulador->estadisticas.entrenadores_atendidos = NULO;
    simulador->estadisticas.entrenadores_totales = (unsigned)hospital_cantidad_entrenadores(hospital);
    simulador->estadisticas.pokemon_atendidos = NULO;
    simulador->estadisticas.pokemon_en_espera = NULO;
    simulador->estadisticas.pokemon_totales = (unsigned)hospital_cantidad_pokemon(hospital);
    simulador->estadisticas.puntos = NULO;
    
    simulador->opciones_dificultades.hash_dificultades = dificultades_iniciales(simulador);

    if(!simulador->opciones_dificultades.hash_dificultades)
        return NULL;

    simulador->consultorio.paciente_actual = NULL;
    simulador->consultorio.pacientes_espera = heap_crear(comparador_pacientes, CANTIDAD_INICIAL_POKEMONES);

    return simulador;
}

void simulador_destruir(simulador_t* simulador)
{
    if(!simulador)
        return;
    
    hospital_destruir(simulador->hospital);
    heap_a_cada_elemento(simulador->consultorio.pacientes_espera, destruir_pacientes);
    heap_destruir(simulador->consultorio.pacientes_espera);
    hash_con_cada_clave(simulador->opciones_dificultades.hash_dificultades, destruir_dificultad, NULL);
    hash_destruir(simulador->opciones_dificultades.hash_dificultades);  
    
    if (simulador->consultorio.paciente_actual)
    {
        free(simulador->consultorio.paciente_actual->nombre_entrenador);
        free(simulador->consultorio.paciente_actual);
    }

    free(simulador);
}

/*Pre_condiciones: El simulador y la estructura pasada por parámetro no deben ser nulos.
  Post_condiciones: Inicializa la estructura pasada por parámetro con las estadisticas corerspondientes. 
  Devuelve “ErrorSimulacion” en caso de error o “ExitoSimulacion” en caso contrario.*/
ResultadoSimulacion obtener_estadisticas(EstadisticasSimulacion* estadisticas, simulador_t* simulador)
{
    if((!simulador) || (!estadisticas))
        return ErrorSimulacion;
    else if(simulacion_finalizada(simulador->en_ejecucion))
        return ErrorSimulacion;

    estadisticas->puntos = simulador->estadisticas.puntos;
    estadisticas->cantidad_eventos_simulados = simulador->estadisticas.cantidad_eventos_simulados;
    estadisticas->entrenadores_atendidos = simulador->estadisticas.entrenadores_atendidos;
    estadisticas->entrenadores_totales = simulador->estadisticas.entrenadores_totales;
    estadisticas->pokemon_atendidos = simulador->estadisticas.pokemon_atendidos;
    estadisticas->pokemon_en_espera = simulador->estadisticas.pokemon_en_espera;
    estadisticas->pokemon_totales = (unsigned)hospital_cantidad_pokemon(simulador->hospital);

    return ExitoSimulacion;
}

/*Pre_condiciones: El simulador no debe ser nulo.
  Post_condiciones: Si quedan entrenadores en la sala de espera selecciona a el primero en orden de llegada y lo pasa a 
  estado de atendido y sus pokemon quedan en espera. Devuelve “ErrorSimulacion” en caso de error o “ExitoSimulacion” 
  en caso contrario.*/
ResultadoSimulacion atender_proximo_entrenador(simulador_t* simulador)
{
    if(!simulador)
        return ErrorSimulacion;
    else if(simulacion_finalizada(simulador->en_ejecucion))
        return ErrorSimulacion;

    entrenador_t* nuevo_entrenador = (entrenador_t*)lista_elemento_en_posicion(simulador->hospital->lista_entrenadores, simulador->indice_entrenador_actual);

    if(!nuevo_entrenador)
    {
        printf("\nNo hay mas entrenadores en espera\n");
        return ErrorSimulacion;
    }
        
    size_t cantidad_pokemones = heap_cantidad(nuevo_entrenador->heap_pokemones);
    
    for (int i = 0; i < cantidad_pokemones; i++)
    {
        paciente_t* nuevo_paciente = malloc(sizeof(paciente_t));
        char* nombre_entrenador = malloc((strlen(nuevo_entrenador->nombre) + 1) * sizeof(char));
        pokemon_t* resultado = (pokemon_t*)heap_obtener_posicion(nuevo_entrenador->heap_pokemones, i);
        nuevo_paciente->pokemon = resultado;
        strcpy(nombre_entrenador, nuevo_entrenador->nombre);
        nuevo_paciente->nombre_entrenador = nombre_entrenador;
        heap_insertar(simulador->consultorio.pacientes_espera, nuevo_paciente);
    }
    
    printf("\nProximo entrenador: %s\n", nuevo_entrenador->nombre);
    simulador->estadisticas.entrenadores_atendidos++;
    simulador->estadisticas.pokemon_en_espera += (unsigned)cantidad_pokemones;
    simulador->indice_entrenador_actual ++;

    if (!simulador->consultorio.paciente_actual)
    {
        simulador->consultorio.paciente_actual = heap_extraer_raiz(simulador->consultorio.pacientes_espera);
        simulador->estadisticas.pokemon_en_espera --;
    }
    return ExitoSimulacion;
}

/*Pre_condiciones:  El simulador y la estructura pasada por parámetro no deben ser nulos.
  Post_condiciones: Llena los datos de la estructura con la informacion del pokemon que está siendo o debe 
  ser atendido. Devuelve “ErrorSimulacion” en caso de error o “ExitoSimulacion” en caso contrario.*/
ResultadoSimulacion obtener_informacion_pokemon_en_tratamiento(InformacionPokemon* informacion, simulador_t* simulador)
{
    if((!simulador) || (!informacion))
        return ErrorSimulacion;
    else if(simulacion_finalizada(simulador->en_ejecucion))
    {
        return ErrorSimulacion;
    }
    else if(!simulador->consultorio.paciente_actual)
    {
        informacion->nombre_entrenador = NULL;
        informacion->nombre_pokemon = NULL;
        printf("\nActualmente no se atiende a ningun pokemon\n");
        return ErrorSimulacion;
    }

    informacion->nombre_pokemon = pokemon_nombre(simulador->consultorio.paciente_actual->pokemon);
    informacion->nombre_entrenador = simulador->consultorio.paciente_actual->nombre_entrenador;

    return ExitoSimulacion;
}

/*Pre_condiciones:  El simulador y la estructura pasada por parámetro no deben ser nulos.
  Post_condiciones: Verifica si el nivel pasado es el correcto del pokemon que está siendo atendido 
  (usando las funciones que corresponden según la dificultad activa) y llena la estructura pasada por parámetro 
  con el resultado. Devuelve “ErrorSimulacion” en caso de error o “ExitoSimulacion” en caso contrario.*/
ResultadoSimulacion adivinar_nivel_pokemon(Intento* intento, simulador_t* simulador)
{
    paciente_t* paciente_anterior = NULL;
    int resultado;

    if ((!simulador) || (!intento))
        return ErrorSimulacion;
    else if(simulacion_finalizada(simulador->en_ejecucion))
        return ErrorSimulacion;
    else if (!simulador->consultorio.paciente_actual)
    {
        simulador->consultorio.paciente_actual = heap_extraer_raiz(simulador->consultorio.pacientes_espera);
        if (!simulador->consultorio.paciente_actual)
        {
            printf("\nNo quedan mas pacientes en espera. Se debe atender a un nuevo entrenador\n");
            return ErrorSimulacion;
        }
        simulador->estadisticas.pokemon_en_espera --;
    }
 
    resultado = simulador->opciones_dificultades.dificultad_actual->verificar_nivel(intento->nivel_adivinado, (unsigned)pokemon_nivel(simulador->consultorio.paciente_actual->pokemon));
    intento->resultado_string = simulador->opciones_dificultades.dificultad_actual->verificacion_a_string(resultado);
    simulador->consultorio.intentos ++;

    if (resultado == 0)
    {
        intento->es_correcto = true;
        paciente_anterior = simulador->consultorio.paciente_actual;
        free(paciente_anterior->nombre_entrenador);
        free(paciente_anterior);
        printf("\nPacientes restantes: %li\n", heap_cantidad(simulador->consultorio.pacientes_espera));
        simulador->estadisticas.pokemon_atendidos++;
        if (heap_cantidad(simulador->consultorio.pacientes_espera) > 0)
        {
            simulador->consultorio.paciente_actual = heap_extraer_raiz(simulador->consultorio.pacientes_espera);
            simulador->estadisticas.pokemon_en_espera--;
        }
        else
            simulador->consultorio.paciente_actual = NULL;
        
        simulador->estadisticas.puntos += simulador->opciones_dificultades.dificultad_actual->calcular_puntaje(simulador->consultorio.intentos);
        simulador->consultorio.intentos = 0; 
    }
    else
    {
        intento->es_correcto = false;
    }
    return ExitoSimulacion;
}

/*Pre_condiciones: El simulador y la estructura pasada por parámetro no deben ser nulos.
  Post_condiciones: Agrega una nueva dificultad al simulador usando los datos pasados por parámetro en la estructura
  Devuelve “ErrorSimulacion” en caso de error o “ExitoSimulacion” en caso contrario.*/
ResultadoSimulacion agregar_dificultad(DatosDificultad* datos_dificultad, simulador_t* simulador)
{   
    int resultado;
    char id_clave[2];

    if ((!datos_dificultad) || (!simulador))
        return ErrorSimulacion;
    else if(simulacion_finalizada(simulador->en_ejecucion))
        return ErrorSimulacion;
    
    dificultad_t* nueva_dificultad = malloc(sizeof(dificultad_t));
    nueva_dificultad->nombre = malloc(strlen(datos_dificultad->nombre) + 1);
    strcpy(nueva_dificultad->nombre, datos_dificultad->nombre);
    nueva_dificultad->en_uso = false;
    nueva_dificultad->id = simulador->opciones_dificultades.ultimo_id + 1;
    nueva_dificultad->calcular_puntaje = datos_dificultad->calcular_puntaje;
    nueva_dificultad->verificar_nivel = datos_dificultad->verificar_nivel;
    nueva_dificultad->verificacion_a_string = datos_dificultad->verificacion_a_string;

    resultado = sprintf(id_clave, "%hu", (unsigned short)nueva_dificultad->id);
    if(resultado == ERROR)
    {
        printf("\nError a la hora de agregar una nueva dificultad\n");
        return ErrorSimulacion;
    }

    hash_insertar(simulador->opciones_dificultades.hash_dificultades, id_clave, nueva_dificultad);
    simulador->opciones_dificultades.ultimo_id++;
    return ExitoSimulacion;
}

/*Pre_condiciones: El simulador y el elemento pasado por parámetro no deben ser nulos.
  Post_condiciones: Selecciona la dificultad especificada con el id pasado por parámetro y pasa a ser la 
  dificultad activa en el simulador. Devuelve “ErrorSimulacion” en caso de error o “ExitoSimulacion” en caso contrario.*/
ResultadoSimulacion seleccionar_dificultad(int* id,simulador_t* simulador)
{
    char id_clave[2];
    int resultado;
    dificultad_t* dificultad_buscada = NULL;

    if((!simulador) || (!id))
        return ErrorSimulacion;
    if(simulacion_finalizada(simulador->en_ejecucion))
        return ErrorSimulacion;

    resultado = sprintf(id_clave, "%hu", *(unsigned short*)id);
    if(!resultado)
        return ErrorSimulacion;

    dificultad_buscada = hash_obtener(simulador->opciones_dificultades.hash_dificultades, id_clave);

    if (!dificultad_buscada)
    {
        printf("\nError al seleccionar dificultad, posible ID inexistente\n");
        return ErrorSimulacion;
    }
    
    simulador->opciones_dificultades.dificultad_actual->en_uso = false;
    dificultad_buscada->en_uso = true;
    simulador->opciones_dificultades.dificultad_actual = dificultad_buscada;

    return ExitoSimulacion;
}

/*Pre_condiciones: El simulador y la estructura pasada por parámetro no deben ser nulos.
  Post_condiciones: Busca la dificultad con el id pasado por parámetro y llena la estructura con la información de la 
  dificultad (nombre y si está en uso o no). Si el id no existe, el nombre se inicializa en NULL y el id se cambia 
  a -1. Devuelve “ErrorSimulacion” en caso de error o “ExitoSimulacion” en caso contrario.*/
ResultadoSimulacion obtener_informacion_dificultad(InformacionDificultad* informacion, simulador_t* simulador)
{
    char id_clave[2];
    int resultado;

    if((!simulador) || (!informacion))
        return ErrorSimulacion;
    else if(simulacion_finalizada(simulador->en_ejecucion))
        return ErrorSimulacion;

    resultado = sprintf(id_clave, "%hu", (unsigned short)informacion->id);
    if(!resultado)
    {
        printf("\nError al obtener información sobre la dificultad\n");
        return ErrorSimulacion;
    }

    dificultad_t* dificultad_buscada = hash_obtener(simulador->opciones_dificultades.hash_dificultades, id_clave);

    if (!dificultad_buscada)
    {
        informacion->nombre_dificultad = NULL;
        informacion->id = ERROR;
        return ErrorSimulacion;
    }
    else
    {
        informacion->nombre_dificultad = dificultad_buscada->nombre;
        informacion->en_uso = dificultad_buscada->en_uso;
    } 

    return ExitoSimulacion;
}

/*Pre_condiciones: El simulador no debe ser nulo.
  Post_condiciones: Establece la simulación como finalizada. En caso de que ya se encuentre finalizada imprimirá por
  pantalla un mensaje informándolo. Devuelve “ErrorSimulacion” en caso de error o “ExitoSimulacion” en caso contrario.*/
ResultadoSimulacion finalizar_simulacion(simulador_t* simulador)
{
    if (!simulador)
        return ErrorSimulacion;
    else if(!simulador->en_ejecucion)
    {
        printf("\nEl simulador ya ha finalizado su ejecución\n");
        return ErrorSimulacion;
    }

    simulador->en_ejecucion = false;
    printf("\nSimulación finalizada!\n");
    return ExitoSimulacion;
}

ResultadoSimulacion simulador_simular_evento(simulador_t* simulador, EventoSimulacion evento, void* datos)
{
    ResultadoSimulacion resultado = ExitoSimulacion;

    if(!simulador)
        return ErrorSimulacion;
    
    simulador->estadisticas.cantidad_eventos_simulados++;
        
    switch (evento)
    {
    case ObtenerEstadisticas:
        resultado = obtener_estadisticas(datos, simulador);
        break;
    case AtenderProximoEntrenador:
        resultado = atender_proximo_entrenador(simulador);
        break;
    case ObtenerInformacionPokemonEnTratamiento:
        resultado = obtener_informacion_pokemon_en_tratamiento(datos, simulador);
        break;
    case AdivinarNivelPokemon:
        resultado = adivinar_nivel_pokemon(datos, simulador);
        break;
    case AgregarDificultad:
        resultado = agregar_dificultad(datos, simulador);
        break;            
    case SeleccionarDificultad:
        resultado = seleccionar_dificultad(datos, simulador);
        break;
    case ObtenerInformacionDificultad:
        resultado = obtener_informacion_dificultad(datos, simulador);
        break;
    case FinalizarSimulacion:
        resultado = finalizar_simulacion(simulador);
        break;           
    default:
        simulador->estadisticas.cantidad_eventos_simulados++;
        printf("\nEvento desconocido, intente otra vez.\n");
        resultado = ErrorSimulacion;
        break;
    }

    return resultado;
}