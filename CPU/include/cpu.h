#ifndef CPU_H
#define CPU_H
#include <stdio.h>
#include <commons/log.h>
#include <commons/string.h>
#include <stdbool.h>
#include "shared_utils.h"
#include "tests.h"
#endif

typedef struct {
    int entradas_tlb;
    char* reemplazo_tlb;
    int retardo_instruccion;
    char* ip_memoria;
    char* puerto_memoria;
    char* puerto_escucha_dispatch;
    char* puerto_escucha_interrupt;
}t_cpu_config;

char* fetch(t_pcb*);
t_instruccion* decode(char*);
void cicloInstruccion(t_pcb*);
void *atender_cliente_interrupt();
void *atender_cliente_dispatch(void*);
void *atender_kernel_dispatch(void*);
void *atender_kernel_interrupt();
t_cpu_config* inicializarConfig();