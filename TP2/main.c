#include "src/hospital.h"
#include "src/simulador.h"
#include "src/hash.h"
#include "src/lista.h"
#include "src/heap.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

//Dificultad 
#define PUNTOS_PERFECTOS 3
#define PUNTOS_INTERMEDIOS 2
#define PUNTOS_BASICOS 1
#define PUNTOS_NULOS 0

#define EXITO 0
#define ERROR -1

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
    pokemon_t* paciente;
    InformacionPokemon informacion_paciente;
} paciente_t;

typedef struct lista_dificultades
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

char leer_comando();

///// DIFICULTADES EXTRA ///////////////////////////////////////////////////////////////////////////////////////////////
/*Pre_condiciones:
  Post_condiciones: Recibe el numero adivinado y el correcto y devuelve 0 en caso de acertar 
  (y cualquier otro valor en otro caso)*/
int verificar_nivel_extra(unsigned nivel_adivinado, unsigned nivel_pokemon)
{
    return ((int)(nivel_adivinado - nivel_pokemon));
}

/*Pre_condiciones: La cantidad de intentos debe ser mayor a 0.
  Post_condiciones: Devuelve la cantidad de puntos ganados por acertar dada la cantidad de intentos requeridos 
  en la dificultad "normal plus"*/
unsigned calcular_puntaje_normal_plus(unsigned cantidad_intentos)
{
    if (cantidad_intentos == 1)
        return PUNTOS_PERFECTOS;
    else if (cantidad_intentos <= 20)
        return PUNTOS_BASICOS;
    else
        return PUNTOS_NULOS;
}

/*Pre_condiciones: 
  Post_condiciones: Recibe el numero resultado de la función verificar_nivel y lo transforma en un string que 
  representa el resultado en forma de texto. (dificultad "normal plus")*/
const char* verificacion_a_string_normal_plus(int resultado_verificacion)
{
    if (resultado_verificacion == EXITO)
        return "EXITO";
    else if (resultado_verificacion >= 1)
        return "Mas bajo";
    else
        return "Mas alto";
}

/*Pre_condiciones: La cantidad de intentos debe ser mayor a 0.
  Post_condiciones: Devuelve la cantidad de puntos ganados por acertar dada la cantidad de intentos requeridos 
  en la dificultad "temperatura"*/
unsigned calcular_puntaje_temperatura(unsigned cantidad_intentos)
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

/*Pre_condiciones: 
  Post_condiciones: Recibe el numero resultado de la función verificar_nivel y lo transforma en un string que 
  representa el resultado en forma de texto. (dificultad "temperatura")*/
const char* verificacion_a_string_temperatura(int resultado_verificacion)
{
    if (resultado_verificacion < 0)
        resultado_verificacion *= -1;

    if (resultado_verificacion == EXITO)
        return "EXITO";
    else if (resultado_verificacion >= 30)
        return "Muy frio";
    else if (resultado_verificacion >= 20)
        return "frio";
    else if (resultado_verificacion >= 10)
        return "Tibio";
    else if (resultado_verificacion >= 5)
        return "Caliente";    
    else
        return "Muy caliente";
}
///// FUNCIONES PROPIAS ///////////////////////////////////////////////////////////////////////////////////////////////
/*Pre_condiciones:  El hash no debe ser nulo.
  Post_condiciones: Imprime por pantalla la dificultad asociada a la clave pasada por parámetro. En caso de error devuelve true.*/
bool imprimir_dificultad(hash_t* hash, const char* clave, void* aux)
{
    if(!hash)
        return true;
    dificultad_t* dificultad = (dificultad_t*)hash_obtener(hash, clave);
    printf("\n%s \n", dificultad->nombre);
    printf("ID: %i\n", dificultad->id);
    return false;
}

/*Pre_condiciones: El simulador no debe ser nulo.
  Post_condiciones: Imprime por pantalla todas las dificultades disponibles en el simulador.*/
void lista_dificultades(simulador_t* simulador)
{
    if(!simulador)
        return;
    printf("\nLas dificultades disponibles son: \n");
    printf("------------------------------------------");
    hash_con_cada_clave(simulador->opciones_dificultades.hash_dificultades, imprimir_dificultad, NULL);
    printf("------------------------------------------\n");
}

/*Pre_condiciones: 
  Post_condiciones: Imprime por pantalla un mensaje indicando que se presione cualquier tecla y se lee lo escrito por 
  consola para pausar la ejecución del programa. */
void mensaje_continuar()
{
    printf("\n\nPresione cualquier tecla para continuar: ");
    leer_comando(); 
}

