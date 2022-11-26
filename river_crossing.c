#include <stdio.h>
#include <pthread.h>

#define QTD_SERFS_GERADOS               100 // número de serfs para cruzar
#define QTD_HACKERS_GERADOS             100 // número de hackers para cruzar
#define TRIPULACAO_NECESSARIA           4   // cadeiras nos barcos
#define TOTAL_DE_BARCOS                 10  // barcos que zarparão 

pthread_mutex_t mutex_contador = PTHREAD_MUTEX_INITIALIZER;

pthread_cond_t acorda_serf = PTHREAD_COND_INITIALIZER;
pthread_cond_t acorda_hacker = PTHREAD_COND_INITIALIZER;

unsigned short int hackers_na_fila = 0;
unsigned short int serfs_na_fila = 0;

void rowBoat(){
    printf("Remando o barco.\n");
}

void board(int cargo){
    if(cargo) printf("Serf entrando no barco.\n");
    else printf("Hacker entrando no barco.\n");
}

void *serf(void *arg){
    pthread_mutex_lock(&mutex_contador);

    if(serfs_na_fila == 3){
        pthread_cond_signal(&acorda_serf);
        pthread_cond_signal(&acorda_serf);
        pthread_cond_signal(&acorda_serf);
        serfs_na_fila -= 3;
        board(1);
        rowBoat();
    }
    else if((serfs_na_fila >= 1) && (hackers_na_fila >= 2)){
        pthread_cond_signal(&acorda_hacker);
        pthread_cond_signal(&acorda_hacker);
        pthread_cond_signal(&acorda_serf);
        serfs_na_fila--;
        hackers_na_fila-=2;
        board(1);
        rowBoat();
    }
    else{
        serfs_na_fila++;
        pthread_cond_wait(&acorda_serf, &mutex_contador);
        board(1);
    }
    pthread_mutex_unlock(&mutex_contador);
    pthread_exit(NULL);
}

void *hacker(void *arg){
    pthread_mutex_lock(&mutex_contador);

    if(hackers_na_fila == 3){
        pthread_cond_signal(&acorda_hacker);
        pthread_cond_signal(&acorda_hacker);
        pthread_cond_signal(&acorda_hacker);
        hackers_na_fila -= 3;
        board(0);
        rowBoat();
    }
    else if((hackers_na_fila >= 1) && (serfs_na_fila >= 2)){
        pthread_cond_signal(&acorda_hacker);
        pthread_cond_signal(&acorda_serf);
        pthread_cond_signal(&acorda_serf);
        hackers_na_fila--;
        serfs_na_fila-=2;
        board(0);
        rowBoat();
    }
    else{
        hackers_na_fila++;
        pthread_cond_wait(&acorda_hacker, &mutex_contador);
        board(0);
    }
    pthread_mutex_unlock(&mutex_contador);
    pthread_exit(NULL);
}

void *criaHackers(){
    pthread_t threadsHackers[QTD_HACKERS_GERADOS];

    for (int i = 0 ; i < QTD_HACKERS_GERADOS; i++) pthread_create(&threadsHackers[i], NULL, hacker, NULL);
    for (int i = 0 ; i < QTD_HACKERS_GERADOS; i++) pthread_join(threadsHackers[i], NULL);
    pthread_exit(NULL);
}

void *criaSerfs(){
    pthread_t threadsSerfs[QTD_SERFS_GERADOS];

    for (int i = 0 ; i < QTD_SERFS_GERADOS; i++) pthread_create(&threadsSerfs[i], NULL, serf, NULL); 
    for (int i = 0 ; i < QTD_SERFS_GERADOS; i++) pthread_join(threadsSerfs[i], NULL); 
    pthread_exit(NULL);
}

int main(int argc, char **argv){
    pthread_t criadores[2];

    pthread_create(&criadores[0], NULL, criaHackers, NULL);
    pthread_create(&criadores[1], NULL, criaSerfs, NULL);

    pthread_join(criadores[0], NULL);
    pthread_join(criadores[1], NULL);
    return 0;
}