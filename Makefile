CFLAGS=-std=c11 -g -static

64cc: 64cc.c

test: 64cc
	./test.sh

clean:
	rm -f 64cc *.o *~ tmp*

.PHONY: test clean