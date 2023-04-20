#include "shared_utils.h"

 // Ver donde poner la variable hilo. Para reutilizarlo



typedef struct t_paqueteInstruccion {
    char* instruccion;
    int longitud;
} t_paqueteInstruccion;

char* mi_funcion_compartida(){
    return "Hice uso de la shared!";
}

void getAndPrint(t_list* list, int index){
    char* a = list_get(list,index);
    printf("(%d) %s\n",index,a);
}

void list_view(t_list* list){
    printf("\n--List view--\n");
    for(int i=0;i<list_size(list);i++)
        getAndPrint(list,i);
}

char* buscarEnConfig(t_config* config,char* index){
    char* valorObtenido;
    valorObtenido = config_get_string_value(config, index);
    return valorObtenido;
}

void inicializar_logger(char* direccion, char* nombre,t_log* logger){  //no funca bien
    logger = log_create(direccion, nombre, true, LOG_LEVEL_INFO);
}

void terminar_programa(int conexion, t_log* logger, t_config* config){
    if (logger != NULL){
        log_destroy(logger);
    }
    
    if (config != NULL){
        config_destroy(config);
    }
}

int inicializar_servidor(char* puerto){
    struct addrinfo hints, *servinfo;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    getaddrinfo(NULL, puerto, &hints, &servinfo); //crea el socket del server

    int socket_servidor = socket(servinfo->ai_family,
                                servinfo->ai_socktype,
                                servinfo->ai_protocol);

    bind(socket_servidor, servinfo->ai_addr, servinfo->ai_addrlen);

    listen(socket_servidor, SOMAXCONN);

    freeaddrinfo(servinfo);
    
    return socket_servidor;
}


 
void handshakeServidor(int socket_cliente,enum idProtocolo id_conectado){
    uint32_t handshake;
    uint32_t resultOk = 0;
    uint32_t resultError = -1;

    printf("\nHandshake solicitado. Fue: %d\n", id_conectado);  //Borrar, solo para corroborar


    recv(socket_cliente, &handshake, sizeof(uint32_t), MSG_WAITALL);
    sleep(3);
    if(handshake == id_conectado){
        send(socket_cliente, &resultOk, sizeof(uint32_t), 0);
        printf("\nHandshake OK. Fue: %d\n", resultOk);  //Borrar, solo para corroborar
    }
    else{
        send(socket_cliente, &resultError, sizeof(uint32_t), 0);
        printf("\nHandshake incorrecto. Fue: %d\n", resultError); //Borrar, solo para corroborar
        char* mensajeError = "ERROR AL HACER EL HANDSHAKE";
        error_show(mensajeError);
    }
    
    printf("\nhandshake recibido. Fue: %d\n", handshake);  //Borrar, solo para corroborar

    
}

int inicializar_cliente(char *ip,char *puerto,t_log* logger){
        struct addrinfo hints, *servinfo;

        memset(&hints, 0, sizeof(hints));
        hints.ai_family = AF_INET;
        hints.ai_socktype = SOCK_STREAM;
        hints.ai_flags = AI_PASSIVE;

        getaddrinfo(ip, puerto, &hints, &servinfo); //agarra el puerto y la ip del server

        int socket_cliente = socket(servinfo->ai_family,
                                    servinfo->ai_socktype,
                                    servinfo->ai_protocol);

        if(connect(socket_cliente, servinfo->ai_addr, servinfo->ai_addrlen) == -1)
            log_info(logger, "Error al conectar\n");
        else
            log_info(logger, "Conecto bien al puerto %s\n", puerto);
        freeaddrinfo(servinfo);
        
        return socket_cliente;
}
int handshakeCliente(int socket_cliente,enum idProtocolo id_a_conectar){
    uint32_t handshake = id_a_conectar;
    uint32_t result = -1;
    send(socket_cliente, &handshake, sizeof(uint32_t), 0);
    recv(socket_cliente, &result, sizeof(uint32_t), MSG_WAITALL);

    printf("\nHICE EL HANDSHAKE, FUE: %d\n", result);
    return result;
}

