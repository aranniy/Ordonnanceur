#include "../include/deque.h"

struct deque * init(int id, int qlen) {

    struct deque * deque = malloc(sizeof(struct deque));

    if(deque != NULL) {
        deque -> id = id;
        deque -> top = NULL;
        deque -> bottom = NULL;
        deque -> size = 0;
        deque -> qlen = qlen;
        pthread_mutex_init(&deque -> lock, NULL);
    }

    return deque;

}

int isEmpty(struct deque *d) {
    pthread_mutex_lock(&d -> lock);
    if(d == NULL) {
        pthread_mutex_unlock(&d -> lock);
        return -1;
    }
    int tmp = d -> top == NULL && d -> bottom == NULL && d -> size == 0;
    pthread_mutex_unlock(&d -> lock);
    return tmp;
}

int addBottom(struct deque *d, void * function, void * args) {

    pthread_mutex_lock(&d -> lock);

    if(d == NULL || function == NULL || args == NULL) {
        pthread_mutex_unlock(&d -> lock);
        return -1;
    }

    if(d -> size >= d -> qlen) {
        pthread_mutex_unlock(&d -> lock);
        return -1;
    }

    struct node *tmp = malloc(sizeof(struct node));
    struct task *fun = malloc(sizeof(struct task));

    if(tmp == NULL || fun == NULL) {
        pthread_mutex_unlock(&d -> lock);
        return -1;
    }

    fun -> function = function;
    fun -> args = args;

    tmp -> task = fun;
    tmp -> next = NULL;

    if(d -> top == NULL && d -> bottom == NULL && d -> size == 0) {
        tmp -> prev = NULL;
        d -> bottom = tmp;
        d -> top = tmp;

    } else {
        tmp -> prev = d -> bottom;
        d -> bottom -> next = tmp;
        d -> bottom = tmp;

    }

    d -> size = d -> size + 1;

    pthread_mutex_unlock(&d -> lock);
    return 0;

}

int addTop(struct deque * d, void * function, void * args) {

    pthread_mutex_lock(&d -> lock);

    if(d == NULL || function == NULL || args == NULL) {
        pthread_mutex_unlock(&d -> lock);
        return -1;
    }

    if(d -> size >= d -> qlen) {
        pthread_mutex_unlock(&d -> lock);
        return -1;
    }

    struct node *tmp = malloc(sizeof(struct node));
    struct task *fun = malloc(sizeof(struct task));

    if(tmp == NULL || fun == NULL) {
        pthread_mutex_unlock(&d -> lock);
        return -1;
    }

    fun -> function = function;
    fun -> args = args;

    tmp -> task = fun;
    tmp -> prev = NULL;

    if(d -> top == NULL && d -> bottom == NULL && d -> size == 0) {

        tmp -> next = NULL;

        d -> bottom = tmp;
        d -> top = tmp;

    } else {

        tmp -> next = d -> top;

        d -> top -> prev = tmp;
        d -> top = tmp;

    }

    d -> size = d -> size + 1;

    pthread_mutex_unlock(&d -> lock);

    return 0;

}

struct task * getBottom(struct deque *d) {

    pthread_mutex_lock(&d -> lock);

    if(d == NULL) {
        pthread_mutex_unlock(&d -> lock);
        return NULL;
    }

    if(d -> top == NULL && d -> bottom == NULL && d -> size == 0) {
        pthread_mutex_unlock(&d -> lock);
        return NULL;
    }

    struct node * node = d -> bottom;

    if(node -> prev == NULL) {
        d -> bottom = NULL;
        d -> top = NULL;
    }

    else {
        node -> prev -> next = NULL;
        d -> bottom = node -> prev;
        node -> prev = NULL;
    }
    
    struct task * data = node -> task;

    free(node);

    d -> size = d -> size - 1;

    pthread_mutex_unlock(&d -> lock);

    return data;
}

struct task * getTop(struct deque *d){

    pthread_mutex_lock(&d -> lock);

    if(d == NULL) {
        pthread_mutex_unlock(&d -> lock);
        return NULL;
    }

    if(d -> top == NULL && d -> bottom == NULL && d -> size == 0) {
        pthread_mutex_unlock(&d -> lock);
        return NULL;
    }

    struct node * node = d -> top;

    if(node -> next == NULL) {
        d -> bottom = NULL;
        d -> top = NULL;
    }

    else {
        node -> next -> prev = NULL;
        d -> top = node -> next;
        node -> next = NULL;
    }
    
    struct task * data = node -> task;

    free(node);

    d -> size = d -> size - 1;

    pthread_mutex_unlock(&d -> lock);

    return data;
}