/*Pre_condiciones:
  Post_condiciones: Imprime por pantalla un mensaje indicando que ha ocurrido un error en la ejecución de un evento y 
  se ejecuta la función “mensaje_continuar”.*/
void mensaje_error_simulacion()
{
    printf("\nError al ejecutar el evento\n");
    mensaje_continuar();
}

/*Pre_condiciones:
  Post_condiciones: En caso de que el simulador haya terminado su ejecución imprime por pantalla un mensaje indicando 
  esto y devuelve true, en caso contrario solo devuelve false.*/
bool menu_simulacion_finalizada(bool estado)//Para esos eventos que requieren previamente una entrada del usuario por consola, se notifica que la simulación finalizo sin tenes que ingresar algo previamente.
{
    if (!estado)
    {
        printf("\nLa simulacion ha finalizado, no es posible ejecutar mas eventos.\n");
        return true;
    }
    
    return false;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
char leer_comando(){
    char linea[100];
    char* leido;
    leido = fgets(linea, 100, stdin);
    if(!leido)
        return 0;
    while(*leido == ' ')
        leido++;
    return (char)tolower(*leido);
}

/*Pre_condiciones: El simulador no debe ser nulo.
  Post_condiciones: Imprime por pantalla las estadísticas del simulador.*/
void evento_mostrar_estadisticas(simulador_t* simulador)
{
    if(!simulador)
        return;

    system("clear");
    EstadisticasSimulacion estadisticas;
    ResultadoSimulacion resultado = simulador_simular_evento(simulador, ObtenerEstadisticas, &estadisticas);
    if(resultado == ErrorSimulacion)
    {
        mensaje_error_simulacion();
        return;
    }
    printf("\n------------------------------------------\n");
    printf("Cantidad de eventos simulador: %i\n", estadisticas.cantidad_eventos_simulados);
    printf("Cantidad de puntos obtenidos: %i\n", estadisticas.puntos);
    printf("Cantidad de entrenadores: %i\n", estadisticas.entrenadores_totales);
    printf("Cantidad de entrenadores atendidos: %i\n", estadisticas.entrenadores_atendidos);
    printf("Cantidad de pokemones: %i\n", estadisticas.pokemon_totales);
    printf("Cantidad de pokemones atendidos: %i\n", estadisticas.pokemon_atendidos);
    printf("Cantidad de pokemones en espera: %i\n", estadisticas.pokemon_en_espera);
    printf("------------------------------------------\n\n");
    printf("Presione cualquier tecla para continuar: ");
    leer_comando();
}

/*Pre_condiciones: El simulador no debe ser nulo.
  Post_condiciones: Ejecuta el evento AtenderProximoEntrenador, por lo cual se atiende al próximo entrenador y sus 
  pokemones quedan en estado de espera. */
void evento_atender_proximo_entrenador(simulador_t* simulador)
{
    if(!simulador)
        return;

    system("clear");
    ResultadoSimulacion resultado = simulador_simular_evento(simulador, AtenderProximoEntrenador, NULL);
    if(resultado == ErrorSimulacion)
        mensaje_error_simulacion();
    else
        mensaje_continuar();
}

/*Pre_condiciones: El simulador no debe ser nulo.
  Post_condiciones: Imprime por pantalla la el nombre del pokemon que esta siendo atendido y el nombre de su 
  entrenador.*/
void evento_informacion_pokemon_en_tratamiento(simulador_t* simulador)
{
    if(!simulador)
        return;

    system("clear");
    InformacionPokemon informacion_paciente;
    ResultadoSimulacion resultado = simulador_simular_evento(simulador, ObtenerInformacionPokemonEnTratamiento, &informacion_paciente);
    if(resultado == ErrorSimulacion)
    {
        mensaje_error_simulacion();
        return;
    }
    printf("\n------------------------------------------\n");
    printf("\nNombre Pokemon: %s\n", informacion_paciente.nombre_pokemon);
    printf("Nombre Entrenador: %s\n", informacion_paciente.nombre_entrenador);
    printf("\n------------------------------------------\n");
    printf("Presione cualquier tecla para continuar: ");
    leer_comando();
}

/*Pre_condiciones: El simulador no debe ser nulo.
  Post_condiciones: Permite al usuario ingresar un nivel por consola para ejecutar el evento “AdivinarNivelPokemon” e 
  indica si el nivel es correcto con un mensaje. En caso contrario se imprime por pantalla un mensaje dependiendo 
  la dificultad actual del simulador. */
void evento_adivinar_nivel_pokemon(simulador_t* simulador)
{
    if(!simulador)
        return;

    ResultadoSimulacion resultado;
    Intento intento;
    char comando;
    char linea[100];
    bool ejecucion = true;

    system("clear");
    if (menu_simulacion_finalizada(simulador->en_ejecucion))
    {
        mensaje_continuar();
        return;
    }
    else if ((!simulador->consultorio.paciente_actual) && (heap_cantidad(simulador->consultorio.pacientes_espera) == 0))
    {
        printf("\nNo quedan mas pacientes en espera. Se debe atender a un nuevo entrenador\n");
        mensaje_continuar();
        return;
    }

        while(ejecucion)
        {
            printf("\nInserte el nivel del pokemon: \n");
            fgets(linea, 100, stdin);
            intento.nivel_adivinado = (unsigned)atoi(linea);
            resultado = simulador_simular_evento(simulador, AdivinarNivelPokemon, &intento);
            if (resultado == ErrorSimulacion)
            {
                mensaje_error_simulacion();
                ejecucion = false;
            }
            else if (intento.es_correcto)
            {
                printf("\nNivel Adivinado!\n");
                ejecucion = false;
                mensaje_continuar();
            }
            else
            {
                printf("\n%s\n", intento.resultado_string);
                printf("\n¿Desea volver a intentarlo? ('S': Si) ('Otro': No)\n");
                printf("Comando: ");
                comando = leer_comando();
                if(comando != 's')
                    ejecucion = false;
            }
        }  
}

/*Pre_condiciones: El simulador no debe ser nulo.
  Post_condiciones: Cambia la dificultad actual del simulador dependiendo de la ID ingresada por consola por el usuario.*/
void evento_seleccionar_dificultad(simulador_t* simulador)
{
    if(!simulador)
        return;

    ResultadoSimulacion resultado;
    bool ejecucion = true;
    char comando;
    char id;
    int id_numero = 0;

    system("clear");
    if (menu_simulacion_finalizada(simulador->en_ejecucion))
    {
        mensaje_continuar();
        return;
    }

    while (ejecucion)
    {
        system("clear");
        lista_dificultades(simulador);
        printf("\nSeleccione la dificultad deseada: \nComando: ");
        id = leer_comando();
        id_numero = id - '0';
        resultado = simulador_simular_evento(simulador, SeleccionarDificultad, &id_numero);

        if(resultado == ErrorSimulacion)
        {
            mensaje_error_simulacion();
            ejecucion = false;
        }
        else
        {
            printf("\nDificultad cambiada a: %s\n", simulador->opciones_dificultades.dificultad_actual->nombre);
            printf("\n¿Desea volver a cambiar la dificultad? ('S': Si) ('Otro': No)\n");
            printf("Comando: ");
            comando = leer_comando();
            if(comando != 's')
                ejecucion = false;
        }
    }
}

/*Pre_condiciones: El simulador no debe ser nulo.
  Post_condiciones: Indica que dificultad se encuentra en uso actualmente por el simulador.*/
void evento_informacion_dificultad(simulador_t* simulador)
{
    if(!simulador)
        return;

    bool ejecucion = true;
    char comando;
    char id;
    int id_numero = 0;
    InformacionDificultad info_dificultad;

    system("clear");
    if (menu_simulacion_finalizada(simulador->en_ejecucion))
    {
        mensaje_continuar();
        return;
    }

    while (ejecucion)
    {
        system("clear");
        ResultadoSimulacion resultado;
        lista_dificultades(simulador);
        printf("\nIngrese el ID de la dificultad deseada: ");
        id = leer_comando();
        id_numero = id - '0';
        info_dificultad.id = id_numero;

        resultado = simulador_simular_evento(simulador, ObtenerInformacionDificultad, &info_dificultad);

        if((resultado == ErrorSimulacion) && (info_dificultad.id != ERROR))
        {
            mensaje_error_simulacion();
            ejecucion = false;
        }
        else
        {
            if(info_dificultad.id != ERROR)
            {
                printf("------------------------------------------");
                printf("\nDificultad: %s\n", info_dificultad.nombre_dificultad);
                printf("ID: %i\n", info_dificultad.id);
                printf("%s\n", info_dificultad.en_uso ? "Se encuentra en uso" : "No se encuentra en uso");
                printf("------------------------------------------\n\n");
            }
            else
                printf("\nNo existe una dificultad con ese ID\n");
            
            printf("¿Desea consultar la información de otra dificultad? ('S': Si) ('Otro': No)\n");
            printf("Comando: ");
            comando = leer_comando();
            if(comando != 's')
                ejecucion = false;
        }
    } 
}

/*Pre_condiciones: El simulador no debe ser nulo.
  Post_condiciones: Ejecuta el evento “FinalizarSimulacion”, por lo cual se finaliza la ejecución del simulador.*/
void evento_finalizar_simulacion(simulador_t* simulador)
{
    if(!simulador)
        return;

    ResultadoSimulacion resultado;

    system("clear");
    resultado = simulador_simular_evento(simulador, FinalizarSimulacion, NULL);

    if (resultado == ErrorSimulacion)
    {
        mensaje_error_simulacion();
    }
    mensaje_continuar(); 
}

/*Pre_condiciones: 
  Post_condiciones: Imprime por pantalla un mensaje indicando que se ingreso un comando incorrecto.*/
void comando_invalido()
{
    printf("\nComando incorrecto ingresado por consola, intente otra vez.\n\n");
    printf("Presione cualquier tecla para continuar: ");
    leer_comando();
}

/*Pre_condiciones: El simulador no debe ser nulo.
  Post_condiciones: Ejecuta una función especifica dependiendo el comando pasado por parámetro, en caso de que el 
  comando sea invalido se imprime un mensaje de error.*/
void ejecutar_comando(simulador_t* simulador, char comando)
{
    if(!simulador)
        return;

    switch (comando) {
        case 's'://Salir del simulador
            break;
        case 'q': 
            evento_finalizar_simulacion(simulador);
            break;
        case 'e':
            evento_mostrar_estadisticas(simulador);
            break;
        case 'p':
            evento_atender_proximo_entrenador(simulador);
            break;
        case 'i':
            evento_informacion_pokemon_en_tratamiento(simulador);
            break;
        case 'a':
            evento_adivinar_nivel_pokemon(simulador);
            break;
        case 'd':
            evento_seleccionar_dificultad(simulador);
            break;
        case 'o':
            evento_informacion_dificultad(simulador);
            break;                
        default: //el resto de las cosas
            comando_invalido();
            break;
    }
}

/*Pre_condiciones: El simulador no debe ser nulo.
  Post_condiciones: Imprime por pantalla un menú indicando todas las posibles operaciones a realizar en el simulador 
  y permite al usuario ingresar un comando por consola indicando la opción deseada. */
void menu(simulador_t* simulador)
{
    if(!simulador)
        return;
        
    char comando = ' ';

    while (comando != 's')
    {
        system("clear"); 
        printf("-----------------------------------------------------------------");
        printf("\n|\t   Descripción del menu                          \t|\n");
        printf("|\t                                                 \t|");
        printf("\n|\t A. Adivinar nivel de pokemones                  \t|");
        printf("\n|\t P. Atender Próximo entrenador                   \t|");
        printf("\n|\t I. Obtener información de pokemon en tratamiento\t|");
        printf("\n|\t E. Obtener estadísticas                         \t|");        
        printf("\n|\t D. Seleccionar dificultad                       \t|");
        printf("\n|\t O. Obtener información de la dificultad         \t|");
        printf("\n|\t Q. Finalizar Simulacion                         \t|");
        printf("\n|\t S. Salir                                        \t|");
        printf("\n-----------------------------------------------------------------\n\n");
        printf("Comando: ");
        comando = leer_comando();
        ejecutar_comando(simulador, comando);
    }
}

int main(int argc, char *argv[])
{
    hospital_t* hospital = hospital_crear();
    hospital_leer_archivo(hospital, "ejemplos/varios_entrenadores.hospital");

    simulador_t* simulador = simulador_crear(hospital);

    DatosDificultad dificultad_normal_plus;
    DatosDificultad dificultad_temperatura;

    dificultad_normal_plus.nombre = "Normal Plus";
    dificultad_normal_plus.calcular_puntaje = calcular_puntaje_normal_plus;
    dificultad_normal_plus.verificacion_a_string = verificacion_a_string_normal_plus;
    dificultad_normal_plus.verificar_nivel = verificar_nivel_extra;

    dificultad_temperatura.nombre = "Temperatura";
    dificultad_temperatura.calcular_puntaje = calcular_puntaje_temperatura;
    dificultad_temperatura.verificacion_a_string = verificacion_a_string_temperatura;
    dificultad_temperatura.verificar_nivel = verificar_nivel_extra;

    
    simulador_simular_evento(simulador, AgregarDificultad, &dificultad_temperatura);
    simulador_simular_evento(simulador, AgregarDificultad, &dificultad_normal_plus);

    menu(simulador);
    simulador_destruir(simulador);

    return 0;
}