void* serializar_instrucciones(t_list* instr, char* segmentos, int *bytes, t_log* logger){

    int size_acumulado_instr = 0;
    for(int i=0; i<list_size(instr); i++){
        char* temp = malloc(sizeof(char)*30);
        temp = list_get(instr,i);
        size_acumulado_instr += sizeof(strlen(temp)) + sizeof(temp);
        log_info(logger, "(%d) %d", i, size_acumulado_instr);
    }
    
    log_info(logger, "Start Serialization");
    int bytes_escritos = 0;
    int size = sizeof(op_code) + sizeof(int) + size_acumulado_instr + sizeof(strlen(segmentos)) + strlen(segmentos);

    log_info(logger, "Cluster size: %d",size);

    void* buffer = malloc(size);

    log_info(logger,"Buffer size: %ld",sizeof(buffer));

    int listSize = list_size(instr)+1;

    memcpy(buffer+bytes_escritos, &listSize, sizeof(listSize));
    bytes_escritos += sizeof(int);

    op_code code = INSTR;

    memcpy(buffer+bytes_escritos, &code, sizeof(op_code));
    bytes_escritos += sizeof(op_code);

    for(int i=0; i<list_size(instr); i++){
        char* instruccion = malloc(30*sizeof(char));
        instruccion = list_get(instr, i);
        int len = strlen(instruccion)+1;
        memcpy(buffer + bytes_escritos, &len, sizeof(len));
        bytes_escritos += sizeof(len);
        memcpy(buffer + bytes_escritos, instruccion, len);
        log_info(logger, "(%d) [Longitud: %d] Instruccion: %s", i, len, instruccion);
        bytes_escritos += len;
    }
    int len = strlen(segmentos) + 1;
    memcpy(buffer+bytes_escritos, &len, sizeof(len));
    bytes_escritos += sizeof(len);

    memcpy(buffer+bytes_escritos, segmentos, len);
    bytes_escritos += len;

    (*bytes) = size;

    log_info(logger, "Cluster size: %d",size);
    log_info(logger,"Buffer size: %ld",sizeof(buffer));
    return buffer;
}

void* serializar_paquete(t_paquete *paquete, int* bytes){
    int size_serializado = sizeof(op_code)+sizeof(int)+paquete->buffer->size;
    char *buffer = malloc(size_serializado);

    int bytes_escritos = 0;

    memcpy(buffer + bytes_escritos, &(paquete->code), sizeof(paquete->code));
    bytes_escritos+=sizeof(paquete->code);

    memcpy(buffer + bytes_escritos, &(paquete->buffer->size), sizeof(paquete->buffer->size));
    bytes_escritos+=sizeof(paquete->buffer->size);

    memcpy(buffer + bytes_escritos, paquete->buffer->stream, paquete->buffer->size);
    bytes_escritos += paquete->buffer->size;

    (*bytes) = size_serializado;

    return buffer;

}

void enviar_instr(t_list* instr, char* segmentos, int socket_cliente, t_log* logger){
    log_info(logger, "---ENVIANDO INSTRUCCIONES---");
    int size_serializado;
    void* serializado = serializar_instrucciones(instr, segmentos, &size_serializado, logger);
    send(socket_cliente, serializado, size_serializado, 0);
    free(serializado);
}

void enviar(void* contenido, int socket_cliente){
    t_paquete *paquete = malloc(sizeof(t_paquete));
    paquete->code = 1;
    paquete->buffer = malloc(sizeof(t_buffer));
    paquete->buffer->stream = contenido;
    paquete->buffer->size = strlen(contenido) + 1;
    int size_serializado;
    void* serializado = serializar_paquete(paquete, &size_serializado);
    send(socket_cliente, serializado, size_serializado, 0);
    free(serializado);
}

char* borrarElementosCadena(char* cadena){
    int s,i,j = 0;
    char* nuevaCadena = malloc(strlen(cadena)+1);

    s = strlen(cadena);

    for (i = 0; i < s; i++)
    {
        if (cadena[i] != '[' && cadena[i] != ']'&& cadena[i] != ' ')
        {
            nuevaCadena[j] = cadena[i];
            j++;
        }
    }

    nuevaCadena[j] = '\0';
    return nuevaCadena;
}

int isNumber(char* cadena){
    int longitud = strlen(cadena);
    for(int i=0; i<longitud; i++){
        if(!isdigit(cadena[i]))
            return 0;
    }
    return 1;
}

void agregar_a_paquete(t_paquete* paquete, void *valor, int size)
{
    
  paquete->buffer->stream = realloc(paquete->buffer->stream, paquete->buffer->size + size + sizeof(int));

  memcpy(paquete->buffer->stream + paquete->buffer->size, &size, sizeof(int));
  memcpy(paquete->buffer->stream + paquete->buffer->size + sizeof(int), valor, size);

  paquete->buffer->size += size + sizeof(int);
}

