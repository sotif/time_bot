CONCORD	= ./concord
CC	= gcc
OUT	= bot

CORE_DIR	= $(CONCORD)/core
INCLUDE_DIR	= $(CONCORD)/include
GENCODECS_DIR	= $(CONCORD)/gencodecs


CFLAGS += -O0 -g -pthread -Wall \
	  -I$(INCLUDE_DIR) -I$(CORE_DIR) -I$(GENCODECS_DIR)

LDFLAGS	= -L$(CONCORD)/lib
LDLIBS	= -ldiscord -lcurl -pthread

OBJS	= bot.o

all: lib $(OBJS)
	$(CC) $(OBJS) $(LDFLAGS) $(LDLIBS) -o $(OUT)

clean:
	rm -rf bot *.o *.log
lib:
	$(MAKE) -C $(CONCORD)
