CC= clang
CFLAGS= -ggdb -O  -Wall -Werror
LIBS= -lsqlite3

rstat: rstat.o ruser.o rman.o
	$(CC) $(CFLAGS) $(LIBS) -o rstat $^ 

rstat.o: rstat.h rstat.c
	$(CC) $(CFLAGS) -c rstat.c

ruser.o: rstat.h ruser.c
	$(CC) $(CFLAGS) -c ruser.c

rman.o: rstat.h rman.c
	$(CC) $(CFLAGS) -c rman.c

clean: 
	rm -f rstat
	rm -f *.o
