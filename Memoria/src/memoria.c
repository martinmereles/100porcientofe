#include "memoria.h"





t_config* leer_config(){
    t_config *config;
    config = config_create("./cfg/memoria.config");
    return config;
}

t_memoria_config* inicializarConfig(){
    t_config* configLeida = leer_config();
    t_memoria_config* memoriaConfig = malloc(sizeof(t_memoria_config));

    memoriaConfig->puerto_escucha = buscarEnConfig(configLeida, "PUERTO_ESCUCHA");
    memoriaConfig->tam_memoria = atoi (buscarEnConfig(configLeida, "TAM_MEMORIA"));
    memoriaConfig->tam_pagina = atoi (buscarEnConfig(configLeida, "TAM_PAGINA"));
    memoriaConfig->entradas_por_tabla = (buscarEnConfig(configLeida, "ENTRADAS_POR_TABLA"));
    memoriaConfig->retardo_memoria = atoi (buscarEnConfig(configLeida, "RETARDO_MEMORIA"));
    memoriaConfig->algoritmo_reemplazo = buscarEnConfig(configLeida, "ALGORITMO_REEMPLAZO");
    memoriaConfig->marcos_por_proceso = atoi (buscarEnConfig(configLeida, "MARCOS_POR_PROCESO"));
    memoriaConfig->retardo_swap = atoi (buscarEnConfig(configLeida, "RETARDO_SWAP"));
    memoriaConfig->path_swap = buscarEnConfig(configLeida, "PATH_SWAP");
    memoriaConfig->tamanio_swap = atoi (buscarEnConfig(configLeida, "TAMANIO_SWAP"));

    return memoriaConfig;

//avance Martin
/*
void iniciar_estructuras_memoria()
{
  contadorPageFaults = 0;
  contadorAccesosADisco = 0;

  memoriaPrincipal = (void *)malloc(memoriaConfig->tam_memoria);
  memset(memoriaPrincipal, '0', memoriaConfig->tam_memoria);

  iniciar_marcos();
  procesos = list_create();

  //log_destroy(logger);// borrar/modificar

}
void iniciar_marcos()
{
  int tamanio_memoria = memoriaConfig->tam_memoria;
  int tamanio_pagina = memoriaConfig->tam_pagina;

  int cantidadMarcos = tamanio_memoria / tamanio_pagina;
  marcos = list_create();
  for (int numeroDeMarco = 0; numeroDeMarco < cantidadMarcos; numeroDeMarco++)
  {
    Marco *marco = malloc(sizeof(Marco));
    marco->numeroMarco = numeroDeMarco;
    marco->idProceso = -1;
  //  marco->paginaActual = NULL; //borrar/modificar
    list_add(marcos, marco);
  }
}
/*Proceso *crear_proceso(int id, int tamanio, t_memoria_config config, t_log* logger)
{
 
// t_log *logger_proceso = log_create ("./cfg/logger_proceso", "LOGGER_PROCESO",true, LOG_LEVEL_INFO);
  Proceso *proceso = malloc(sizeof(Proceso));
  proceso->idProceso = id;
  proceso->tamanio = tamanio;
  //proceso->tablaPrimerNivel = crear_tabla_primer_nivel();// borrar/modificar
  proceso->paginasAsignadas = list_create();
  proceso->posicionDelPunteroDeSustitucion = 0;

  log_info(logger, "Se creo el Proceso: %d, tamanio: %d", proceso->idProceso, proceso->tamanio);

  proceso->archivoSwap = crear_archivo_swap(logger, config);
  agregar_proceso_a_lista_de_procesos(procesos, proceso);

  //log_destroy(logger_proceso);// borrar/modificar

  return proceso;
}
*/
void agregar_proceso_a_lista_de_procesos(Proceso *proceso, t_memoria_config memoria_config)
{
  pthread_mutex_lock(&semaforoProcesos);
  list_add(procesos, proceso);
  pthread_mutex_unlock(&semaforoProcesos);
  char *generar_path_archivo_swap(int idProceso)
{
  return string_from_format("%s%s%d%s", memoria_config.path_swap, "/", idProceso, ".swap");
}
}
}
/*FILE *crear_archivo_swap(t_log* loggermemoria, t_memoria_config* config)
{
 //Logger *logger = iniciar_logger_memoria();

  char *pathArchivoSwap = config.path_swap;

  log_info(loggermemoria, "Se creo el archivo de swap del proceso en el path %s", pathArchivoSwap);
  //log_destroy(logger);//borrar modificar

  FILE *ficheroArchivoSwap = fopen(pathArchivoSwap, "w+");
  free(pathArchivoSwap);

  return ficheroArchivoSwap;
}*/


