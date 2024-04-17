#include "pa2mm.h"
#include "src/hospital.h"
#include "src/simulador.h"
#include "src/hospital.h"
#include "src/hash.h"
#include "src/lista.h"
#include "src/heap.h"

#include "string.h"
#include <stdbool.h>

//Dificultades
#define ID_FACIL 0
#define ID_NORMAL 1
#define ID_DIFICIL 2
#define DIFICULTADES_ESTANDAR 3

#define PUNTOS_PERFECTOS 3
#define PUNTOS_BASICOS 1
#define PUNTOS_NULOS 0

bool ignorar_pokemon(pokemon_t* p){
    p = p;
    return true;
}

/* No intenten esto en sus casas */
/* Ya vamos a ver como evitar esto en el TDA Lista */
struct{
    pokemon_t* pokemon[500];
    size_t cantidad;
} acumulados;

void resetear_acumulados(){
    acumulados.cantidad = 0;
}

bool acumular_pokemon(pokemon_t* p){
    acumulados.pokemon[acumulados.cantidad] = p;
    acumulados.cantidad++;
    return true;
}

bool acumular_pokemon_hasta_miltank(pokemon_t* p){
    acumulados.pokemon[acumulados.cantidad] = p;
    acumulados.cantidad++;
    return strcmp(pokemon_nombre(p), "miltank");
}

bool acumulados_en_orden_correcto(){
    if(acumulados.cantidad < 2)
        return true;
    pokemon_t* anterior = acumulados.pokemon[0];
    for(int i=1;i<acumulados.cantidad;i++){
        pokemon_t* actual =  acumulados.pokemon[i];
        if(strcmp(pokemon_nombre(anterior), pokemon_nombre(actual)) > 0)
            return false;
    }
    return true;
}

typedef struct paciente
{
    pokemon_t* pokemon;
    char* nombre_entrenador;
} paciente_t;

typedef struct dificultad
{
    char* nombre;
    int id;
    bool en_uso;
    unsigned (*calcular_puntaje)(unsigned cantidad_intentos);
    int (*verificar_nivel)(unsigned nivel_adivinado, unsigned nivel_pokemon);
    const char* (*verificacion_a_string)(int resultado_verificacion);
} dificultad_t;

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

/// Pruebas Hospital /////////////////////////////////////////////////////////////////////////////////////////////////////
void puedoCrearYDestruirUnHospital(){
    hospital_t* h=NULL;

    pa2m_afirmar((h=hospital_crear()), "Crear un hospital devuelve un hospital");

    pa2m_afirmar(hospital_cantidad_entrenadores(h)==0, "Un hospital se crea con cero entrenadores");
    pa2m_afirmar(hospital_cantidad_pokemon(h)==0, "Un hospital se crea con cero pokemon");

    pa2m_afirmar(hospital_a_cada_pokemon(h, ignorar_pokemon)==0, "Recorrer los pokemon resulta en 0 pokemon recorridos");

    hospital_destruir(h);
}

void dadoUnHospitalNULL_lasPuedoAplicarLasOperacionesDelHospitalSinProblema(){
    hospital_t* h=NULL;

    pa2m_afirmar(hospital_cantidad_entrenadores(h)==0, "Un hospital NULL tiene cero entrenadores");
    pa2m_afirmar(hospital_cantidad_pokemon(h)==0, "Un hospital NULL tiene cero pokemon");

    pa2m_afirmar(hospital_a_cada_pokemon(h, ignorar_pokemon)==0, "Recorrer los pokemon de un hospital NULL resulta en 0 pokemon recorridos");

    hospital_destruir(h);
}

void dadoUnArchivoVacio_NoSeAgreganPokemonAlHospital(){
    hospital_t* h=hospital_crear();

    pa2m_afirmar(hospital_leer_archivo(h, "ejemplos/archivo_vacio.hospital"), "Puedo leer un archivo vacío");

    pa2m_afirmar(hospital_cantidad_entrenadores(h)==0, "Un hospital vacío tiene cero entrenadores");
    pa2m_afirmar(hospital_cantidad_pokemon(h)==0, "Un hospital vacío tiene tiene cero pokemon");

    pa2m_afirmar(hospital_a_cada_pokemon(h, ignorar_pokemon)==0, "Recorrer los pokemon resulta en 0 pokemon recorridos");

    hospital_destruir(h);
}

