#include "cpu.h"

// Registros
uint32_t registros[] = {0, 0, 0, 0};
// Registros
t_log *logger;
t_pcb *inicializarPCBtest()
{

    //-----PCB de prueba, borrar después----->>

    t_list *testInstr = list_create();
    list_add(testInstr, "SET AX 1");
    list_add(testInstr, "SET BX 2");
    list_add(testInstr, "SET CX 3");
    list_add(testInstr, "SET DX 4");
    list_add(testInstr, "ADD AX BX");
    list_add(testInstr, "ADD AX CX");
    list_add(testInstr, "ADD BX AX");
    // list_add(testInstr, "MOV_IN CX 4");
    // list_add(testInstr, "I/O DISCO 10");
    list_add(testInstr, "EXIT");

    t_pcb *testPCB = malloc(sizeof(t_pcb));

    t_registros_cpu *testRegistros = malloc(sizeof(t_registros_cpu));

    t_segmento *segmento1 = malloc(sizeof(t_segmento));
    segmento1->idSegmento = 1;
    segmento1->indiceTablaDePaginas = 1;
    segmento1->tamanioSegmentos = 32;

    t_segmento *segmento2 = malloc(sizeof(t_segmento));
    segmento2->idSegmento = 2;
    segmento2->indiceTablaDePaginas = 2;
    segmento2->tamanioSegmentos = 16;

    t_list *testTablasSegmentos = list_create();

    list_add(testTablasSegmentos, segmento1);
    list_add(testTablasSegmentos, segmento2);

    testRegistros->AX = 0;
    testRegistros->BX = 0;
    testRegistros->CX = 0;
    testRegistros->DX = 0;

    testPCB->pid = 1;
    testPCB->estado = READY;
    testPCB->instrucciones = testInstr;
    testPCB->program_counter = 0;
    testPCB->registros_cpu = testRegistros;
    testPCB->t_segmentos = testTablasSegmentos;

    //<<-----PCB de prueba, borrar después-----
    return testPCB;
}
int *socket_cliente_dispatch;
t_pcb *pcbActual;

int main(int argc, char **argv)
{
    if (argc > 1 && strcmp(argv[1], "-test") == 0)
        return run_tests();
    else
    {
        logger = log_create("./cfg/cpu.log", "CPU", true, LOG_LEVEL_INFO);
        log_info(logger, "Soy la CPU! %s", mi_funcion_compartida());
        t_pcb *pcbActual = inicializarPCBtest();
        t_cpu_config *cpuConfig = inicializarConfig();

        log_info(logger, "INICIALIZO LA CONFIG");

        int socket_servidor_dispatch = inicializar_servidor(cpuConfig->puerto_escucha_dispatch);
        log_info(logger, "El CPU_DISPATCH tiene socket %d\n", socket_servidor_dispatch);
        //int socket_servidor_interrupt = inicializar_servidor(cpuConfig->puerto_escucha_interrupt);
        //log_info(logger, "El CPU_INTERRUPT tiene socket %d\n", socket_servidor_interrupt);

        log_info(logger, "CREO LOS SOCKETS SERVER");
        // int socket_servidor_memoria = inicializar_servidor(cpuConfig->puerto_memoria);
        // Memoria es servidor del CPU
        pthread_t hiloDispatch;
        pthread_t hiloInterrupt;
        // pthread_t hiloMemoria;

        log_info(logger, "CREO LOS HILOS CONEXION");

        socket_cliente_dispatch = malloc(sizeof(int));
        log_info(logger, "ESPERANDO KERNEL DISPATCH");
        socket_cliente_dispatch = accept(socket_servidor_dispatch, NULL, NULL);
        log_info(logger, "SOCKETS SON: SRV=%d CLI=%d", socket_servidor_dispatch, socket_cliente_dispatch);
        log_info(logger, "ACEPTO CLIENTE");
        handshakeServidor(socket_cliente_dispatch, KERNEL); // Argv es socket_cliente

        pthread_create(&hiloDispatch, // Crea hilo atendedor de Dispatch
                       NULL,
                       (void *)atender_cliente_dispatch,
                       socket_servidor_dispatch);
        pthread_join(hiloDispatch, NULL);

        log_info(logger, "CREO EL HILO ANTENDER DISPATCH");

        /*pthread_create(&hiloInterrupt,  //Crea hilo atendedor de Interrupt
                        NULL,
                        (void *)atender_cliente_interrupt,
                        socket_servidor_interrupt
                        );
        pthread_join(atender_cliente_interrupt, NULL);*/

        // Temporal-->
        // t_pcb* testPCB = inicializarPCBtest();
        //<--Temporal
        // TODO: Función que recibe el PCB del Kernel (Por ahora se usa testPCB)
        //mostrarPCB(pcbActual, logger);

        printf("\nOK\n");
        log_destroy(logger);
        printf("\nOK FINAL\n");
    }
    return 0;
}