void* serializarPCB(t_pcb* pcb, int socket){

    printf("\nARRANCA SERIALIZACION\n");

    t_list* instr = list_duplicate(pcb->instrucciones);
    int cantInstr = list_size(instr);//agrego un int para la cantidad de instrucciones
    printf("\nSON %d INSTRUCCIONES\n", cantInstr);

    int size_acumulado_instr = 0;
    for(int i=0; i<list_size(instr); i++){
        char* temp = malloc(sizeof(char)*30);
        temp = list_get(instr,i);
        size_acumulado_instr += sizeof(strlen(temp)) + sizeof(temp);
    }
    t_list* segmentos = pcb->t_segmentos;

    int size_segmentos = list_size(segmentos) * sizeof(t_segmento);

    int size = sizeof(op_code) 
                + sizeof(int) 
                + sizeof(estado_proceso)
                + sizeof(int)//agrego un int para la cantidad de instrucciones
                + size_acumulado_instr
                + sizeof(int)
                + sizeof(uint32_t)*4
                + sizeof(int)
                + size_segmentos;
    printf("\nSIZE ES %d \n", size);

    void* buffer = malloc(size);

    int bytes_escritos = 0;
    printf("\nBYTES ESCRITOS: %d\n", bytes_escritos);
    op_code code = PCB;

    memcpy(buffer+bytes_escritos, &code, sizeof(op_code));
    bytes_escritos+=sizeof(op_code);

    printf("\nBYTES ESCRITOS: %d\n", bytes_escritos);

    int pid = 8;

    memcpy(buffer+bytes_escritos, &(pcb->pid), sizeof(int));
    bytes_escritos+=sizeof(int);

    printf("\nBYTES ESCRITOS: %d\n", bytes_escritos);

    printf("\nPID %d \n", pcb->pid);

    memcpy(buffer+bytes_escritos, &(pcb->estado), sizeof(estado_proceso));
    bytes_escritos+=sizeof(estado_proceso);

    printf("\nBYTES ESCRITOS: %d\n", bytes_escritos);

    printf("\nSON %d INSTRUCCIONES\n", cantInstr);

    memcpy(buffer+bytes_escritos, &cantInstr, sizeof(int));
    bytes_escritos+=sizeof(int);

    printf("\nBYTES ESCRITOS: %d\n", bytes_escritos);

    for(int i=0; i<list_size(instr); i++){
        char* instruccion = malloc(30*sizeof(char));
        instruccion = list_get(instr, i);
        int len = strlen(instruccion)+1;
        memcpy(buffer + bytes_escritos, &len, sizeof(int));
        bytes_escritos += sizeof(int);
        printf("\nBYTES ESCRITOS: %d\n", bytes_escritos);
        memcpy(buffer + bytes_escritos, instruccion, len);
        bytes_escritos += len;
        printf("\nBYTES ESCRITOS: %d\n", bytes_escritos);
    }

    memcpy(buffer+bytes_escritos, &(pcb->program_counter), sizeof(int));
    bytes_escritos+=sizeof(int);

    printf("\nBYTES ESCRITOS: %d\n", bytes_escritos);

    memcpy(buffer+bytes_escritos, &(pcb->registros_cpu->AX), sizeof(uint32_t));
    bytes_escritos+=sizeof(uint32_t);

    printf("\nBYTES ESCRITOS: %d\n", bytes_escritos);

    memcpy(buffer+bytes_escritos, &(pcb->registros_cpu->BX), sizeof(uint32_t));
    bytes_escritos+=sizeof(uint32_t);

    printf("\nBYTES ESCRITOS: %d\n", bytes_escritos);

    memcpy(buffer+bytes_escritos, &(pcb->registros_cpu->CX), sizeof(uint32_t));
    bytes_escritos+=sizeof(uint32_t);

    printf("\nBYTES ESCRITOS: %d\n", bytes_escritos);

    memcpy(buffer+bytes_escritos, &(pcb->registros_cpu->DX), sizeof(uint32_t));
    bytes_escritos+=sizeof(uint32_t);

    printf("\nBYTES ESCRITOS: %d\n", bytes_escritos);

    //mando la cant de segmentos y despues los segmentos
    int cantSegmentos = list_size(segmentos);

    memcpy(buffer+bytes_escritos, &(cantSegmentos), sizeof(int));
    bytes_escritos+=sizeof(int);

    printf("\nBYTES ESCRITOS: %d\n", bytes_escritos);

    for(int i=0; i<list_size(segmentos); i++){
        t_segmento* segmento;
        segmento = list_get(segmentos, i);
        memcpy(buffer + bytes_escritos, &(segmento->idSegmento), sizeof(int));
        printf("\nID SEGMENTO: %d\n", segmento->idSegmento);
        bytes_escritos += sizeof(int);
        memcpy(buffer + bytes_escritos, &(segmento->tamanioSegmentos), sizeof(int));
        printf("\nSIZE SEGMENTO: %d\n", segmento->tamanioSegmentos);
        bytes_escritos += sizeof(int);
        memcpy(buffer + bytes_escritos, &(segmento->indiceTablaDePaginas), sizeof(int));
        printf("\nID TABLA: %d\n", segmento->indiceTablaDePaginas);
        bytes_escritos += sizeof(int);
        printf("\nBYTES ESCRITOS: %d\n", bytes_escritos);
    }

    printf("\nYA SERIALICE %d BYTES\n", bytes_escritos);

    send(socket, buffer, bytes_escritos, 0);

    return 0;
}

void enviarPCB(void* serializado, int size_serializado , int socket_a_enviar, t_log* logger){
    log_info(logger, "---ENVIANDO PCB---");
    log_info(logger, "---SIZE: %d---", size_serializado);
    send(socket_a_enviar, serializado, size_serializado, 0);
    log_info(logger, "---SEND---");
    //free(serializado);
    //free(size_serializado);
}