void dadoUnArchivoConUnEntrenador_SeAgregaElEntrenadorYSusPokemonAlHospital(){
    hospital_t* h=hospital_crear();

    pa2m_afirmar(hospital_leer_archivo(h, "ejemplos/un_entrenador.hospital"), "Puedo leer un archivo con un entrenador");

    pa2m_afirmar(hospital_cantidad_entrenadores(h)==1, "El hospital tiene 1 entrenador");
    pa2m_afirmar(hospital_cantidad_pokemon(h)==3, "El hospital tiene 3 pokemon");

    resetear_acumulados();
    pa2m_afirmar(hospital_a_cada_pokemon(h, acumular_pokemon)==3, "Recorrer los pokemon resulta en 3 pokemon recorridos");
    pa2m_afirmar(acumulados_en_orden_correcto(), "Los pokemon se recorrieron en orden alfabetico");

    hospital_destruir(h);
}

void dadoUnArchivoConVariosEntrenadores_SeAgreganLosEntrenadoresYSusPokemonAlHospital(){
    hospital_t* h=hospital_crear();

    pa2m_afirmar(hospital_leer_archivo(h, "ejemplos/varios_entrenadores.hospital"), "Puedo leer un archivo con varios entrenadores");

    pa2m_afirmar(hospital_cantidad_entrenadores(h)==5, "El hospital tiene 5 entrenadores");
    pa2m_afirmar(hospital_cantidad_pokemon(h)==24, "El hospital tiene 24 pokemon");

    resetear_acumulados();
    pa2m_afirmar(hospital_a_cada_pokemon(h, acumular_pokemon)==24, "Recorrer los pokemon resulta en 24 pokemon recorridos");
    pa2m_afirmar(acumulados_en_orden_correcto(), "Los pokemon se recorrieron en orden alfabetico");

    hospital_destruir(h);
}


void dadosVariosArchivos_puedoAgregarlosTodosAlMismoHospital(){
    hospital_t* h=hospital_crear();

    pa2m_afirmar(hospital_leer_archivo(h, "ejemplos/varios_entrenadores.hospital"), "Puedo leer un archivo con varios entrenadores");
    pa2m_afirmar(hospital_leer_archivo(h, "ejemplos/varios_entrenadores.hospital"), "Puedo leer otro archivo con varios entrenadores");
    pa2m_afirmar(hospital_leer_archivo(h, "ejemplos/varios_entrenadores.hospital"), "Puedo leer un tercer archivo con varios entrenadores");

    pa2m_afirmar(hospital_cantidad_entrenadores(h)==15, "El hospital tiene 15 entrenadores");
    pa2m_afirmar(hospital_cantidad_pokemon(h)==72, "El hospital tiene 72 pokemon");

    resetear_acumulados();
    pa2m_afirmar(hospital_a_cada_pokemon(h, acumular_pokemon)==72, "Recorrer los pokemon resulta en 72 pokemon recorridos");
    pa2m_afirmar(acumulados_en_orden_correcto(), "Los pokemon se recorrieron en orden alfabetico");

    hospital_destruir(h);
}

/// Dificultad prueba ////////////////////////////////////////////////////////////////////////////////////////////////////
int verificar_nivel_prueba(unsigned nivel_adivinado, unsigned nivel_pokemon)
{
    return ((int)(nivel_adivinado - nivel_pokemon));
}

unsigned calcular_puntaje_prueba(unsigned cantidad_intentos)
{
    if (cantidad_intentos == 1)
        return PUNTOS_PERFECTOS;
    else if (cantidad_intentos <= 10)
        return PUNTOS_BASICOS;
    else
        return PUNTOS_NULOS;
}

const char* verificacion_a_string_prueba(int resultado_verificacion)
{
    if (resultado_verificacion == 0)
        return "EXITO";
    else if (resultado_verificacion >= 1)
        return "Mas bajo";
    else
        return "Mas alto";
}

