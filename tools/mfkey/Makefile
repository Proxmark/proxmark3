VPATH = ../../common ../../common/crapto1 ../../client
CC = gcc
LD = gcc
CFLAGS = -I../../common -I../../client -Wall -O4
LDFLAGS =

OBJS = crypto1.o crapto1.o parity.o util.o mfkey.o
EXES = mfkey32 mfkey64
WINEXES = $(patsubst %, %.exe, $(EXES))

all: $(OBJS) $(EXES)

%.o : %.c
	$(CC) $(CFLAGS) -c -o $@ $<

% : %.c $(OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $(OBJS) $<

clean: 
	rm -f $(OBJS) $(EXES) $(WINEXES)
