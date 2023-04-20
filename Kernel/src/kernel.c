#include "kernel.h"


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


t_queue* colaNEW;
t_list*  colaREADY;
<<<<<<< HEAD
t_list*  colaREADYFIFOMULTINIVEL;
=======
t_list* colaREADY_FIFO_MULTINIVEL;
>>>>>>> d3dc53a385faab65a2d50890681e9480c8d360e1
t_queue* colaEXEC;
t_queue* colaBLOCKED;
t_queue* colaSUSP;
t_queue* colaEXIT;

t_log *logger;
t_log *loggerPlanificadores;


t_kernel_config* kernelConfig;
t_list* dispositivos;

int contadorIDProcesos;
int contadorProcesosEnMemoria;
//Hilos
pthread_t hiloConsolas;
pthread_t hiloLargoPlazo;
pthread_t hiloCortoPlazo;
pthread_t hiloCPUdispatch;
pthread_t hiloCPUinterrupt;
pthread_t hiloMemoria;
pthread_t hiloContadorDeQuantum;

//Sockets
int socket_servidor;
int* socket_dispatch;
int socket_interrupt;
int socket_memoria;

/*semaforos*/
pthread_mutex_t mutexNumeroProceso;
pthread_mutex_t mutexProcesoListo;

pthread_mutex_t mutexColaNEW;// para modificar cola new
pthread_mutex_t mutexColaREADY;// para modificar cola ready
<<<<<<< HEAD
pthread_mutex_t mutexColaREADYFIFOMULTINIVEL;// para modificar cola ready cuando hay FIFO Multinivel
=======
pthread_mutex_t mutexColaREADY_FIFO_MULTINIVEL;// para modificar cola ready
>>>>>>> d3dc53a385faab65a2d50890681e9480c8d360e1
pthread_mutex_t mutexColaBLOCKED;// para modificar cola block
pthread_mutex_t mutexColaEXEC;// para modificar cola exec
pthread_mutex_t mutexColaEXIT;// para modificar cola exit
pthread_mutex_t mutexColaSUSP;// para modificar cola suspendido

pthread_mutex_t mutex_cola;
pthread_mutex_t mutexCantidadProcesosMemoria;

sem_t semaforoProcesoNEW;
sem_t semaforoProcesoREADY;
sem_t semaforoProcesoEXECUTING;

sem_t semaforoComienzoDeQuantum;
sem_t semaforoFinDeQuantum;

sem_t contadorBloqueados;

sem_t analizarSuspension;
sem_t suspensionFinalizada;
sem_t despertarPlanificadorLargoPlazo;

sem_t semaforoCantidadProcesosEjecutando;
sem_t comunicarMemoria;

int main(int argc, char **argv)
{
    if (argc > 1 && strcmp(argv[1], "-test") == 0)
        return run_tests();
    else
    {   
        logger = log_create("./cfg/kernel.log", "KERNEL", true, LOG_LEVEL_INFO);
        loggerPlanificadores = log_create("./cfg/kernelPlanificadores.log", "KERNELPLANIFICADORES", true, LOG_LEVEL_INFO);


        kernelConfig = inicializarConfig();
        inicializar_colas_procesos();
        inicializar_semaforos();
        log_info(logger, "inicialice los semaforos"); //pruebo la funcion de inicializar los semaforos

        

        log_info(logger, "Soy el Kernel y termine la config! %s", mi_funcion_compartida());

        // Creamos el server
        socket_servidor = inicializar_servidor(kernelConfig->puerto_escucha);
        log_info(logger, "El kernel tiene socket %d\n", socket_servidor);

        inicializar_conexiones();
        log_info(loggerPlanificadores, "Antes de iniciar planificadores");

        /*op_code test = PCB;
        
        send(socket_dispatch, &test, sizeof(op_code), 0);*/
        
        iniciar_planificadores();
        
        log_info(loggerPlanificadores, "SAli de iniciar planificadores");


        pthread_create(&hiloConsolas,  //Crea hilo que crea hilos atendedores de consolas
                        NULL,
                        (void *)atender_cliente,
                        socket_servidor
                        );
        pthread_join(hiloConsolas, NULL);

        destruirLoggers();
    }
}

