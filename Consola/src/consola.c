#include "consola.h"

t_log* logger;
typedef struct t_paqueteInstruccion {
    char* instruccion;
    uint32_t instruccion_longitud;
} t_paqueteInstruccion;


int readFile(char* path, t_list* list, t_log* logger){
    FILE* file;
    file = fopen(path,"r");

    if(file==NULL){
        log_info(logger, "Error abriendo archivo\n");
        return -1;
    }
    int size = 30*sizeof(char);
    int i=0;
    char* buff = malloc(size);
    while(fgets(buff, size, file)){
        char* aux = malloc(size);
        strcpy(aux,buff);
        aux = strtok(aux,"\n");
        list_add(list,aux);
        log_info(logger, "(%d) Agregado: %s", i,list_get(list,i));

        i++;
    }
    free(buff);
    fclose(file);
    log_info(logger,"Lei las instrucciones correctamente!\n");
    return 0;
}
int main(int argc, char ** argv){
    if(argc > 1 && strcmp(argv[1],"-test")==0)
        return run_tests();
    else{
        t_config* config;  
        logger = log_create("./cfg/consola.log", "consola", true, LOG_LEVEL_INFO);
        log_info(logger, "Inicializando la consola %s", mi_funcion_compartida());
        t_list* instr = list_create();
        if(argc==3){
            readFile(argv[1], instr,logger);
            //list_view(instr);
            config = config_create(argv[2]);
            //config_save_in_file(config,"/home/utnso/Desktop/config2.txt");
        }
        else{
            log_info(logger,"\nError: Argumentos insuficientes\n");
            return(-1);
        }
        
        
        //Conectar con el Kernel
        char* ip_kernel;
        char* puerto_kernel;
        char* segmentos;

        ip_kernel = buscarEnConfig(config, "IP_KERNEL");
        puerto_kernel = buscarEnConfig(config, "PUERTO_KERNEL");
        segmentos = buscarEnConfig(config, "SEGMENTOS");
        
        log_info(logger, "El Kernel tiene ip %s y su puerto es %s\n", ip_kernel,puerto_kernel);

        //Creando socket cliente
        int socket_cliente = inicializar_cliente(ip_kernel,puerto_kernel,logger);

        //Handshake
        int resultHandshake = handshakeCliente(socket_cliente,CONSOLA);
        log_info(logger, "El handshake %d\n", resultHandshake);
        
        for(int i=0; i<list_size(instr); i++){
            log_info(logger, "Instruccion: %s", list_get(instr, i));
        }

        enviar_instr(instr, segmentos, socket_cliente, logger);
        log_info(logger, "Mensaje enviado");

        /*op_code code;
        respuestaConsola respuesta;
        while(1){
            printf("\n--ESPERANDO MENSAJE DEL KERNEL--\n");
            recv(socket_cliente, &code, sizeof(op_code), MSG_WAITALL);
            printf("\n--RECIBIDO CODIGO DE OPERACION: %d--\n", (int)code);
            recv(socket_cliente, &respuesta, sizeof(respuestaConsola), MSG_WAITALL);
            printf("\n--RECIBIDO TIPO RESPUESTA: %d--\n", (int)respuesta);
            switch(respuesta){
                case CONTINUE: break;
                case END: return 0;
                case RESP_PANTALLA: log_info(logger, "Informacion del Kernel:\n%s", recibirNotificacion(socket_cliente)); break;
                case RESP_TECLADO: log_info(logger, "Solicitud de ingreso de datos:\n%s", 
                                            recibirNotificacion(socket_cliente));
                                    responderPorTeclado(socket_cliente); break;
                default: log_info(logger, "Mensaje incorrecto"); return(-1);
            }
        }*/

        log_destroy(logger);
    } 
}

char* recibirNotificacion(int socket_cliente){
    int longitud_mensaje;
    recv(socket_cliente, &longitud_mensaje, sizeof(int), MSG_WAITALL);
    char* mensaje = malloc(longitud_mensaje);
    recv(socket_cliente, mensaje, longitud_mensaje, MSG_WAITALL);
    return mensaje;
}

void responderPorTeclado(int socket_cliente){
    //op_code test;
    //log_info(logger, "CONGELADO, SOCKET VACIO...");
    //recv(socket_cliente, &test, sizeof(op_code), MSG_WAITALL);
    log_info(logger, "VOY A RESPONDER POR TECLADO...");
    char* respuesta = malloc(sizeof(char*)*21);
    log_info(logger, "CREO EL CHAR*");
    scanf("%20s",respuesta);
    log_info(logger, "HICE EL SCANF: %s", respuesta);
    int longitud_respuesta = strlen(respuesta) + 1;
    log_info(logger, "LONGITUD ES: %d", longitud_respuesta);
    int bytes_escritos = 0;
    int size = sizeof(op_code)+sizeof(int)+longitud_respuesta;
    void* buffer = malloc(size);

    op_code code = RTA_TECLADO;
    memcpy(buffer+bytes_escritos, &code, sizeof(op_code));
    log_info(logger, "ENVIO OPCODE: %d, SON %d BYTES", code, sizeof(op_code));
    bytes_escritos += sizeof(op_code);

    memcpy(buffer+bytes_escritos, &longitud_respuesta, sizeof(int));
    log_info(logger, "ENVIO LONGITUD: %d, SON %d BYTES", longitud_respuesta, sizeof(int));
    bytes_escritos += sizeof(int);

    memcpy(buffer+bytes_escritos, respuesta, longitud_respuesta);
    log_info(logger, "ENVIO RESPUESTA: %s, SON %d BYTES", respuesta, longitud_respuesta);
    bytes_escritos += sizeof(longitud_respuesta);
    
    log_info(logger, "SIZE ES %d", size);

    send(socket_cliente, buffer, size, 0);

}