/// Pruebas simulador////////////////////////////////////////////////////////////////////////////////////////////////////
void prueba_simulador_vacio()
{
    ResultadoSimulacion resultado;
    hospital_t* hospital = hospital_crear();
    pa2m_afirmar(hospital_leer_archivo(hospital, "ejemplos/archivo_vacio.hospital"), "Puedo leer un archivo con varios entrenadores");

    simulador_t* simulador = simulador_crear(hospital);

    pa2m_afirmar(simulador, "Se crea el simulador con exito");
    pa2m_afirmar(hash_cantidad(simulador->opciones_dificultades.hash_dificultades) == 3, "La cantidad de dificultades inicialmente son 3"); 
    
    //Estadisticas
    EstadisticasSimulacion estadisticas;
    resultado = simulador_simular_evento(simulador, ObtenerEstadisticas, &estadisticas);
    pa2m_afirmar(resultado == ExitoSimulacion, "Obtener estadísticas devuelve exito simulación");
    pa2m_afirmar(estadisticas.cantidad_eventos_simulados == 1, "Se simulo 1 evento");
    pa2m_afirmar(estadisticas.entrenadores_atendidos == 0 , "Se atendieron 0 entrenadores");
    pa2m_afirmar(estadisticas.entrenadores_totales == 0, "La cantidad de entrenadores totales es 0");
    pa2m_afirmar(estadisticas.pokemon_atendidos == 0, "Se atendieron 0 pokemones");
    pa2m_afirmar(estadisticas.pokemon_en_espera == 0, "La cantidad de pokemones en espera es 0");
    pa2m_afirmar(estadisticas.pokemon_totales == 0, "La cantidad de pokemones totales es 0");
    pa2m_afirmar(estadisticas.puntos == 0, "La cantidad de puntos es 0");
    pa2m_afirmar(simulador_simular_evento(simulador, AtenderProximoEntrenador, NULL) == ErrorSimulacion, "No se puede atender a un nuevo entrenador");
    
    //Pokemon
    InformacionPokemon informacion_pokemon;
    pa2m_afirmar(simulador_simular_evento(simulador, ObtenerInformacionPokemonEnTratamiento, &informacion_pokemon) == ErrorSimulacion, "No se puede obtener información del pokemon en tratamiento");
    
    //Dificultades
    InformacionDificultad info_dificultad;
    info_dificultad.id = 0;
    resultado = simulador_simular_evento(simulador, ObtenerInformacionDificultad, &info_dificultad);
    pa2m_afirmar(resultado == ExitoSimulacion, "Se puede obtener la información de la dificultad con id 0");

    info_dificultad.id = 1;
    resultado = simulador_simular_evento(simulador, ObtenerInformacionDificultad, &info_dificultad);
    pa2m_afirmar(resultado == ExitoSimulacion, "Se puede obtener la información de la dificultad con id 1");

    info_dificultad.id = 2;
    resultado = simulador_simular_evento(simulador, ObtenerInformacionDificultad, &info_dificultad);
    pa2m_afirmar(resultado == ExitoSimulacion, "Se puede obtener la información de la dificultad con id 2");

    info_dificultad.id = 3;
    resultado = simulador_simular_evento(simulador, ObtenerInformacionDificultad, &info_dificultad);
    pa2m_afirmar(resultado == ErrorSimulacion, "Obtener información de una dificultad inexistente da error");

    int id = 3;
    pa2m_afirmar(simulador_simular_evento(simulador, SeleccionarDificultad, &id) == ErrorSimulacion, "No se puede seleccionar una dificultad inexistente");
    id = 0;
    pa2m_afirmar(simulador_simular_evento(simulador, SeleccionarDificultad, &id) == ExitoSimulacion, "Se puede seleccionar una dificultad existente");
    info_dificultad.id = 0;
    simulador_simular_evento(simulador, ObtenerInformacionDificultad, &info_dificultad);
    pa2m_afirmar(info_dificultad.en_uso, "La dificultad seleccionada se encuentra en uso");
    info_dificultad.id = 2;
    simulador_simular_evento(simulador, ObtenerInformacionDificultad, &info_dificultad);
    pa2m_afirmar(!info_dificultad.en_uso, "La dificultad no seleccionada no se encuentra en uso");
    pa2m_afirmar(simulador_simular_evento(simulador, ObtenerEstadisticas, &estadisticas) == ExitoSimulacion, "Puedo volver a obtener estadisticas");
    pa2m_afirmar(estadisticas.cantidad_eventos_simulados == 12, "La cantidad de eventos simulados es 12");

    DatosDificultad dificultad;
    dificultad.nombre = "Prueba";
    dificultad.calcular_puntaje = calcular_puntaje_prueba;
    dificultad.verificacion_a_string = verificacion_a_string_prueba;
    dificultad.verificar_nivel = verificar_nivel_prueba;
    id = 3;

    InformacionDificultad informacion;
    informacion.id = id;

    pa2m_afirmar(simulador_simular_evento(simulador, AgregarDificultad, &dificultad) == ExitoSimulacion, "Puedo agregar una nueva dificultad");
    pa2m_afirmar(simulador_simular_evento(simulador, SeleccionarDificultad, &id) == ExitoSimulacion, "Puedo seleccionar la nueva dificultad");
    pa2m_afirmar(simulador_simular_evento(simulador, ObtenerInformacionDificultad, &informacion) == ExitoSimulacion, "Puedo obtener los datos de la nueva dificultad");
    pa2m_afirmar(informacion.en_uso == true, "La nueva dificultad se encuentra en uso");
    pa2m_afirmar(strcmp(informacion.nombre_dificultad,  "Prueba") == 0, "La nueva dificultad tiene el nombre correcto");
    

    //Finalizar simulacion
    pa2m_afirmar(simulador_simular_evento(simulador, FinalizarSimulacion, NULL) == ExitoSimulacion, "Se puede finalizar la simulación");
    pa2m_afirmar(simulador_simular_evento(simulador, FinalizarSimulacion, NULL) == ErrorSimulacion, "Finalizarla otra vez devuelve error");

    simulador_destruir(simulador);
}