t_kernel_config* inicializarConfig(){
    t_config* configLeida = leer_config();
    t_kernel_config* kernelConfig = malloc(sizeof(t_kernel_config));

    kernelConfig->ip_memoria = buscarEnConfig(configLeida, "IP_MEMORIA");
    kernelConfig->puerto_memoria = buscarEnConfig(configLeida, "PUERTO_MEMORIA");
    kernelConfig->ip_cpu = buscarEnConfig(configLeida, "IP_CPU");
    kernelConfig->puerto_cpu_dispatch = buscarEnConfig(configLeida, "PUERTO_CPU_DISPATCH");
    kernelConfig->puerto_cpu_interrupt = buscarEnConfig(configLeida, "PUERTO_CPU_INTERRUPT");
    kernelConfig->puerto_escucha = buscarEnConfig(configLeida, "PUERTO_ESCUCHA");
    kernelConfig->algoritmo_planificacion = buscarEnConfig(configLeida, "ALGORITMO_PLANIFICACION");
    kernelConfig->grado_max_multiprogramacion = atoi(buscarEnConfig(configLeida, "GRADO_MAX_MULTIPROGRAMACION"));
    kernelConfig->dispositivos_io = buscarEnConfig(configLeida, "DISPOSITIVOS_IO");
    kernelConfig->tiempos_io = buscarEnConfig(configLeida, "TIEMPOS_IO");
    kernelConfig->quantum_rr = atoi(buscarEnConfig(configLeida, "QUANTUM_RR"));

    return kernelConfig;
}

t_config *leer_config()
{
    t_config *config;
    config = config_create("./cfg/kernel.config");
    return config;
}

void* inicializar_semaforos() //funcion que inicializa todos los semaforos
{
    pthread_mutex_init(&mutexNumeroProceso, NULL);

    pthread_mutex_init(&mutexColaNEW, NULL);
    pthread_mutex_init(&mutexColaREADY, NULL);
    pthread_mutex_init(&mutexColaREADYFIFOMULTINIVEL, NULL);
    pthread_mutex_init(&mutexColaEXEC, NULL);
    pthread_mutex_init(&mutexColaBLOCKED, NULL);
    pthread_mutex_init(&mutexColaSUSP, NULL);
    pthread_mutex_init(&mutexColaEXIT, NULL);

    sem_init(&semaforoProcesoNEW, 0, 0);
    sem_init(&semaforoProcesoREADY, 0, 0);

    sem_init(&contadorBloqueados, 0, 0);

    sem_init(&analizarSuspension, 0, 0);
    sem_init(&suspensionFinalizada, 0, 0);

    sem_init(&semaforoProcesoEXECUTING, 0, 1);

    sem_init(&semaforoComienzoDeQuantum, 0, 0);
    sem_init(&semaforoFinDeQuantum, 0, 0);


    sem_init(&despertarPlanificadorLargoPlazo, 0, 0);

    sem_init(&comunicarMemoria, 0, 1);
    return 0;
}

void *inicializar_colas_procesos(){
    colaNEW = queue_create();
    colaREADY = list_create();
    colaREADYFIFOMULTINIVEL = list_create();
    colaEXEC = queue_create();
    colaBLOCKED = queue_create();
    colaSUSP = queue_create();
    colaEXIT = queue_create();
    return 0;
}

void* iniciar_planificadores(){

    pthread_create(&hiloLargoPlazo,
                    NULL,
                    (void *)planificadorLargoPlazo,
                    NULL
                    );
    pthread_detach(hiloLargoPlazo);
   
    if( strcmp(kernelConfig->algoritmo_planificacion, "FIFO") == 0){
            pthread_create(&hiloCortoPlazo,NULL,(void *)planificadorCortoPlazoFIFO,NULL);
            pthread_detach(hiloCortoPlazo);
        }
        
    if(strcmp(kernelConfig->algoritmo_planificacion, "RR") == 0){
            pthread_create(&hiloCortoPlazo,NULL,(void *)planificadorCortoPlazoRR,NULL);
            pthread_detach(hiloCortoPlazo);
        }
            

    if(strcmp(kernelConfig->algoritmo_planificacion, "Feedback") == 0){
            pthread_create(&hiloCortoPlazo,NULL,(void *)planificadorCortoPlazoFeedback,NULL);
            pthread_detach(hiloCortoPlazo);
        }
    
    if(strcmp(kernelConfig->algoritmo_planificacion, "Feedback") == 0 && strcmp(kernelConfig->algoritmo_planificacion, "RR") == 0 && strcmp(kernelConfig->algoritmo_planificacion, "FIFO") == 0){
        log_info(loggerPlanificadores, "ERROR AL ELEGIR ALGORITMO DE PLANIFICACION");
    }

    pthread_create(&hiloContadorDeQuantum,NULL,(void *)hiloContadorQuantum,NULL);
    pthread_detach(hiloContadorDeQuantum);

//falta el de dispositivo IO
    return 0;
}