t_config *leer_config()
{
    t_config *config;
    config = config_create("./cfg/CPU.config");
    return config;
}

t_cpu_config *inicializarConfig()
{
    log_info(logger, "INICIALIZANDO CONFIG");
    t_config *configLeida = leer_config();
    log_info(logger, "LEO LA CONFIG");
    t_cpu_config *cpuConfig = malloc(sizeof(t_cpu_config));
    cpuConfig->entradas_tlb = buscarEnConfig(configLeida, "ENTRADAS_TLB");
    log_info(logger, "LEO 1");
    cpuConfig->reemplazo_tlb = buscarEnConfig(configLeida, "REEMPLAZO_TLB");
    log_info(logger, "LEO 2");
    cpuConfig->retardo_instruccion = buscarEnConfig(configLeida, "RETARDO_INSTRUCCION");
    log_info(logger, "LEO 3");
    cpuConfig->ip_memoria = buscarEnConfig(configLeida, "IP_MEMORIA");
    log_info(logger, "LEO 4");
    cpuConfig->puerto_memoria = buscarEnConfig(configLeida, "PUERTO_MEMORIA");
    log_info(logger, "LEO 5");
    cpuConfig->puerto_escucha_dispatch = buscarEnConfig(configLeida, "PUERTO_ESCUCHA_DISPATCH");
    log_info(logger, "LEO 6");
    cpuConfig->puerto_escucha_interrupt = buscarEnConfig(configLeida, "PUERTO_ESCUCHA_INTERRUPT");
    log_info(logger, "LEO 7");
    return cpuConfig;
}

void cicloInstruccion(t_pcb *pcb)
{
    t_instruccion *instruccion;
    char *textInstr;
    while (instruccion->tipo != T_EXIT && instruccion->tipo != T_IND)
    {
        printf("\nOK\n");
        textInstr = fetch(pcb);
        printf("\nOK\n");
        instruccion = decode(textInstr);
        if (instruccion->tipo != T_EXIT)
            log_info(logger, "Tipo instr: %d\n Tipo param1: %d\n Valor param1: %d\n Tipo param2: %d\n Valor param2: %d",
                   instruccion->tipo,
                   instruccion->param1->tipoParam, instruccion->param1->valor,
                   instruccion->param2->tipoParam, instruccion->param2->valor);
        else
            printf("----EXIT----");
        execute(instruccion);
        pcb->program_counter++;
    }
    log_info(logger, "FINALIZA CICLO DE INSTRUCCION POR: %d", instruccion->tipo);
}
void executeSET(t_parametro_instr *destino, t_parametro_instr *origen)
{
    if (destino->tipoParam == REGISTRO && origen->tipoParam == NUM)
    {
        registros[(int)destino->valor] = origen->valor;
        printf("\n\nRegistros actuales:\nAX: %d\nBX: %d\nCX: %d\nDX: %d\n", registros[0], registros[1], registros[2], registros[3]);
    }
    else
    {
        // HACER ERROR
    }
}
void executeADD(t_parametro_instr *destino, t_parametro_instr *origen)
{
    if (destino->tipoParam == origen->tipoParam == REGISTRO)
    {
        registros[(int)destino->valor] += registros[(int)origen->valor];
        printf("\n\nRegistros actuales:\nAX: %d\nBX: %d\nCX: %d\nDX: %d\n", registros[0], registros[1], registros[2], registros[3]);
    }
    else
    {
        // HACER ERROR
    }
}
void executeMOVIN() {}
void executeMOVOUT() {}
/*
void executeIO(refIO tipoIO, int info) {
    int size;
    void* buffer = serializarPCB_IO(pcbActual, &size, tipoIO, info);
    send(socket_cliente_dispatch, buffer, size, 0);
}*/
void executeEXIT() {
    int size;
    void* buffer = serializarPCB_DEVOLVER_conMotivo(pcbActual, &size,EXIT);
    send(socket_cliente_dispatch, buffer, size, 0);
}

