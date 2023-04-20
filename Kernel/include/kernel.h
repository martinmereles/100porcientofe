#ifndef KERNEL_H
#define KERNEL_H
#include <stdio.h>
#include <commons/log.h>
#include <commons/temporal.h>
#include <commons/config.h>
#include <commons/collections/queue.h>
#include <stdbool.h>
#include <pthread.h>
#include <semaphore.h>
#include "shared_utils.h"
#include "tests.h"



// -----------------------ESTRUCTURAS INICIALES ---------------------------

enum algoritmo {
    FIFO,
    RR,
    Feedback
};

//------------------------ESTRUCTURAS DEL PLANIFICACION -------------------------

typedef struct t_kernel_config {
    char* ip_memoria;
    char* puerto_memoria;
    char* ip_cpu;
    char* puerto_cpu_dispatch;
    char* puerto_cpu_interrupt;
    char* puerto_escucha;
    char* algoritmo_planificacion;
    int grado_max_multiprogramacion;
    char* dispositivos_io;
    char* tiempos_io;
    int quantum_rr;
}t_kernel_config;

typedef struct t_dispositivo {
    char* dispositivo;
    int duracionPorUnidad;
}t_dispositivo;


t_pcb *inicializarPCBtest();

//PROTOTIPOS DE FUNCIONES
t_kernel_config* inicializarConfig();
void* inicializar_semaforos();
void* inicializar_colas_procesos();
t_config* leer_config();

void* iniciar_planificadores();
void inicializar_conexiones();

void* planificadorLargoPlazo();
bool evaluarProcesoNEW();
bool evaluarGradoDeMultiprogramacion();
int procesosEnMemoria();
void sumarCantidadDeProcesosEnMemoria();
void restarCantidadDeProcesosEnMemoria();


t_pcb* crearPCB();
void* loggearLaCreacionDelPCB();
void* agregar_PCB_NEW_Queue();
t_pcb* sacar_PCB_NEW_Queue();
void* agregar_PCB_READY_List();
t_pcb* sacar_PCB_READY_List();
t_pcb* sacar_PCB_READY_List_FIFO_Multinivel();
void* agregar_PCB_EXEC_Queue();
t_pcb* sacar_PCB_EXEC_Queue();



void* cambiarEstado();

void* planificadorCortoPlazoFIFO();
void* planificadorCortoPlazoRR();
void* planificadorCortoPlazoFeedback();
<<<<<<< HEAD


void* contarQuantum();
void enviarInterrupcion();

void* ejecutarProceso();


=======
void* hiloContadorQuantum();

void* recibirPCB_Dispatch();
void  enviarInterrupcion();

bool revisarColaDeREADY();
void* ejecutarProceso();

void* actualizarPCB();
>>>>>>> d3dc53a385faab65a2d50890681e9480c8d360e1

void destruirColas();
void destruirLoggers();
#endif
/**/

