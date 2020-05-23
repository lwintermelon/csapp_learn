CFLAGS+=-I../ -g
LDFLAGS+=-g
LIBS+=-lpthread

obj+=../csapp.o

srcs=$(wildcard *.c)
objs=$(srcs:%.c=%.o)
targets=$(srcs:%.c=%)


all : $(target)

target?=waitSig
$(target) : $(target).o $(obj)
	$(CC) $^ -o $@ $(LDFLAGS) $(LIBS)

kqueue.o : kqueue.c
	$(CC) -c $^ -o $@ -I/usr/include/kqueue -I../

kqueue : kqueue.o
	$(CC) $^ -o $@ $(LDFLAGS) $(LIBS) -lkqueue

.PHONY: clean
clean:
	rm -f $(objs) $(targets)
