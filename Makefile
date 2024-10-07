CC = gcc
CCO = $(CC) -O2 -Wall -c $<

OBJECTS = quicksort quicksort2

all : 
	make quicksort
	make quicksort2

deque.o: projet/src/deque.c projet/include/deque.h
	$(CCO) 

sched.o: projet/src/sched.c projet/include/sched.h
	$(CCO) 

sched2.o: projet/src/sched2.c projet/include/sched.h
	$(CCO) 

quicksort.o: projet/quicksort.c projet/include/sched.h
	$(CCO)

quicksort : quicksort.o sched.o
	$(CC) -O2 -o quicksort quicksort.o sched.o
	rm -f *.o

quicksort2 : quicksort.o sched2.o deque.o
	$(CC) -O2 -o quicksort2 quicksort.o sched2.o deque.o
	rm -f *.o

clean:
	rm -f $(OBJECTS)