#ifndef SHARED_UTILS_H
#define SHARED_UTILS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <commons/log.h>
#include <commons/config.h>
#include <commons/collections/list.h>
#include <stdbool.h>
//server
#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h>
#include <pthread.h>
#include <ctype.h>

typedef enum op_code{
    INSTR = 1,
    PCB = 2,
    RTA_TECLADO = 3,
    SOL_TECLADO = 4,
    SOL_PANTALLA = 5,
    INTERRUPCION = 6
}op_code;

typedef enum motivo{
    ERROR = -1,
    OK = 1,
    QUANTUM = 2,
    PAGEFAULT = 3,
    IO = 4,
    FINALIZAR = 5
}motivo;

typedef struct {
    uint32_t size; // Tamaño del payload
    void* stream; // Payload
} t_buffer;

typedef struct {
    op_code code;
    t_buffer* buffer;
} t_paquete;


typedef struct {
    uint32_t AX;
    uint32_t BX;
    uint32_t CX;
    uint32_t DX;
}t_registros_cpu; // estructura para registros de CPU, falta definir correctamente

typedef struct t_segmento{
    int idSegmento;
    int tamanioSegmentos;
    int indiceTablaDePaginas;
}t_segmento;// lista de segmentos para la tabla de segmentos

typedef enum estado_proceso {
    NEW = 0,
    READY = 1,
    EXEC = 2,
    BLOCK = 3,
    EXIT = 4
}estado_proceso;// 5 estados que tendran los procesos

typedef enum idProtocolo {
    CONSOLA = 0,
    KERNEL = 1,
    CPU_DISP = 2,
    CPU_INTERRUPT = 3,
    MEMORIA = 4
}idProtocolo;
typedef struct {
    int pid;
    estado_proceso estado;
    t_list* instrucciones; //lista de chars*
    int program_counter;
    t_registros_cpu *registros_cpu;
    t_list* t_segmentos;
}t_pcb; //estructura de PCB, va a tener 2 estructuras dentro

typedef enum tipoParametro {
    NUM,
    REGISTRO,
    IO_P,
    IND
}tipoParametro;

typedef enum refRegistro {
    AX,
    BX,
    CX,
    DX
}refRegistro;

typedef enum refIO {
    TECLADO,
    PANTALLA,
    DISCO,
    IMPRESORA,
    MODEM,
    OTRO
}refIO;

typedef enum respuestaConsola{
    END,
    CONTINUE,
    RESP_TECLADO,
    RESP_PANTALLA
}respuestaConsola;

typedef struct{
    respuestaConsola tipoRespuesta;
    char* mensaje;
}pack_rtaConsola;

typedef struct {
    tipoParametro tipoParam;
    void* valor;
}t_parametro_instr;

typedef enum tipoInstruccion{
    T_SET,
    T_ADD,
    T_MOV_IN,
    T_MOV_OUT,
    T_IO,
    T_EXIT,
    T_IND
}tipoInstruccion;

typedef struct {
    tipoInstruccion tipo;
    t_parametro_instr* param1;
    t_parametro_instr* param2;

    int retardo;
}t_instruccion;

char* mi_funcion_compartida();
char* buscarEnConfig();
void inicializar_logger();

int crear_conexion();
void esperar_cliente();
void serve_client();

void terminar_programa();

int inicializar_servidor();
void *atender_cliente();
void handshakeServidor();

int inicializar_cliente();
int handshakeCliente();

void* serializar_paquete();
void enviar();
void* recibir();
void list_view();

void* serializar_instruccion();

t_list* recibir_instrucciones();

char* borrarElementosCadena();

int isNumber(char*);

void enviarPCB();
t_pcb* deserializarPCB();

void mostrarPCB();

void* serializarPCB_DEVOLVER_conMotivo();

void* serializarPCB_DEVOLVER_porIO();

void* serializarPCB_DEVOLVER_porPAGEFAULT();




#endif