t_pcb* deserializarPCB(int socket_recibe){
    printf("\nESPERANDO UN PCB...\n");
    t_pcb* pcbRecibido = malloc(sizeof(t_pcb));

    //checkeamos el op code
    op_code operacion;
    recv(socket_recibe, &operacion, sizeof(op_code), MSG_WAITALL);

    printf("---Op_code: %d--\n", operacion);
    //recibe el Pid
    int processID;
    recv(socket_recibe, &processID, sizeof(int), MSG_WAITALL);

    printf("---processID: %d--\n", processID);

    //recibe el estado
    estado_proceso estadoProceso;
    recv(socket_recibe, &estadoProceso, sizeof(estado_proceso), MSG_WAITALL);

    printf("---estadoProceso: %d--\n", (int)estadoProceso);

    //recibimos las instrucciones y su cantidad
    int cantInstr;
    t_list *listaInstrucciones = list_create();
    
    recv(socket_recibe, &cantInstr, sizeof(int), MSG_WAITALL);
    printf("---CANT DE INSTRUCCIONES: %d--\n", cantInstr);
    for (int i = 0; i < cantInstr; i++)
    {
        int buffer_size;
        recv(socket_recibe, &buffer_size, sizeof(int), MSG_WAITALL);

        printf("\n---Longitud: %d\n", buffer_size);

        char *buffer = malloc(buffer_size);
        recv(socket_recibe, buffer, buffer_size, MSG_WAITALL);

        printf("\n---Instruccion: %s\n", buffer);

        list_add(listaInstrucciones, buffer);
    }
    //recibo el program counter
    int programCounter;
    recv(socket_recibe, &programCounter, sizeof(int), MSG_WAITALL);

    //recibo los registros del CPU
    uint32_t RAX,RBX,RCX,RDX;
    recv(socket_recibe, &RAX, sizeof(uint32_t), MSG_WAITALL);
    recv(socket_recibe, &RBX, sizeof(uint32_t), MSG_WAITALL);
    recv(socket_recibe, &RCX, sizeof(uint32_t), MSG_WAITALL);
    recv(socket_recibe, &RDX, sizeof(uint32_t), MSG_WAITALL);

    printf("\n---REGISTROS: %d, %d, %d, %d\n", RAX, RBX, RCX, RDX);

    //recibo los segmentos y su cantidad    
    t_list *listaSegmentos = list_create();
    int cantSegmentos;

    recv(socket_recibe, &cantSegmentos, sizeof(int), MSG_WAITALL);
    printf("\n---CANT SEGMENTOS: %d\n", cantSegmentos);

    for(int i=0; i<cantSegmentos; i++){
        t_segmento* segmento = malloc(sizeof(t_segmento));
        printf("\n---CREO SEGMENTO---\n");
        int id;
        int size;
        int idtabla;
        recv(socket_recibe, &id, sizeof(int), MSG_WAITALL);
        printf("\n---RECIBO ID---\n");
        recv(socket_recibe, &size, sizeof(int), MSG_WAITALL);
        printf("\n---RECIBO SIZE---\n");
        recv(socket_recibe, &idtabla, sizeof(int), MSG_WAITALL);
        printf("\n---RECIBO ID TABLA---\n");

        segmento->idSegmento = id;
        printf("\n---ASIGNO ID---\n");
        segmento->tamanioSegmentos = size;
        printf("\n---ASIGNO SIZE---\n");
        segmento->indiceTablaDePaginas = idtabla;
        printf("\n---ASIGNO ID TABLA---\n");

        printf("\n---SEGMENTO %d: %d, %d, %d\n", i, segmento->idSegmento, segmento->tamanioSegmentos, segmento->indiceTablaDePaginas);  
        list_add(listaSegmentos, segmento);
        printf("\n---AGREGA A LA LISTA---\n");
        //free(segmento);
    }
    printf("\n---SALI DEL FOR---\n");

    t_registros_cpu* registros = malloc(sizeof(t_registros_cpu));

    pcbRecibido->registros_cpu = registros;

    //meto todo al pcb
    pcbRecibido->pid = processID;
    printf("\n---OK---\n");
    pcbRecibido->estado = estadoProceso;
    printf("\n---OK---\n");
    pcbRecibido->instrucciones = list_duplicate(listaInstrucciones);
    printf("\n---OK---\n");
    pcbRecibido->program_counter = programCounter;
    printf("\n---OK---\n");
    pcbRecibido->registros_cpu->AX = RAX;
    printf("\n---OK---\n");
    pcbRecibido->registros_cpu->BX = RBX;
    printf("\n---OK---\n");
    pcbRecibido->registros_cpu->CX = RCX;
    printf("\n---OK---\n");
    pcbRecibido->registros_cpu->DX = RDX;
    printf("\n---OK---\n");

    pcbRecibido->t_segmentos = list_duplicate(listaSegmentos);
    printf("\n---OK---\n");
    
    return pcbRecibido;
}


