CC=gcc
CFLAGS= -g3 -O2 -Wall
LIBS = -lpcap -lpthread -lncurses
DFLAGS= -DEXPORT_TO_FILE 

OBJS= main.o ip_process.o tcpconnection.o display_tcp_conn.o
EXEC=tcpconn
%.o:%.c
	$(CC) $(CFLAGS) $(DFLAGS) -c $< -o $@
$(EXEC): $(OBJS)
	$(CC) $(CFLAGS) $(DFLAGS) $(OBJS) $(LIBS) -o $@
clean:
	rm -rf $(EXEC) $(OBJS)


