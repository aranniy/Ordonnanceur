#ifndef DEQUE_H
#define DEQUE_H

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

struct task {
    void * function;
    void * args;
};

struct node {
    struct task * task;
    struct node* next;
    struct node* prev;
};

struct deque {
    int id;
    struct node * top;
    struct node * bottom;
    int size;
    int qlen;
    pthread_mutex_t lock;
};

struct deque * init(int id, int qlen);
int isEmpty(struct deque *d);
int addBottom(struct deque *d, void * function, void * args);
int addTop(struct deque *d, void * function, void * args);
int getSize(struct deque *d);
struct task * getBottom(struct deque *d);
struct task * getTop(struct deque *d);


#endif