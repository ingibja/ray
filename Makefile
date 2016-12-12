
CFLAGS=-g

top : tl

tt : travelt 
	./travelt -d 3 -x 15

tp : phases 
	./phases -r

tr : 
	cc -g -DTEST readdata.c
	a.out

P =  ray.o readdata.o distance.o stations.o
phases : $P phases.c
	cc -DTEST phases.c $P -o phases -lproj -lm

tl : locate
	locate 

L = $P phases.o golubc.o
locate : $L locate.c
	cc -o locate  -DTEST $L locate.c -lproj -lm

T = travelt.o ray.o

$T : ray.h

travelt : $T
	cc ${CFLAGS}  -o travelt $T -lm

t : ray
	./ray

ray : ray.c  ray.h
	cc -g -DDEBUG -o ray ray.c -lm

t22 : travelt
	travelt -n 0 -l 9 -d 7.3 -x 22.1 > jt22  2>&1

t14 : travelt
	travelt -n 0 -l 9  -x 14.6 -d 5.6 > jtest  2>&1

clean : 
	rm *.o travelt ray


