#define SEM_CHECK(s_id) if(s_id == -1) {perror("semget"); exit(EXIT_FAILURE);}
#define DEF_SEM_ID 1234