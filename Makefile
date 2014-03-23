

all: rm dv dump

rm: rm.o util.o
	gcc $^ -o rm
	
dv: dv.o util.o
	gcc $^ -o dv
	
dump: dump.o util.o
	gcc $^ -o dump
	
rm.o: rm.c rm.h util.h
	gcc -c rm.c
	
dv.o: dv.c dv.h util.h
	gcc -c dv.c
	
dump.o: dump.c dump.h util.h
	gcc -c dump.c

util.o: util.c util.h
	gcc -c util.c
	
clean:
	rm -f *.o rm dv dump