/*void agregar_marco (t_marco* marco_recibido, t_tablas_segmentos* tabla_recibida){
    t_list* tabla = tabla_recibida;
    t_marco* marco = marco_recibido; 
    list_add(tabla, marco);
    return tabla;
}; borrar*/
void *atender_modulo(void *args)
{
    int *socket_cliente = (int *)args;
    handshakeServidor(*socket_cliente, KERNEL); // Argv es socket_cliente
    // log_info(logger,"Lei las instrucciones correctamente!\n");

   /* t_list *listaInstrucciones = list_create();
    t_list *tablaSegmentos = list_create();
    int ultimoElemento;

    list_add_all(listaInstrucciones, recibir_instrucciones(*socket_cliente));
    ultimoElemento = list_size(listaInstrucciones) - 1;

    tablaSegmentos = separarSegmentoDeInstrucciones(listaInstrucciones);
    list_remove(listaInstrucciones, ultimoElemento);

    t_pcb *pcbCreado = crearPCB(listaInstrucciones, tablaSegmentos);
    loggearLaCreacionDelPCB(pcbCreado);
    agregar_PCB_NEW_Queue(pcbCreado);*/

    //pedirInputTeclado("Mandame algo, maestro", *socket_cliente);

    // list_destroy(listaInstrucciones); //HAY UN PROBLEMA CON LAS MEMORIAS DE ESTAS LISTAS
    // list_destroy(tablaSegmentos);
    close(*socket_cliente);
    return 0;
}
*/
//fin avance martin


void *atender_cliente(void *args)
{
    int *socket_servidor = (int *)args;

    while (1)
    {
        pthread_t hiloAtendedor;
        int *socket_cliente = malloc(sizeof(int));
        *socket_cliente = accept(socket_servidor, NULL, NULL);
        pthread_create(&hiloAtendedor, // Crea hilo que crea hilos atendedores de consolas
                       NULL,
                       (void *)atender_modulo,
                       socket_cliente);
        pthread_detach(hiloAtendedor);
    }
    return 0;
}

int main(int argc, char ** argv){

    if(argc > 1 && strcmp(argv[1],"-test")==0)
        return 0;
    else{  
        t_log* logger = log_create("./cfg/memoria.log", "MEMORIA", true, LOG_LEVEL_INFO);
        log_info(logger, "Soy la Memoria %s", mi_funcion_compartida());
        t_memoria_config* memoriaConfig = inicializarConfig();
        
        log_info(logger, "complete la config");


     // iniciar_estructuras_memoria();
      //log_info(logger, "Estructuras de memoria inicializadas");
    int socket_servidor = inicializar_servidor(memoriaConfig->puerto_escucha);
        log_info(logger, "la memoria tiene socket %d\n", socket_servidor);

        //inicalizar_conexiones_memoria();
       
        pthread_create(&hiloModulos,  //Crea hilo que crea hilos atendedores de consolas
                        NULL,
                        (void *)atender_cliente,
                        socket_servidor);
        pthread_join(hiloModulos, NULL);
    }
  //      destruirLoggers();
    





    



    } 

