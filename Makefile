CC= clang
CFLAGS= -ggdb -O  -Wall -Werror -D_DEBUG
LIBS= -lsqlite3

rstat: rstat.o ruser.o rman.o rrun.o
	$(CC) $(CFLAGS) $(LIBS) -o rstat $^ 

rstat.o: rstat.h rstat.c
	$(CC) $(CFLAGS) -c rstat.c

ruser.o: rstat.h ruser.c
	$(CC) $(CFLAGS) -c ruser.c

rman.o: rstat.h rman.c
	$(CC) $(CFLAGS) -c rman.c

rrun.o: rstat.h rrun.c
	$(CC) $(CFLAGS) -c rrun.c

clean: 
	rm -f rstat
	rm -f *.o
	rm -f test.db
