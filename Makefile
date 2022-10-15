# Cコンパイラに渡すコマンドラインオプションを記述
CFLAGS=-std=c11 -g -static
SRCS=$(wildcard *.c)
# SRCの中の.cを.oに置換した値を生成
OBJS=$(SRCS:.c=.o)

64cc: $(OBJS)
	$(CC) -o 64cc $(OBJS) $(LDFLAGS)

# すべての.oファイルが64cc.hに依存
$(OBJS): 64cc.h

test: 64cc
	./test.sh

clean:
	rm -f 64cc *.o *~ tmp*

.PHONY: test clean