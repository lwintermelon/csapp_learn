CFLAGS+=-I ../
LDFLAGS+=
LIBS+=-lpthread

obj+=../csapp.o

srcs=$(wildcard *.c)
objs=$(srcs:%.c=%.o)
targets=$(srcs:%.c=%)

all:tiny adder

target=tiny
$(target) : $(target).o $(obj)
	$(CC) $^ -o $@ $(LDFLAGS) $(LIBS)

adder:./cgi-bin/adder.o
	$(CC) $^ -o ./cgi-bin/adder $(LDFLAGS) $(LIBS)
	
.PHONY: clean
clean:
	rm -f $(objs) $(targets) ./cgi-bin/adder ./cgi-bin/adder.o
