CFLAGS+=
LDFLAGS+=
LIBS+=-lpthread

obj+=csapp.o

srcs=$(wildcard *.c)
objs=$(srcs:%.c=%.o)
targets=$(srcs:%.c=%)


all : $(target)

target?=waitSig
$(target) : $(target).o $(obj)
	$(CC) $^ -o $@ $(LDFLAGS) $(LIBS)

.PHONY: clean
clean:
	rm -f $(objs) $(targets)