void execute(t_instruccion *instruccion)
{
    switch (instruccion->tipo)
    {
    case T_SET:
        executeSET(instruccion->param1, instruccion->param2);
        break;
    case T_ADD:
        executeADD(instruccion->param1, instruccion->param2);
        break;
    case T_MOV_IN:
        executeMOVIN(instruccion->param1, instruccion->param2);
        break;
    case T_MOV_OUT:
        executeMOVOUT(instruccion->param1, instruccion->param2);
        break;
    case T_IO:
        //executeIO(instruccion->param1->valor, instruccion->param2->valor);
        break;
    case T_EXIT:
        executeEXIT();
        break;
    }
}

tipoInstruccion deducirTipo(char *parte)
{
    if (string_equals_ignore_case(parte, "set"))
        return T_SET;
    if (string_equals_ignore_case(parte, "add"))
        return T_ADD;
    if (string_equals_ignore_case(parte, "mov_in"))
        return T_MOV_IN;
    if (string_equals_ignore_case(parte, "mov_out"))
        return T_MOV_OUT;
    if (string_equals_ignore_case(parte, "i/o"))
        return T_IO;
    if (string_equals_ignore_case(parte, "exit"))
        return T_EXIT;
    return T_IND;
}

t_parametro_instr *deducirParametro(char *param)
{
    t_parametro_instr *parametro = malloc(sizeof(t_parametro_instr));
    if (isNumber(param))
    {
        parametro->tipoParam = NUM;
        parametro->valor = atoi(param);
        return parametro;
    }
    else
    {
        if (string_equals_ignore_case(param, "pantalla"))
        {
            parametro->tipoParam = IO_P;
            parametro->valor = PANTALLA;
            return parametro;
        }
        if (string_equals_ignore_case(param, "teclado"))
        {
            parametro->tipoParam = IO_P;
            parametro->valor = TECLADO;
            return parametro;
        }

        // RESERVADO PARA OTROS IO

        if (string_equals_ignore_case(param, "ax"))
        {
            parametro->tipoParam = REGISTRO;
            parametro->valor = AX;
            return parametro;
        }

        if (string_equals_ignore_case(param, "bx"))
        {
            parametro->tipoParam = REGISTRO;
            parametro->valor = BX;
            return parametro;
        }

        if (string_equals_ignore_case(param, "cx"))
        {
            parametro->tipoParam = REGISTRO;
            parametro->valor = CX;
            return parametro;
        }

        if (string_equals_ignore_case(param, "dx"))
        {
            parametro->tipoParam = REGISTRO;
            parametro->valor = DX;
            return parametro;
        }

        parametro->tipoParam = IND;
        parametro->valor = 0;
        return parametro;
    }
}

char *fetch(t_pcb *pcb)
{

    char *instr = malloc(50 * sizeof(char));
    instr = list_get(pcb->instrucciones, pcb->program_counter);
    printf("\nInstruccion: %s\n", instr); // BORRAR
    return instr;
}

