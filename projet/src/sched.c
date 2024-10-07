#include "../include/sched.h"

struct scheduler {
    taskfunc *tasks;
    void **closures;
    int qlen;
    int task_libre;
    int sleeping_threads;
    int nthreads;
    pthread_mutex_t lock;
    pthread_cond_t cond;
};

void * scheduler_thread(void *arg) {
    
    struct scheduler *s = (struct scheduler *) arg;

    while (1) {

        pthread_mutex_lock(&s->lock);

        s->sleeping_threads++;

        // on réveille tous les threads si ils tous sont endormis et 0 tâches en attente
        if (s->task_libre < 0 && s->sleeping_threads == s->nthreads) {
            pthread_cond_broadcast(&s->cond);
            pthread_mutex_unlock(&s->lock);
            break;
        }

        // on attend une tâche disponible
        if (s->task_libre < 0) {
            pthread_cond_wait(&s->cond, &s->lock);

            // s'il n'y a toujours pas de tâches au réveil on termine
            if (s->task_libre < 0) {
                pthread_mutex_unlock(&s->lock);
                break;
            }
        }

        s->sleeping_threads--;

        taskfunc task = s->tasks[s->task_libre];
        void *closure = s->closures[s->task_libre];
        s->task_libre--;

        pthread_mutex_unlock(&s->lock);

        // on execute la tâche
        task(closure, s);

    }

    return NULL;
}


int sched_init(int nthreads, int qlen, taskfunc f, void *closure) {

    int rc;

    // on alloue de la mémoire
    struct scheduler *s = malloc(sizeof(struct scheduler));
    if (s == NULL) {
        perror("malloc");
        return -1;
    }

    if (nthreads == -1) nthreads = sched_default_threads();

    s->tasks = malloc(qlen * sizeof(taskfunc));
    if (s->tasks == NULL) {
        perror("malloc");
        free(s);
        return -1;
    }
    
    s->closures = malloc(qlen * sizeof(void *));
    if (s->closures == NULL) {
        perror("malloc");
        free(s->tasks);
        free(s);
        return -1;
    }

    // on initialise les pthread
    pthread_mutex_init(&s->lock, NULL);
    pthread_cond_init(&s->cond, NULL);

    // on stocke les données
    s->qlen = qlen;
    s->nthreads = nthreads;
    s->task_libre = 0;
    s->sleeping_threads = 0;
    s->tasks[s->task_libre] = f;
    s->closures[s->task_libre] = closure;

    pthread_t threads[nthreads];

    for (int i = 0; i < nthreads; i++) {
        
        rc = pthread_create(&threads[i], NULL, scheduler_thread, s);

        if (rc != 0) {
            perror("pthread_create");
            free(s->closures);
            free(s->tasks);
            free(s);
            return -1;
        }
    }

    for (int j = 0; j < nthreads; j++) {
        rc = pthread_join(threads[j], NULL);
        if (rc != 0) {
            perror("pthread_join");
            free(s->tasks);
            free(s->closures);
            free(s);
            return -1;
        }
    }

    // on désalloue la mémoire
    free(s->closures);
    free(s->tasks);
    free(s);

    return 0;
}

int sched_spawn(taskfunc f, void *closure, struct scheduler *s) {

    // on ne gère pas le cas où on on enfile + de qlen que prévu
    if (s->task_libre >= s->qlen) {
        errno = EAGAIN;
        return -1;
    }

    pthread_mutex_lock(&s->lock);

    // on enfile une nouvelle task
    s->task_libre++;
    s->tasks[s->task_libre] = f;
    s->closures[s->task_libre] = closure; 

    // on signale la nouvelle task
    pthread_cond_signal(&s->cond);

    pthread_mutex_unlock(&s->lock);

    return 0;
}