<<<<<<< HEAD
void mostrarPCB(t_pcb* pcb,t_log* logger){
    t_pcb* pcbAmostrar = malloc(sizeof(t_pcb));
    pcbAmostrar = pcb;
=======
void mostrarPCB(t_pcb* pcbAmostrar, t_log* logger){
    
>>>>>>> d3dc53a385faab65a2d50890681e9480c8d360e1
    log_info(logger, "---MOSTRANDO PCB DEL PROCESO---");
    log_info(logger, "ID del proceso: %d", pcbAmostrar->pid);
    log_info(logger, "Estado del proceso: %d", pcbAmostrar->estado);

    for(int i = 0; i < list_size(pcbAmostrar->instrucciones);i++)
    {
        log_info(logger, "Instruccion del proceso: %s", list_get(pcbAmostrar->instrucciones,i));
    }
    
    log_info(logger, "Program Counter del proceso: %d", pcbAmostrar->program_counter);
    
    log_info(logger, "Contexto de registros del proceso. AX: %d. BX: %d. CX: %d. DX: %d",  //MIRAR BIEN COMO SE MUESTRAN CUANDO TIENEN VALOR 
                                                        pcbAmostrar->registros_cpu->AX,
                                                        pcbAmostrar->registros_cpu->BX, 
                                                        pcbAmostrar->registros_cpu->CX,
                                                        pcbAmostrar->registros_cpu->DX);

    for(int i = 0; i < list_size(pcbAmostrar->t_segmentos);i++)
    {
        t_segmento* segmento = list_get(pcbAmostrar->t_segmentos,i);
        log_info(logger, "Tamanio de los segmentos: %d", segmento->tamanioSegmentos);
    }

    //free(pcbAmostrar);
}

void* serializarPCB_DEVOLVER_conMotivo(t_pcb* pcb, int* tamanioEnBytes, motivo motivoDeDevolucion){

    printf("\nARRANCA SERIALIZACION\n");

    t_list* instr = list_duplicate(pcb->instrucciones);
    int cantInstr = list_size(instr);//agrego un int para la cantidad de instrucciones
    printf("\nSON %d INSTRUCCIONES\n", cantInstr);

    int size_acumulado_instr = 0;
    for(int i=0; i<list_size(instr); i++){
        char* temp = malloc(sizeof(char)*30);
        temp = list_get(instr,i);
        size_acumulado_instr += sizeof(strlen(temp)) + sizeof(temp);
    }
    t_list* segmentos = pcb->t_segmentos;

    int size_segmentos = list_size(segmentos) * sizeof(t_segmento);

    int size = sizeof(op_code) 
                + sizeof(int) 
                + sizeof(estado_proceso)
                + sizeof(int)//agrego un int para la cantidad de instrucciones
                + size_acumulado_instr
                + sizeof(int)
                + sizeof(uint32_t)*4
                + sizeof(int)
                + size_segmentos
                + sizeof(motivo);

    printf("\nSIZE ES %d \n", size);

    void* buffer = malloc(size);

    int bytes_escritos = 0;
    printf("\nBYTES ESCRITOS: %d\n", bytes_escritos);
    op_code code = PCB;

    memcpy(buffer+bytes_escritos, &code, sizeof(op_code));
    bytes_escritos+=sizeof(op_code);

    printf("\nBYTES ESCRITOS: %d\n", bytes_escritos);

    memcpy(buffer+bytes_escritos, &(pcb->pid), sizeof(int));
    bytes_escritos+=sizeof(int);

    printf("\nBYTES ESCRITOS: %d\n", bytes_escritos);

    printf("\nPID %d \n", pcb->pid);

    memcpy(buffer+bytes_escritos, &(pcb->estado), sizeof(estado_proceso));
    bytes_escritos+=sizeof(estado_proceso);

    printf("\nBYTES ESCRITOS: %d\n", bytes_escritos);

    printf("\nSON %d INSTRUCCIONES\n", cantInstr);

    memcpy(buffer+bytes_escritos, &cantInstr, sizeof(int));
    bytes_escritos+=sizeof(int);

    printf("\nBYTES ESCRITOS: %d\n", bytes_escritos);

    for(int i=0; i<list_size(instr); i++){
        char* instruccion = malloc(30*sizeof(char));
        instruccion = list_get(instr, i);
        int len = strlen(instruccion)+1;
        memcpy(buffer + bytes_escritos, &len, sizeof(int));
        bytes_escritos += sizeof(int);
        printf("\nBYTES ESCRITOS: %d\n", bytes_escritos);
        memcpy(buffer + bytes_escritos, instruccion, len);
        bytes_escritos += len;
        printf("\nBYTES ESCRITOS: %d\n", bytes_escritos);
    }

    memcpy(buffer+bytes_escritos, &(pcb->program_counter), sizeof(int));
    bytes_escritos+=sizeof(int);

    printf("\nBYTES ESCRITOS: %d\n", bytes_escritos);

    memcpy(buffer+bytes_escritos, &(pcb->registros_cpu->AX), sizeof(uint32_t));
    bytes_escritos+=sizeof(uint32_t);

    printf("\nBYTES ESCRITOS: %d\n", bytes_escritos);

    memcpy(buffer+bytes_escritos, &(pcb->registros_cpu->BX), sizeof(uint32_t));
    bytes_escritos+=sizeof(uint32_t);

    printf("\nBYTES ESCRITOS: %d\n", bytes_escritos);

    memcpy(buffer+bytes_escritos, &(pcb->registros_cpu->CX), sizeof(uint32_t));
    bytes_escritos+=sizeof(uint32_t);

    printf("\nBYTES ESCRITOS: %d\n", bytes_escritos);

    memcpy(buffer+bytes_escritos, &(pcb->registros_cpu->DX), sizeof(uint32_t));
    bytes_escritos+=sizeof(uint32_t);

    printf("\nBYTES ESCRITOS: %d\n", bytes_escritos);

    //mando la cant de segmentos y despues los segmentos
    int cantSegmentos = list_size(segmentos);

    memcpy(buffer+bytes_escritos, &(cantSegmentos), sizeof(int));
    bytes_escritos+=sizeof(int);

    printf("\nBYTES ESCRITOS: %d\n", bytes_escritos);

    for(int i=0; i<list_size(segmentos); i++){
        t_segmento* segmento;
        segmento = list_get(segmentos, i);
        memcpy(buffer + bytes_escritos, &(segmento->idSegmento), sizeof(int));
        printf("\nID SEGMENTO: %d\n", segmento->idSegmento);
        bytes_escritos += sizeof(int);
        memcpy(buffer + bytes_escritos, &(segmento->tamanioSegmentos), sizeof(int));
        printf("\nSIZE SEGMENTO: %d\n", segmento->tamanioSegmentos);
        bytes_escritos += sizeof(int);
        memcpy(buffer + bytes_escritos, &(segmento->indiceTablaDePaginas), sizeof(int));
        printf("\nID TABLA: %d\n", segmento->indiceTablaDePaginas);
        bytes_escritos += sizeof(int);
        printf("\nBYTES ESCRITOS: %d\n", bytes_escritos);
    }

    printf("\nYA SERIALICE %d BYTES\n", bytes_escritos);

    memcpy(buffer+bytes_escritos, &motivoDeDevolucion, sizeof(motivo));
    bytes_escritos+=sizeof(motivo);

    *tamanioEnBytes = bytes_escritos;

    return buffer;
}

void* serializarPCB_DEVOLVER_porIO(t_pcb* pcb, int* tamanioEnBytes,char* dispositivo,int cantidad_a_ejecutar){

    printf("\nARRANCA SERIALIZACION\n");
    motivo motivoDeDevolucion = IO;
    int tamanioDelDispositivo = strlen(dispositivo)+1;

    t_list* instr = list_duplicate(pcb->instrucciones);
    int cantInstr = list_size(instr);//agrego un int para la cantidad de instrucciones
    printf("\nSON %d INSTRUCCIONES\n", cantInstr);

    int size_acumulado_instr = 0;
    for(int i=0; i<list_size(instr); i++){
        char* temp = malloc(sizeof(char)*30);
        temp = list_get(instr,i);
        size_acumulado_instr += sizeof(strlen(temp)) + sizeof(temp);
    }
    t_list* segmentos = pcb->t_segmentos;

    int size_segmentos = list_size(segmentos) * sizeof(t_segmento);

    int size = sizeof(op_code) 
                + sizeof(int) 
                + sizeof(estado_proceso)
                + sizeof(int)//agrego un int para la cantidad de instrucciones
                + size_acumulado_instr
                + sizeof(int)
                + sizeof(uint32_t)*4
                + sizeof(int)
                + size_segmentos
                + sizeof(motivo)
                + sizeof(int)
                + tamanioDelDispositivo
                + sizeof(int);

    printf("\nSIZE ES %d \n", size);

    void* buffer = malloc(size);

    int bytes_escritos = 0;
    printf("\nBYTES ESCRITOS: %d\n", bytes_escritos);
    op_code code = PCB;

    memcpy(buffer+bytes_escritos, &code, sizeof(op_code));
    bytes_escritos+=sizeof(op_code);

    printf("\nBYTES ESCRITOS: %d\n", bytes_escritos);

    memcpy(buffer+bytes_escritos, &(pcb->pid), sizeof(int));
    bytes_escritos+=sizeof(int);

    printf("\nBYTES ESCRITOS: %d\n", bytes_escritos);

    printf("\nPID %d \n", pcb->pid);

    memcpy(buffer+bytes_escritos, &(pcb->estado), sizeof(estado_proceso));
    bytes_escritos+=sizeof(estado_proceso);

    printf("\nBYTES ESCRITOS: %d\n", bytes_escritos);

    printf("\nSON %d INSTRUCCIONES\n", cantInstr);

    memcpy(buffer+bytes_escritos, &cantInstr, sizeof(int));
    bytes_escritos+=sizeof(int);

    printf("\nBYTES ESCRITOS: %d\n", bytes_escritos);

    for(int i=0; i<list_size(instr); i++){
        char* instruccion = malloc(30*sizeof(char));
        instruccion = list_get(instr, i);
        int len = strlen(instruccion)+1;
        memcpy(buffer + bytes_escritos, &len, sizeof(int));
        bytes_escritos += sizeof(int);
        printf("\nBYTES ESCRITOS: %d\n", bytes_escritos);
        memcpy(buffer + bytes_escritos, instruccion, len);
        bytes_escritos += len;
        printf("\nBYTES ESCRITOS: %d\n", bytes_escritos);
    }

    memcpy(buffer+bytes_escritos, &(pcb->program_counter), sizeof(int));
    bytes_escritos+=sizeof(int);

    printf("\nBYTES ESCRITOS: %d\n", bytes_escritos);

    memcpy(buffer+bytes_escritos, &(pcb->registros_cpu->AX), sizeof(uint32_t));
    bytes_escritos+=sizeof(uint32_t);

    printf("\nBYTES ESCRITOS: %d\n", bytes_escritos);

    memcpy(buffer+bytes_escritos, &(pcb->registros_cpu->BX), sizeof(uint32_t));
    bytes_escritos+=sizeof(uint32_t);

    printf("\nBYTES ESCRITOS: %d\n", bytes_escritos);

    memcpy(buffer+bytes_escritos, &(pcb->registros_cpu->CX), sizeof(uint32_t));
    bytes_escritos+=sizeof(uint32_t);

    printf("\nBYTES ESCRITOS: %d\n", bytes_escritos);

    memcpy(buffer+bytes_escritos, &(pcb->registros_cpu->DX), sizeof(uint32_t));
    bytes_escritos+=sizeof(uint32_t);

    printf("\nBYTES ESCRITOS: %d\n", bytes_escritos);

    //mando la cant de segmentos y despues los segmentos
    int cantSegmentos = list_size(segmentos);

    memcpy(buffer+bytes_escritos, &(cantSegmentos), sizeof(int));
    bytes_escritos+=sizeof(int);

    printf("\nBYTES ESCRITOS: %d\n", bytes_escritos);

    for(int i=0; i<list_size(segmentos); i++){
        t_segmento* segmento;
        segmento = list_get(segmentos, i);
        memcpy(buffer + bytes_escritos, &(segmento->idSegmento), sizeof(int));
        printf("\nID SEGMENTO: %d\n", segmento->idSegmento);
        bytes_escritos += sizeof(int);
        memcpy(buffer + bytes_escritos, &(segmento->tamanioSegmentos), sizeof(int));
        printf("\nSIZE SEGMENTO: %d\n", segmento->tamanioSegmentos);
        bytes_escritos += sizeof(int);
        memcpy(buffer + bytes_escritos, &(segmento->indiceTablaDePaginas), sizeof(int));
        printf("\nID TABLA: %d\n", segmento->indiceTablaDePaginas);
        bytes_escritos += sizeof(int);
        printf("\nBYTES ESCRITOS: %d\n", bytes_escritos);
    }

    printf("\nYA SERIALICE %d BYTES\n", bytes_escritos);

    memcpy(buffer+bytes_escritos, &motivoDeDevolucion, sizeof(motivo));
    bytes_escritos+=sizeof(motivo);

    //tamanio del char dispositivo

    memcpy(buffer+bytes_escritos, &tamanioDelDispositivo, sizeof(int));
    bytes_escritos+=sizeof(int);

    //dispositivo
    memcpy(buffer+bytes_escritos, dispositivo, tamanioDelDispositivo);
    bytes_escritos+=tamanioDelDispositivo;

    //cantidad de veces a ejecutar
    memcpy(buffer+bytes_escritos, &cantidad_a_ejecutar, sizeof(int));
    bytes_escritos+=sizeof(int);

    *tamanioEnBytes = bytes_escritos;

    return buffer;
}


void* serializarPCB_DEVOLVER_porPAGEFAULT(t_pcb* pcb, int* tamanioEnBytes, int paginaReclamada){

    printf("\nARRANCA SERIALIZACION\n");
    motivo motivoDeDevolucion = PAGEFAULT;
    t_list* instr = list_duplicate(pcb->instrucciones);
    int cantInstr = list_size(instr);//agrego un int para la cantidad de instrucciones
    printf("\nSON %d INSTRUCCIONES\n", cantInstr);

    int size_acumulado_instr = 0;
    for(int i=0; i<list_size(instr); i++){
        char* temp = malloc(sizeof(char)*30);
        temp = list_get(instr,i);
        size_acumulado_instr += sizeof(strlen(temp)) + sizeof(temp);
    }
    t_list* segmentos = pcb->t_segmentos;

    int size_segmentos = list_size(segmentos) * sizeof(t_segmento);

    int size = sizeof(op_code) 
                + sizeof(int) 
                + sizeof(estado_proceso)
                + sizeof(int)//agrego un int para la cantidad de instrucciones
                + size_acumulado_instr
                + sizeof(int)
                + sizeof(uint32_t)*4
                + sizeof(int)
                + size_segmentos
                + sizeof(motivo)
                + sizeof(int);

    printf("\nSIZE ES %d \n", size);

    void* buffer = malloc(size);

    int bytes_escritos = 0;
    printf("\nBYTES ESCRITOS: %d\n", bytes_escritos);
    op_code code = PCB;

    memcpy(buffer+bytes_escritos, &code, sizeof(op_code));
    bytes_escritos+=sizeof(op_code);

    printf("\nBYTES ESCRITOS: %d\n", bytes_escritos);

    memcpy(buffer+bytes_escritos, &(pcb->pid), sizeof(int));
    bytes_escritos+=sizeof(int);

    printf("\nBYTES ESCRITOS: %d\n", bytes_escritos);

    printf("\nPID %d \n", pcb->pid);

    memcpy(buffer+bytes_escritos, &(pcb->estado), sizeof(estado_proceso));
    bytes_escritos+=sizeof(estado_proceso);

    printf("\nBYTES ESCRITOS: %d\n", bytes_escritos);

    printf("\nSON %d INSTRUCCIONES\n", cantInstr);

    memcpy(buffer+bytes_escritos, &cantInstr, sizeof(int));
    bytes_escritos+=sizeof(int);

    printf("\nBYTES ESCRITOS: %d\n", bytes_escritos);

    for(int i=0; i<list_size(instr); i++){
        char* instruccion = malloc(30*sizeof(char));
        instruccion = list_get(instr, i);
        int len = strlen(instruccion)+1;
        memcpy(buffer + bytes_escritos, &len, sizeof(int));
        bytes_escritos += sizeof(int);
        printf("\nBYTES ESCRITOS: %d\n", bytes_escritos);
        memcpy(buffer + bytes_escritos, instruccion, len);
        bytes_escritos += len;
        printf("\nBYTES ESCRITOS: %d\n", bytes_escritos);
    }

    memcpy(buffer+bytes_escritos, &(pcb->program_counter), sizeof(int));
    bytes_escritos+=sizeof(int);

    printf("\nBYTES ESCRITOS: %d\n", bytes_escritos);

    memcpy(buffer+bytes_escritos, &(pcb->registros_cpu->AX), sizeof(uint32_t));
    bytes_escritos+=sizeof(uint32_t);

    printf("\nBYTES ESCRITOS: %d\n", bytes_escritos);

    memcpy(buffer+bytes_escritos, &(pcb->registros_cpu->BX), sizeof(uint32_t));
    bytes_escritos+=sizeof(uint32_t);

    printf("\nBYTES ESCRITOS: %d\n", bytes_escritos);

    memcpy(buffer+bytes_escritos, &(pcb->registros_cpu->CX), sizeof(uint32_t));
    bytes_escritos+=sizeof(uint32_t);

    printf("\nBYTES ESCRITOS: %d\n", bytes_escritos);

    memcpy(buffer+bytes_escritos, &(pcb->registros_cpu->DX), sizeof(uint32_t));
    bytes_escritos+=sizeof(uint32_t);

    printf("\nBYTES ESCRITOS: %d\n", bytes_escritos);

    //mando la cant de segmentos y despues los segmentos
    int cantSegmentos = list_size(segmentos);

    memcpy(buffer+bytes_escritos, &(cantSegmentos), sizeof(int));
    bytes_escritos+=sizeof(int);

    printf("\nBYTES ESCRITOS: %d\n", bytes_escritos);

    for(int i=0; i<list_size(segmentos); i++){
        t_segmento* segmento;
        segmento = list_get(segmentos, i);
        memcpy(buffer + bytes_escritos, &(segmento->idSegmento), sizeof(int));
        printf("\nID SEGMENTO: %d\n", segmento->idSegmento);
        bytes_escritos += sizeof(int);
        memcpy(buffer + bytes_escritos, &(segmento->tamanioSegmentos), sizeof(int));
        printf("\nSIZE SEGMENTO: %d\n", segmento->tamanioSegmentos);
        bytes_escritos += sizeof(int);
        memcpy(buffer + bytes_escritos, &(segmento->indiceTablaDePaginas), sizeof(int));
        printf("\nID TABLA: %d\n", segmento->indiceTablaDePaginas);
        bytes_escritos += sizeof(int);
        printf("\nBYTES ESCRITOS: %d\n", bytes_escritos);
    }

    printf("\nYA SERIALICE %d BYTES\n", bytes_escritos);

    memcpy(buffer+bytes_escritos, &motivoDeDevolucion, sizeof(motivo));
    bytes_escritos+=sizeof(motivo);

    memcpy(buffer+bytes_escritos, &paginaReclamada, sizeof(int));
    bytes_escritos+=sizeof(int);

    *tamanioEnBytes = bytes_escritos;

    return buffer;
}