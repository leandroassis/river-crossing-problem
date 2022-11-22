#include <stdio.h>
#include <pthread.h>

#define QTD_SERFS_GERADOS               100 // número de serfs para cruzar
#define QTD_HACKERS_GERADOS             100 // número de hackers para cruzar
#define TRIPULACAO_NECESSARIA           4   // cadeiras nos barcos
#define TOTAL_DE_BARCOS                 10  // barcos que zarparão 

typedef enum {
    esperando,
    boarded,
    capitao
} cargo;

pthread_mutex_t mutex_contador = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_altera_estado_hacker = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_altera_estado_serf = PTHREAD_MUTEX_INITIALIZER;

pthread_cond_t acorda_serf = PTHREAD_COND_INITIALIZER;
pthread_cond_t acorda_hacker = PTHREAD_COND_INITIALIZER;

unsigned short int hackers_boarded = 0;
unsigned short int serfs_boarded = 0;

void rowBoat(){
    printf("Remando o barco.\n");
}

cargo board(int profissao){
    int hackers, serfs;

    pthread_mutex_lock(&mutex_contador);
    hackers = hackers_boarded;
    serfs = serfs_boarded;
    pthread_mutex_unlock(&mutex_contador);

    if(hackers+serfs == TRIPULACAO_NECESSARIA) return esperando;

    switch(profissao){
        case 0:
            if ((hackers == 0) || ((hackers == 1) && (serfs != 2)) || ((hackers == 2) && (serfs <= 1))){
                pthread_mutex_lock(&mutex_contador);
                printf("Serf entrando no barco.\n");
                serfs_boarded++;
                pthread_mutex_unlock(&mutex_contador);
                pthread_cond_signal(&acorda_serf);

                if(hackers+serfs == TRIPULACAO_NECESSARIA-1) return capitao;
                return boarded;
            }
        case 1:
            if ((serfs == 0) || ((serfs == 1) && (hackers != 2)) || ((serfs == 2) && (hackers <= 1))){
                pthread_mutex_lock(&mutex_contador);
                printf("Hacker entrando no barco.\n");
                hackers_boarded++;
                pthread_mutex_unlock(&mutex_contador);
                pthread_cond_signal(&acorda_hacker);

                if(hackers+serfs == TRIPULACAO_NECESSARIA-1) return capitao;
                return boarded;
            }
    }

    return esperando;
}

void *serf(void *arg){
    cargo estado;
    
    estado = board(0);
    
    pthread_mutex_lock(&mutex_altera_estado_serf);
    while(estado == esperando){
        pthread_cond_wait(&acorda_serf, &mutex_altera_estado_serf);
        estado = board(0);
    }
    pthread_mutex_unlock(&mutex_altera_estado_serf);

    if(estado == capitao){
        pthread_mutex_lock(&mutex_contador);
        hackers_boarded = 0;
        serfs_boarded = 0;
        rowBoat();
        pthread_mutex_unlock(&mutex_contador);
    }
    
    pthread_exit(NULL);
}

void *hacker(void *arg){
    cargo estado;
    
    estado = board(1);
    
    pthread_mutex_lock(&mutex_altera_estado_hacker);
    while(estado == esperando){
        pthread_cond_wait(&acorda_hacker, &mutex_altera_estado_hacker);
        estado = board(1);
    }
    pthread_mutex_unlock(&mutex_altera_estado_hacker);

    if(estado == capitao){
        pthread_mutex_lock(&mutex_contador);
        hackers_boarded = 0;
        serfs_boarded = 0;
        rowBoat();
        pthread_mutex_unlock(&mutex_contador);
    }
    
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