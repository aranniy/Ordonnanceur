#include "../include/sched.h"
#include "../include/deque.h"

struct scheduler {
    struct deque **d;
    int nbr_threads;
    int qlen;
    int nbr_tasks;
    pthread_mutex_t lock;
};

struct args {
    int id;
    struct scheduler *sched;
};

void* thread_exec(void *arg) {

    struct args * param = (struct args *) arg;

    struct scheduler * sched = param -> sched;
    int id = param -> id;

    int nbr_threads = sched -> nbr_threads;
    int flag = 0;

    srand(time(NULL) + getpid());

    while (1) {

        if(flag == 0) pthread_mutex_lock(&sched -> lock);

        struct deque *deque = (sched -> d)[id];

        if(deque == NULL) {
            pthread_mutex_unlock(&sched -> lock);
            continue;
        }

        if(isEmpty(deque)) {

            //printf("thread %d est vide\n", id);

            int thread = rand() % nbr_threads;

            for(int i = 0; i < nbr_threads; i++) {

                int pos = (thread + i) % nbr_threads;

                if(pos != id) {

                    struct deque *tmp = (sched -> d)[pos];
                    if(tmp == NULL) {
                        continue;
                    }

                    struct task * task = getTop(tmp);
                    if(task != NULL) {
                        //printf("thread %d pioche chez %d\n", id, pos);
                        addTop(deque, task -> function, task -> args);
                        flag = 1;
                        break;
                    }

                }

            }

            if(flag == 1) {
                continue;  
            }

            //printf("thread %d n'a toujours pas de travail\n", id);

            pthread_mutex_unlock(&sched -> lock);

            struct timespec t;
            t.tv_sec = 1 / 1000;
            t.tv_nsec = (1 % 1000) * 1000000;

            nanosleep(&t, &t);

            pthread_mutex_lock(&sched -> lock);

            if(sched -> nbr_tasks == 0) {

                int done = 1;

                for(int i = 0; i < nbr_threads; i++) {
                    if(!isEmpty((sched -> d)[i])) {
                        done = 0;
                        break;
                    }
                }

                if(done) {
                    pthread_mutex_unlock(&sched -> lock);
                    break;
                }

            }

            pthread_mutex_unlock(&sched -> lock);

        } else {

            //printf("thread %d a du travail\n", id);

            struct task * task = getBottom((sched -> d)[id]);

            taskfunc function = task -> function;
            void * param = task -> args;

            sched -> nbr_tasks -= 1;

            //printf("thread %d a fini une tache\n", id);

            pthread_mutex_unlock(&sched -> lock);

            flag = 0;

            function(param, sched);

            free(task);

        }

    }

    //printf("%d termine\n", id);

    free(param);
    return NULL;
}

int sched_init(int nthreads, int qlen, taskfunc f, void *closure) {

    int rc;

    struct scheduler *sched = malloc(sizeof(struct scheduler));

    if (sched == NULL) {
        perror("sched_init : malloc failure ");
        return -1;
    }

    if (nthreads <= 0) nthreads = sched_default_threads();

    sched -> d = malloc(sizeof(struct deque) * nthreads);
    if(sched -> d == NULL) {
        perror("sched_init : malloc failure ");
        return -1;
    }

    sched -> nbr_threads = nthreads;
    sched -> qlen = qlen;
    sched -> nbr_tasks = 0;
    pthread_mutex_init(&sched -> lock, NULL);

    pthread_t threads[nthreads];
    int size = qlen / nthreads;
    if(size <= 0) size = 1;

    pthread_mutex_lock(&sched -> lock);

    for (int i = 0; i < nthreads; i++) {

        struct args * args = malloc(sizeof(struct args));
        if(args == NULL) {
            perror("sched_init : malloc failure ");
        }
        args -> id = i;
        args -> sched = sched;

        if(i == 0) 
            (sched -> d)[i] = init(i, size + (qlen % nthreads));
        else 
            (sched -> d)[i] = init(i, size);

        if((sched -> d)[i] == NULL) {
            perror("sched_init : deque init failure ");
        }
        
        rc = pthread_create(&threads[i], NULL, thread_exec, args);

        if (rc != 0) {
            perror("sched_init : pthread_create ");
            free(sched);
            return -1;
        }
    } 

    pthread_mutex_unlock(&sched -> lock);

    rc = sched_spawn(f, closure, sched);
    if(rc != 0) {
        perror("sched_init : spwaning intial task failure ");
        free(sched);
        return 0;
    } 

    for (int i = 0; i < nthreads; i++) {

        rc = pthread_join(threads[i], NULL);

        if (rc != 0) {
            perror("sched_init : pthread_join ");
            free(sched);
            return -1;
        }
    }

    free(sched);
    return 0;
}

int sched_spawn(taskfunc f, void *closure, struct scheduler *s) {

    pthread_mutex_lock(&s -> lock);

    if (s -> nbr_tasks  >= s -> qlen) {
        errno = EAGAIN;
        return -1;
    }

    srand(time(NULL) + getpid());

    int flag = 0;
    int thread = rand() % (s -> nbr_threads);

    for(int i = 0; i < s -> nbr_threads; i++) {

        int tmp = (thread + i) % s -> nbr_threads;

        struct deque *deque = (s -> d)[tmp];

        if(deque == NULL) continue;

        if(addBottom(deque, f, closure) == 0) {
            //printf("donner une tache à %d\n", i);
            flag = 1;
            break;
        }
    }
 
    if(flag) s -> nbr_tasks += 1;

    pthread_mutex_unlock(&s -> lock);

    return 0;
}