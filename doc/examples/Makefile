LIBS=-ldrawtk

all: bounce bars errormon

bounce: bounce.c
	$(CC) $(CFLAGS) -o $@ $^ $(LIBS)

bars: bars.c
	$(CC) $(CFLAGS) -o $@ $^ $(LIBS)

errormon: errormon.c
	$(CC) $(CFLAGS) -o $@ $^ $(LIBS)

clean:
	$(RM) bounce bars errormon
