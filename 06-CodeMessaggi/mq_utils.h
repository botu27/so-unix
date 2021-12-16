#define MQ_ID 1234
#define MQ_ID_DEFINED_BY_PROC_FATHER 4321
#define MQ_BUFFER_MAX_SIZE 256

#define PRINT_THEN_EXIT(mex_to_print, e_status) perror(mex_to_print); exit(e_status);
#define CHECK_MQ(mq_id) if(mq_id == -1) {perror("msgget"); exit(-1);};
#define PRINT_TRACE(method) fprintf(stderr, "%s: %d. Errore in %s #%03d: %s\n", __FILE__, __LINE__, method,errno, strerror(errno)); exit(EXIT_FAILURE);