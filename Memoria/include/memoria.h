//avance martin

/*
#ifndef MEMORIA_H
#define MEMORIA_H
#include <stdio.h>
#include <commons/log.h>
#include <stdbool.h>
#include <stdlib.h>

#include <commons/collections/list.h>
#include <commons/temporal.h>
#include <commons/config.h>
#include <commons/collections/queue.h>
#include <stdbool.h>
#include <pthread.h>
#include <semaphore.h>
#include "shared_utils.h"
#endif

// Variables globales
pthread_t hiloModulos;
t_list *procesos;
t_list *marcos;
void *memoriaPrincipal;
int contadorPageFaults;
int contadorAccesosADisco;
pthread_mutex_t semaforoProcesos;



// estructura config memoria
typedef struct t_memoria_config{
    char* puerto_escucha;
    int tam_memoria;
    int tam_pagina;
    int entradas_por_tabla;
    int retardo_memoria;
    char* algoritmo_reemplazo;
    int marcos_por_proceso;
    int retardo_swap;
    char* path_swap;
    int tamanio_swap;
}t_memoria_config;

//estructura de tabla de pagina
/*typedef struct t_marco {
    int nro_marco;
    int pagina;
    int bit_uso;
    int bit_modificado;
    int pos_swap;
}t_marco;*/

/* //renombro estructura de lista para identificar mejor
typedef struct t_tablas_segmentos{
    t_list* marcos;
} t_tablas_segmentos;
*/
//funcion para llenar los marcos a una tabla de segmentos
//void agregar_marcos (t_marco* marco, t_tablas_segmentos* tabla);

typedef struct 
{

    int numeroPagina;
    bool paginaVacia;
    bool uso;
    bool modificado;
    int NromarcoAsignado; //NULL = presencia 0
}Pagina;

typedef struct
{
    int idProceso;
    int numeroMarco;
    //Pagina PaginaActual; // NULL = marco vacio// ver diseño
} Marco;


typedef struct
{
    int idProceso;
    int tamanio; //TODO: Ver donde usar este campo, es necesario?
   // TablaPrimerNivel *tablaPrimerNivel;
    FILE *archivoSwap;
    t_list *paginasAsignadas;
    int posicionDelPunteroDeSustitucion;
} Proceso;

// funciones
t_config* leer_config();
t_memoria_config* inicializarConfig();

*/


//Avance MArtin