void prueba_simulador_con_entrenador()
{
    hospital_t* hospital = hospital_crear();
    pa2m_afirmar(hospital_leer_archivo(hospital, "ejemplos/un_entrenador.hospital"), "Puedo leer un archivo con varios entrenadores");

    simulador_t* simulador = simulador_crear(hospital);

    pa2m_afirmar(simulador, "Se crea el simulador con exito");

    pa2m_afirmar(simulador->estadisticas.entrenadores_totales == hospital_cantidad_entrenadores(simulador->hospital), "La estadistica sobre la cantidad de entrenadores coincide con la del hospital");
    pa2m_afirmar(simulador->estadisticas.pokemon_totales == hospital_cantidad_pokemon(simulador->hospital), "La estadistica sobre la cantidad de pokemones coincide con la del hospital");
    pa2m_afirmar(simulador_simular_evento(simulador, AtenderProximoEntrenador, NULL) == ExitoSimulacion, "Se puede atender un nuevo entrenador con exito");
    
    Intento intento;
    intento.nivel_adivinado = 41;
    pa2m_afirmar(simulador_simular_evento(simulador, AdivinarNivelPokemon, &intento) == ExitoSimulacion, "Se puede adivinar el nivel de un pokemon");
    pa2m_afirmar(!intento.es_correcto, "El nivel no es el correcto");
    pa2m_afirmar(strcmp(intento.resultado_string, "Muy cerca") == 0, "El resultado en string es correcto");
    intento.nivel_adivinado = 39;
    pa2m_afirmar(simulador_simular_evento(simulador, AdivinarNivelPokemon, &intento) == ExitoSimulacion, "Se puede adivinar el nivel de un pokemon");
    pa2m_afirmar(!intento.es_correcto, "El nivel no es el correcto");
    pa2m_afirmar(strcmp(intento.resultado_string, "Muy cerca") == 0, "El resultado en string es correcto");
    intento.nivel_adivinado = 40;
    pa2m_afirmar(simulador_simular_evento(simulador, AdivinarNivelPokemon, &intento) == ExitoSimulacion, "Se puede adivinar el nivel de un pokemon");
    pa2m_afirmar(intento.es_correcto == true, "El nivel es el correcto");
    pa2m_afirmar(strcmp(intento.resultado_string, "EXITO") == 0, "El resultado en string es correcto");
    simulador_destruir(simulador);
}

int main(){

    pa2m_nuevo_grupo("Pruebas de  creación y destrucción");
    puedoCrearYDestruirUnHospital();

    pa2m_nuevo_grupo("Pruebas con NULL");
    dadoUnHospitalNULL_lasPuedoAplicarLasOperacionesDelHospitalSinProblema();

    pa2m_nuevo_grupo("Pruebas con un archivo vacío");
    dadoUnArchivoVacio_NoSeAgreganPokemonAlHospital();

    pa2m_nuevo_grupo("Pruebas con un archivo de un entrenador");
    dadoUnArchivoConUnEntrenador_SeAgregaElEntrenadorYSusPokemonAlHospital();

    pa2m_nuevo_grupo("Pruebas con un archivo de varios entrenadores");
    dadoUnArchivoConVariosEntrenadores_SeAgreganLosEntrenadoresYSusPokemonAlHospital();

    pa2m_nuevo_grupo("Pruebas con mas de un archivo");
    dadosVariosArchivos_puedoAgregarlosTodosAlMismoHospital();

    pa2m_nuevo_grupo("Pruebas de simulador vacio");
    prueba_simulador_vacio();

    pa2m_nuevo_grupo("Pruebas de creacion de simulador");
    prueba_simulador_con_entrenador();

    return pa2m_mostrar_reporte();
}
