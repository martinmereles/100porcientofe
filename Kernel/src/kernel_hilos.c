#include "kernel_hilos.h"

void *atender_cliente(void *args)
{
    int *socket_servidor = (int *)args;
    printf("\n--Hilo atendedor de consolas--\n");

    while (1)
    {
        pthread_t hiloAtendedor;
        int *socket_cliente = malloc(sizeof(int));
        *socket_cliente = accept(socket_servidor, NULL, NULL);
        pthread_create(&hiloAtendedor, // Crea hilo que crea hilos atendedores de consolas
                       NULL,
                       (void *)atender_consola,
                       socket_cliente);
        pthread_detach(hiloAtendedor);
    }
    return 0;
}

// TEMPORAL

void pedirInputTeclado(char *mensaje, int socket_cliente)
{
    printf("\n--INICIA SOLICITUD DE INPUT--\n");
    int longitud_mensaje = strlen(mensaje) + 1;
    printf("\n--MIDE LONGITUD DEL MENSAJE: %s--\n", mensaje);
    int bytes_escritos = 0;
    int size = sizeof(op_code) + sizeof(respuestaConsola) + sizeof(int) + longitud_mensaje;
    void *buffer = malloc(size);
    printf("\n--CREA EL BUFFER--\n");

    op_code code = RTA_TECLADO;
    memcpy(buffer, &code, sizeof(op_code));
    printf("\n--GUARDA EL OPCODE EN MEMORIA--\n");
    bytes_escritos += sizeof(op_code);

    respuestaConsola respuesta = RESP_TECLADO;
    memcpy(buffer + bytes_escritos, &respuesta, sizeof(respuestaConsola));
    bytes_escritos += sizeof(respuestaConsola);

    memcpy(buffer + bytes_escritos, &longitud_mensaje, sizeof(int));
    printf("\n--GUARDA LA LONGITUD EN MEMORIA: %d--\n", longitud_mensaje);
    bytes_escritos += sizeof(int);

    printf("\n--CASI GUARDA EL MENSAJE EN MEMORIA: '%s'--\n", mensaje);
    memcpy(buffer + bytes_escritos, mensaje, longitud_mensaje);
    printf("\n--GUARDA EL MENSAJE EN MEMORIA--\n");
    bytes_escritos += longitud_mensaje;

    send(socket_cliente, buffer, size, 0);
    printf("\n--MANDO EL MENSAJE--\n");

    sleep(100);
    op_code codeLoco;
    op_code codeRta = 5;
    int longitudRta;

    recv(socket_cliente, &codeLoco, sizeof(op_code), MSG_WAITALL);
    printf("\n--ESTOY ESPERANDO LA OPCODE DE CONSOLA: %d BYTES--\n", sizeof(op_code));
    recv(socket_cliente, &codeRta, sizeof(op_code), MSG_WAITALL);
    printf("\n--OPCODE DE LA CONSOLA: %d--\n", codeRta);
    printf("\n--ESTOY ESPERANDO LONGITUD DE RESPUESTA: %d BYTES--\n", sizeof(int));
    recv(socket_cliente, &longitudRta, sizeof(int), MSG_WAITALL);

    char* input = malloc(longitudRta);

    printf("\n--LONGITUD DEL INPUT: %d--\n", longitudRta);
    printf("\n--ESTOY ESPERANDO RESPUESTA: %d BYTES--\n", longitudRta);
    recv(socket_cliente, input, longitudRta, MSG_WAITALL);
    printf("\n--INPUT DE LA CONSOLA ES: '%s'--\n", input);
}

// TEMPORAL

void *atender_consola(void *args)
{
    int *socket_cliente = (int *)args;
    handshakeServidor(*socket_cliente, CONSOLA); // Argv es socket_cliente
    // log_info(logger,"Lei las instrucciones correctamente!\n");

    t_list *listaInstrucciones = list_create();
    t_list *tablaSegmentos = list_create();
    int ultimoElemento;

    list_add_all(listaInstrucciones, recibir_instrucciones(*socket_cliente));
    ultimoElemento = list_size(listaInstrucciones) - 1;

    tablaSegmentos = separarSegmentoDeInstrucciones(listaInstrucciones);
    list_remove(listaInstrucciones, ultimoElemento);

    t_pcb *pcbCreado = crearPCB(listaInstrucciones, tablaSegmentos);
    loggearLaCreacionDelPCB(pcbCreado);
    agregar_PCB_NEW_Queue(pcbCreado);

    //t_pcb* testPCB = inicializarPCBtest();

    //pedirInputTeclado("Mandame algo, maestro", *socket_cliente);

    // list_destroy(listaInstrucciones); //HAY UN PROBLEMA CON LAS MEMORIAS DE ESTAS LISTAS
    // list_destroy(tablaSegmentos);
    close(*socket_cliente);
    return 0;
}

t_list *separarSegmentoDeInstrucciones(t_list *listaDeInstrucciones)
{
    t_list *tablaSegmentos = list_create();

    int posicionDelUltimoElemento = list_size(listaDeInstrucciones) - 1;
    char *segmentosEnString = list_get(listaDeInstrucciones, posicionDelUltimoElemento);
    segmentosEnString = borrarElementosCadena(segmentosEnString);

    char *delimitador = ",";
    char *token = strtok(segmentosEnString, delimitador);

    int contadorDeIDSegmentos = 0;
    if (token != NULL)
    {
        while (token != NULL)
        {
            // Sólo en la primera pasamos la cadena; en las siguientes pasamos NULL
            t_segmento *segmento = malloc(sizeof(t_segmento *));
            segmento->tamanioSegmentos = atoi(token); // atoi convierte el char* en un int
            segmento->idSegmento = contadorDeIDSegmentos++;

            list_add(tablaSegmentos, segmento);

            token = strtok(NULL, delimitador);
        }
    }
    return tablaSegmentos; // REVISAR
}