void inicializar_conexiones(){
    socket_dispatch = inicializar_cliente(kernelConfig->ip_cpu,kernelConfig->puerto_cpu_dispatch,logger);
    socket_interrupt = inicializar_cliente(kernelConfig->ip_cpu,kernelConfig->puerto_cpu_interrupt,logger);
    socket_memoria = inicializar_cliente(kernelConfig->ip_memoria,kernelConfig->puerto_memoria,logger);
    
    int resultHandshakeDispatch = handshakeCliente(socket_dispatch,KERNEL);
    //int resultHandshakeInterrupt = handshakeCliente(socket_interrupt,KERNEL);
    //int resultHandshakeMemoria = handshakeCliente(socket_memoria,KERNEL);

    log_info(logger, "El handshake con el Dispatch fue: %d\n", resultHandshakeDispatch);
    //log_info(logger, "El handshake con el Interrupt fue: %d\n", resultHandshakeInterrupt);
    //log_info(logger, "El handshake con la Memoria fue: %d\n", resultHandshakeMemoria);

}


void *recibir(int socket_cliente)
{
    op_code operacion;
    recv(socket_cliente, &operacion, sizeof(operacion), 0);
    int buffer_size;
    recv(socket_cliente, &(buffer_size), sizeof(buffer_size), 0);
    char *buffer = malloc(buffer_size);
    recv(socket_cliente, buffer, buffer_size, 0);
    printf("buffer_size: %d\nop_code: %d\nbuffer: %s\n", buffer_size, operacion, buffer);
    return buffer;
}

t_list *recibir_instrucciones(int socket_cliente)
{

    log_info(logger, "--RECIBIENDO INSTRUCCIONES--");

    int cant;
    recv(socket_cliente, &cant, sizeof(int), 0);

    log_info(logger, "---Cantidad de Instrucciones: %d--\n", cant);

    op_code operacion;
    recv(socket_cliente, &operacion, sizeof(operacion), 0);

    log_info(logger, "---Op_code: %d--\n", operacion);

    t_list *instr = list_create();

    for (int i = 0; i < cant; i++)
    {
        int buffer_size;
        recv(socket_cliente, &buffer_size, sizeof(buffer_size), 0);

        log_info(logger, "---Longitud:: %d", buffer_size);

        char *buffer = malloc(buffer_size);
        recv(socket_cliente, buffer, buffer_size, 0);

        log_info(logger, "---Instruccion: %s", buffer);

        list_add(instr, buffer);
    }
    return instr;
}





t_pcb* crearPCB(t_list* listaDeInstrucciones,t_list* tablaSegmentos){
    t_pcb *pcb = malloc(sizeof(t_pcb));

    
    pthread_mutex_lock(&mutexNumeroProceso);
    pcb->pid = contadorIDProcesos++;
    pthread_mutex_unlock(&mutexNumeroProceso);
    pcb->estado = NEW;
    pcb->instrucciones = list_duplicate(listaDeInstrucciones);
    pcb->program_counter = 0;
    pcb->registros_cpu = malloc(sizeof(t_registros_cpu));
    pcb->t_segmentos = list_duplicate(tablaSegmentos);

    list_destroy(listaDeInstrucciones);
    list_destroy(tablaSegmentos);
    
    return pcb;
    }

void* loggearLaCreacionDelPCB(t_pcb* pcb_a_loggear){
    log_info(logger, "Se crea el proceso <%d> en NEW(%d)", pcb_a_loggear->pid, pcb_a_loggear->estado);//borrar 
    return 0;
}

void* agregar_PCB_NEW_Queue(t_pcb* NewPCB){

    pthread_mutex_lock(&mutexColaNEW);
    
    queue_push(colaNEW,NewPCB);
    log_info(logger, "Agregado a colaNEW ID: %d", NewPCB->pid);//borrar 
    pthread_mutex_unlock(&mutexColaNEW);
    
    sem_post(&despertarPlanificadorLargoPlazo);//DespertamosAlLargoPlazo

    return 0;
    }

