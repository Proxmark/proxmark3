VPATH = ../../common/crapto1
CC = gcc
LD = gcc
CFLAGS = -I../../common -Wall -O4
LDFLAGS =

OBJS = crypto1.o crapto1.o
EXES = mfkey32 mfkey64
WINEXES = $(patsubst %, %.exe, $(EXES))

all: $(OBJS) $(EXES)

%.o : %.c
	$(CC) $(CFLAGS) -c -o $@ $<

% : %.c
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $(OBJS) $<

clean: 
	rm -f $(OBJS) $(EXES) $(WINEXES)