t_instruccion *decode(char *instr)
{
    t_instruccion *instruccion = malloc(sizeof(t_instruccion));
    printf("\nInstruccion: %s\n", instr); // BORRAR
    if (string_equals_ignore_case(instr, "exit"))
    {
        printf("\nInstruccion ES EXIT\n");
        instruccion->tipo = T_EXIT;
        instruccion->param1 = NULL;
        instruccion->param2 = NULL;
    }
    else
    {
        char **partes = string_split(instr, " ");
        printf("\nAnalizando instruccion: %s\n", instr);
        char *parte = string_array_pop(partes);
        instruccion->param2 = deducirParametro(parte);
        parte = string_array_pop(partes);
        instruccion->param1 = deducirParametro(parte);
        parte = string_array_pop(partes);
        instruccion->tipo = deducirTipo(parte);
        printf("\nInstruccion: %d -> (%d,%d) -> (%d,%d)\n", instruccion->tipo,
               instruccion->param1->tipoParam, instruccion->param1->valor,
               instruccion->param2->tipoParam, instruccion->param2->valor); // BORRAR
    }
    return instruccion;
}

void *atender_cliente_dispatch(void *args)
{
    log_info(logger, "ENTRO A ATENDER DISPATCH");
    int *socket_cliente = (int *)args;

    while (1)
    {
        log_info(logger, "ENTRO AL WHILE (3 SEG)");
        sleep(3); //BORRAR
        log_info(logger, "LISTO 3 SEG");
        //op_code test;
        pcbActual = deserializarPCB(socket_cliente_dispatch);
        log_info(logger, "PID DEL PROCESO: %d", pcbActual->pid);
        log_info(logger, "ENTRO A ATENDER DISPATCH");

        //cicloInstruccion(pcbActual);
        int tamanioDelBuffer;
        pcbActual->pid = 23;
        void* buffer = serializarPCB_DEVOLVER_porIO(pcbActual, &tamanioDelBuffer,"Disco", 10);

        send(socket_cliente_dispatch,buffer,tamanioDelBuffer,0);
        //free(buffer);
    }
    return 0;
}

void *atender_cliente_interrupt(void *args)
{
    int *socket_servidor = (int *)args;
    int *socket_cliente = malloc(sizeof(int));
    *socket_cliente = accept(socket_servidor, NULL, NULL);

    /*while (1)
    {
        pthread_t hiloAtendedor;
        int *socket_cliente = malloc(sizeof(int));
        *socket_cliente = accept(socket_servidor, NULL, NULL);
        pthread_create(&hiloAtendedor, // Crea hilo que crea hilos atendedores de consolas
                       NULL,
                       (void *)atender_kernel_interrupt(),
                       socket_cliente);
        pthread_detach(hiloAtendedor);
    }*/
    return 0;
}

/*void *atender_cliente_memoria(void *args)
{
    int *socket_servidor = (int *)args;

    while (1)
    {
        pthread_t hiloAtendedor;
        int *socket_cliente = malloc(sizeof(int));
        *socket_cliente = accept(socket_servidor, NULL, NULL);
        pthread_create(&hiloAtendedor, // Crea hilo que crea hilos atendedores de consolas
                       NULL,
                       (void *)atender_memoria(),
                       socket_cliente);
        pthread_detach(hiloAtendedor);
    }
    return 0;
}*/

void *atender_kernel_dispatch(void *args)
{
    log_info(logger, "ATIENDO DISPATCH");
    int *socket_cliente = (int *)args;
    handshakeServidor(socket_cliente, KERNEL); // Argv es socket_cliente
    // printf("\nATIENDO DISPATCH\n");
    // log_info(logger, "ATIENDO DISPATCH");
    sleep(10); //BORRAR
    pcbActual = deserializarPCB(socket_cliente);

    cicloInstruccion(pcbActual);

    close(*socket_cliente);
    return 0;
}
