LIBS=-ldrawtk

all: bounce bars

bounce: bounce.c
	$(CC) $(CFLAGS) -o $@ $^ $(LIBS)

bars: bars.c
	$(CC) $(CFLAGS) -o $@ $^ $(LIBS)

clean:
	$(RM) bounce bars
