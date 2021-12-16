#define USING_SEM_BIN 1
#ifndef EXIT_STATUS
#define EXIT_STATUS 1
#endif
#define SEM_RESPONSE(resp, message) if(resp == -1){perror(message);exit(EXIT_FAILURE);}

int initSemAvailable(int semId, int semNum);
int initSemsetAvailable(int semId, int size);
int initSemInUse(int semId, int semNum);
int initSemsetInUse(int semId, int size);
int reserveSem(int semId, int semNum);
int releaseSem(int semId, int semNum);