t_pcb *sacar_PCB_NEW_Queue(){ 

    pthread_mutex_lock(&mutexColaNEW);
    t_pcb *pcbRemovido = queue_pop(colaNEW);
    log_info(loggerPlanificadores, "Salido de colaNEW ID: %d", pcbRemovido->pid);//borrar 
    pthread_mutex_unlock(&mutexColaNEW);

    return pcbRemovido;
    }

void* agregar_PCB_READY_List(t_pcb* pcb_a_agregar){

    pthread_mutex_lock(&mutexColaREADY);
    list_add(colaREADY,pcb_a_agregar);
    pthread_mutex_unlock(&mutexColaREADY);
    log_info(loggerPlanificadores, "Agregado a colaREADY ID: %d", pcb_a_agregar->pid);//borrar
    sem_post(&semaforoProcesoREADY);

    sumarCantidadDeProcesosEnMemoria();
    return 0;
    }

t_pcb* sacar_PCB_READY_List(){ 

    pthread_mutex_lock(&mutexColaREADY);
    int ultimaPosicionLista = (list_size(colaREADY)-1);
    t_pcb* pcbRemovido = list_get(colaREADY,ultimaPosicionLista);
    list_remove(colaREADY,ultimaPosicionLista);
    pthread_mutex_unlock(&mutexColaREADY);

    log_info(loggerPlanificadores, "Salido de colaREADY ID: %d", pcbRemovido->pid);//borrar 

    return pcbRemovido;
    }

