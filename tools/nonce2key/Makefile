VPATH = ../../common/crapto1
CC = gcc
LD = gcc
CFLAGS = -I../../common -DSTANDALONE_TOOL -Wall -O4
LDFLAGS =

OBJS = crypto1.o crapto1.o
HEADERS = crapto1.h
EXES = nonce2key

all: $(OBJS) $(EXES)

%.o : %.c
	$(CC) $(CFLAGS) -c -o $@ $<

% : %.c
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $(OBJS) $<

clean: 
	rm -f $(OBJS) $(EXES)