<<<<<<< HEAD
t_pcb* sacar_PCB_READY_List_FIFO_Multinivel(){
    pthread_mutex_lock(&mutexColaREADYFIFOMULTINIVEL);
    int ultimaPosicionLista = (list_size(colaREADYFIFOMULTINIVEL)-1);
    t_pcb* pcbRemovido = list_get(colaREADYFIFOMULTINIVEL,ultimaPosicionLista);
    list_remove(colaREADYFIFOMULTINIVEL,ultimaPosicionLista);
    pthread_mutex_unlock(&mutexColaREADYFIFOMULTINIVEL);

    log_info(loggerPlanificadores, "Salido de colaREADY FIFO en MULTINIVEL ID: %d", pcbRemovido->pid);//borrar 
=======
t_pcb* sacar_PCB_READY_List_FIFO_Multinivel(){ 

    pthread_mutex_lock(&mutexColaREADY_FIFO_MULTINIVEL);
    int ultimaPosicionLista = (list_size(colaREADY_FIFO_MULTINIVEL)-1);
    t_pcb* pcbRemovido = list_get(colaREADY_FIFO_MULTINIVEL,ultimaPosicionLista);
    list_remove(colaREADY_FIFO_MULTINIVEL,ultimaPosicionLista);
    pthread_mutex_unlock(&mutexColaREADY_FIFO_MULTINIVEL);

    log_info(loggerPlanificadores, "Salido de colaREADY FIFO MULTINIVEL ID: %d", pcbRemovido->pid);//borrar 
>>>>>>> d3dc53a385faab65a2d50890681e9480c8d360e1
    return pcbRemovido;
    }

void* agregar_PCB_EXEC_Queue(t_pcb* pcb_a_ejecutar){ 
<<<<<<< HEAD
        pthread_mutex_lock(&mutexColaEXEC);
        queue_push(colaEXEC, pcb_a_ejecutar);
        pthread_mutex_unlock(&mutexColaEXEC);
        log_info(loggerPlanificadores, "Agregado a colaEXEC ID: %d", pcb_a_ejecutar->pid);//borrar 
=======
    pthread_mutex_lock(&mutexColaEXEC);
    queue_push(colaEXEC,pcb_a_ejecutar);
    pthread_mutex_unlock(&mutexColaEXEC);
    
    log_info(loggerPlanificadores, "Entrado EXEC QUEUE ID: %d", pcb_a_ejecutar->pid);//borrar 

>>>>>>> d3dc53a385faab65a2d50890681e9480c8d360e1
    return 0;
    }

t_pcb* sacar_PCB_EXEC_Queue(){
    pthread_mutex_lock(&mutexColaEXEC);
    t_pcb* proceso_ya_ejecutado = queue_pop(colaEXEC);
    pthread_mutex_unlock(&mutexColaEXEC);

    log_info(loggerPlanificadores, "Salido EXEC QUEUE ID: %d", proceso_ya_ejecutado->pid);//borrar 
    return proceso_ya_ejecutado;
}

void* planificadorLargoPlazo(){

    log_info(loggerPlanificadores, "Iniciando planificador a largo plazo con: %s", kernelConfig->algoritmo_planificacion);

    while(1){
        sem_wait(&despertarPlanificadorLargoPlazo);

        if(evaluarGradoDeMultiprogramacion())
        {   
            
            //meter pcb a ready
            t_pcb* pcb_a_agregar = sacar_PCB_NEW_Queue();

            agregar_PCB_READY_List(pcb_a_agregar);//remueve el PCB de NEW y lo mete en Ready
            cambiarEstado(pcb_a_agregar,NEW,READY);
            
            log_info(loggerPlanificadores, "Cantidad de procesos en READY: %d", list_size(colaREADY));
            log_info(loggerPlanificadores, "Cantidad grado de multiprogramacion: %d", kernelConfig->grado_max_multiprogramacion);
            log_info(loggerPlanificadores, "Cantidad de procesos en memoria: %d", contadorProcesosEnMemoria);

            
            //log_info(loggerPlanificadores, "ID: %d", pcbRemovido->pid);
            //log_info(loggerPlanificadores, "Cantidad de procesos en memoria: %d", contadorProcesosEnMemoria);
            //sem_post(&comunicarMemoria); //comunicamos a la memoria que puede enviar el pcb a
        }
    }

    return 0;
}

bool evaluarProcesoNEW(t_pcb* pcb){
    return pcb->estado == NEW;
}

bool evaluarGradoDeMultiprogramacion(){
    return (procesosEnMemoria() < (kernelConfig->grado_max_multiprogramacion)); //Y cantidad de procesos en Nuevo
}

//Funciones para monitorear
int procesosEnMemoria(){
    //usar 2 semaforos para checkear la cantidad de procesos en memoria. Variable global
    int cantidadDeProcesos;
    pthread_mutex_lock(&mutexCantidadProcesosMemoria);
    cantidadDeProcesos = contadorProcesosEnMemoria;
    pthread_mutex_unlock(&mutexCantidadProcesosMemoria);
    return cantidadDeProcesos;
}

void sumarCantidadDeProcesosEnMemoria(){

    pthread_mutex_lock(&mutexCantidadProcesosMemoria);
    contadorProcesosEnMemoria++;
    pthread_mutex_unlock(&mutexCantidadProcesosMemoria);

}

void restarCantidadDeProcesosEnMemoria(){

    pthread_mutex_lock(&mutexCantidadProcesosMemoria);
    contadorProcesosEnMemoria--;
    pthread_mutex_unlock(&mutexCantidadProcesosMemoria);

}

//mostrar



void* cambiarEstado(t_pcb* pcb, estado_proceso estadoAnerior, estado_proceso estadoNuevo){
        pcb->estado = estadoNuevo;
        log_info(loggerPlanificadores,"PID: <%d> - Estado Anterior: <%d> - Estado Actual: <%d>",pcb->pid ,estadoAnerior,pcb->estado);
    return 0;
}

void* planificadorCortoPlazoFIFO(){
<<<<<<< HEAD
    
    log_info(loggerPlanificadores, "Inicio de planificador corto plazo FIFO");
=======
    log_info(loggerPlanificadores,"INICIALIZANDO CORTO PLAZO FIFO");

>>>>>>> d3dc53a385faab65a2d50890681e9480c8d360e1
    while(1){
    sem_wait(&semaforoProcesoREADY);
    sem_wait(&semaforoProcesoEXECUTING);

    t_pcb* proceso_a_ejecutar = sacar_PCB_READY_List();
<<<<<<< HEAD
    cambiarEstado(proceso_a_ejecutar,READY,EXEC);
=======
    agregar_PCB_EXEC_Queue(proceso_a_ejecutar);
    cambiarEstado(proceso_a_ejecutar,READY,EXEC);
    
>>>>>>> d3dc53a385faab65a2d50890681e9480c8d360e1

    ejecutarProceso(proceso_a_ejecutar);
    }
    return 0;
}

void* planificadorCortoPlazoRR(){
<<<<<<< HEAD
    log_info(loggerPlanificadores, "Inicio de planificador corto plazo RR");
=======
    
    log_info(loggerPlanificadores,"INICIALIZANDO CORTO PLAZO RR");
>>>>>>> d3dc53a385faab65a2d50890681e9480c8d360e1
    while(1){
    sem_wait(&semaforoProcesoREADY);
    sem_wait(&semaforoProcesoEXECUTING);

    t_pcb* proceso_a_ejecutar = sacar_PCB_READY_List();
<<<<<<< HEAD
=======
    agregar_PCB_EXEC_Queue(proceso_a_ejecutar);

>>>>>>> d3dc53a385faab65a2d50890681e9480c8d360e1
    cambiarEstado(proceso_a_ejecutar,READY,EXEC);

    ejecutarProceso(proceso_a_ejecutar);

    recibirPCB_Dispatch();

    sem_post(&semaforoComienzoDeQuantum);

    }
    return 0;
}

<<<<<<< HEAD

void* planificadorCortoPlazoFeedback(){
    log_info(loggerPlanificadores, "Inicio de planificador corto plazo FEEDBACK");
    
    while(1){
        sem_wait(&semaforoProcesoREADY);
        sem_wait(&semaforoProcesoEXECUTING);

        t_pcb* proceso_a_ejecutar;
        if(list_is_empty(colaREADY))//Si la lista de prioridad 1 esta vacia
            { 
            //Ejecuto de la lista 2 (FIFO)
            proceso_a_ejecutar = sacar_PCB_READY_List_FIFO_Multinivel();
            cambiarEstado(proceso_a_ejecutar,READY,EXEC);

            ejecutarProceso(proceso_a_ejecutar);
            }
        else
            {
            //Ejecuto de la lista 1 (RR)
            proceso_a_ejecutar = sacar_PCB_READY_List();
            cambiarEstado(proceso_a_ejecutar,READY,EXEC);

            ejecutarProceso(proceso_a_ejecutar);

            contarQuantum();
            sem_wait(&semaforoFinDeQuantum);
            enviarInterrupcion();
        }


    }
    return 0;


    void* contarQuantum(){
        sleep(kernelConfig->quantum_rr);
        sem_post(&semaforoFinDeQuantum);
    return 0;
    }

    void enviarInterrupcion(){
        send(socket_interrupt, INTERRUPCION, sizeof(INTERRUPCION), 0);
    }


}

void* ejecutarProceso(t_pcb* procesoEjecutado){
        
        int* tamanioDelMensaje;
        void* buffer = serializarPCB(procesoEjecutado,tamanioDelMensaje);
        
        enviarPCB(buffer,tamanioDelMensaje,socket_dispatch,loggerPlanificadores);


=======
void* planificadorCortoPlazoFeedback(){
    
    log_info(loggerPlanificadores,"INICIALIZANDO CORTO PLAZO FEEDBACK");

    while(1){
    sem_wait(&semaforoProcesoREADY);
    sem_wait(&semaforoProcesoEXECUTING);

        if(revisarColaDeREADY()){

            t_pcb* proceso_a_ejecutar = sacar_PCB_READY_List_FIFO_Multinivel();
            agregar_PCB_EXEC_Queue(proceso_a_ejecutar);
            cambiarEstado(proceso_a_ejecutar,READY,EXEC);

            ejecutarProceso(proceso_a_ejecutar);
        }
        else
        {
            t_pcb* proceso_a_ejecutar = sacar_PCB_READY_List();
            agregar_PCB_EXEC_Queue(proceso_a_ejecutar);
            cambiarEstado(proceso_a_ejecutar,READY,EXEC);

            ejecutarProceso(proceso_a_ejecutar);

            sem_post(&semaforoComienzoDeQuantum);
            sem_wait(&semaforoFinDeQuantum);

            
        }


    return 0;
    }
}

void* hiloContadorQuantum(){
    log_info(loggerPlanificadores, "SE CREA EL CONTADOR DE QUANTUM");
    while(1){
    sem_wait(&semaforoComienzoDeQuantum);
    sleep((kernelConfig->quantum_rr)/1000); //Dividimos por el tiempo en milisegundos
    //enviarInterrupcion();
    }
    
    return 0;
}


bool revisarColaDeREADY(){
    bool value;
    pthread_mutex_lock(&mutexColaREADY);
    value = list_is_empty(colaREADY);
    pthread_mutex_unlock(&mutexColaREADY);

    return value;
}

void simularIO(refIO tipoIO, int tiempo){
    //log_info("SIMULANDO ESPERA DEL IO %d CON PARAMETRO %d", tipoIO, tiempo);
}

void* recibirPCB_Dispatch(){ //primero recibir el PCB y despues ver el motivo.
    
    t_pcb* pcb_salido_exit = sacar_PCB_EXEC_Queue();
 
    t_pcb* pcb_recibido_devuelta = deserializarPCB(socket_dispatch);
    

    actualizarPCB(pcb_salido_exit,pcb_recibido_devuelta);

    mostrarPCB(pcb_salido_exit,loggerPlanificadores);
    
    log_info(loggerPlanificadores,"Process status %d", pcb_recibido_devuelta->estado);

    motivo motivoDeRecibimiento;
    recv(socket_dispatch, &motivoDeRecibimiento, sizeof(motivo),MSG_WAITALL);
    log_info(loggerPlanificadores,"DevueltoConMotivo: %d", motivoDeRecibimiento);
    
    if(motivoDeRecibimiento==QUANTUM){
        

    }
    if(motivoDeRecibimiento==IO){
        
        int largoDelDispositivo;
        int cantidad_a_ejecutar;

        recv(socket_dispatch, &largoDelDispositivo, sizeof(int),MSG_WAITALL);
        log_info(loggerPlanificadores,"largo del dispositivo: %d", largoDelDispositivo);

        char* dispositivo = malloc(largoDelDispositivo);

        recv(socket_dispatch, dispositivo, largoDelDispositivo,MSG_WAITALL);
        log_info(loggerPlanificadores,"Dispositivo: %s", dispositivo);

        recv(socket_dispatch, &cantidad_a_ejecutar, sizeof(int),MSG_WAITALL);
        log_info(loggerPlanificadores,"Cant a ejecutar: %d", cantidad_a_ejecutar);

        //if(cosas)
    }

    if(motivoDeRecibimiento==PAGEFAULT){
        int paginaReclamada;
        recv(socket_dispatch, &paginaReclamada, sizeof(int),MSG_WAITALL);
        log_info(loggerPlanificadores,"La pagina reclamada es: %d", paginaReclamada);

        //avisar a memoria
    }

    if(motivoDeRecibimiento==FINALIZAR){
        //borrar el pcb y avisarle a la consola de este pcb
    }
    sleep(200);
    
    sem_post(&semaforoProcesoEXECUTING);

    return 0;
}

void* actualizarPCB(t_pcb* pcb_a_actualizar,t_pcb* pcb_actualizado){//le paso el pcb que quiero actualizar y el que recibi del socket
    log_info(loggerPlanificadores,"Actualizando el pcb"); //Borrar

    pcb_a_actualizar->pid = pcb_actualizado->pid;

    pcb_a_actualizar->estado = pcb_actualizado->estado;

    pcb_a_actualizar->instrucciones = list_duplicate(pcb_actualizado->instrucciones);
    
    pcb_a_actualizar->program_counter = pcb_actualizado->program_counter;

    
    t_registros_cpu* registros = malloc(sizeof(t_registros_cpu));

    registros->AX = pcb_actualizado->registros_cpu->AX;
    registros->BX = pcb_actualizado->registros_cpu->BX;
    registros->CX = pcb_actualizado->registros_cpu->CX;
    registros->DX = pcb_actualizado->registros_cpu->DX;

    pcb_a_actualizar->registros_cpu = registros;

    pcb_a_actualizar->t_segmentos = list_duplicate(pcb_actualizado->t_segmentos);
    log_info(loggerPlanificadores,"Fin Actualizando el pcb"); //Borrar
    return 0;
}

void enviarInterrupcion(){
        send(socket_interrupt,INTERRUPCION,sizeof(op_code),0);
}

void* ejecutarProceso(t_pcb* procesoEjecutado){
    log_info(logger, "EJECUTAR PROCESO");
    mostrarPCB(procesoEjecutado, logger);
    serializarPCB(procesoEjecutado, socket_dispatch);
    log_info(logger, "SOCKET ES: %d", socket_dispatch);
    //enviarPCB(buffer,tamanioDelMensaje,socket_dispatch,loggerPlanificadores);

    log_info(logger, "ENVIADO");
>>>>>>> d3dc53a385faab65a2d50890681e9480c8d360e1
    return 0;
}

void destruirColas(){

    queue_destroy(colaNEW);
    list_destroy(colaREADY);
    queue_destroy(colaEXEC);
    queue_destroy(colaBLOCKED);
    queue_destroy(colaSUSP);
    queue_destroy(colaEXIT);
}

void destruirLoggers(){
    log_destroy(logger);
    log_destroy(loggerPlanificadores);
}
