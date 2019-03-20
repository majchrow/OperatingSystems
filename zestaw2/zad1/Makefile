CC = gcc
CFLAGS = -Wall -std=c11
MAIN = main.c
OUT = main.out
APPEND = >> komentarz

all: compile sort_test copy_test clean 

compile:
	$(CC) $(CFLAGS) $(MAIN) -o $(OUT)

sort_test:
	make generate_sort
	make copy
	./$(OUT) sort tmp1.txt 5000 1 lib $(APPEND)
	./$(OUT) sort tmp1cp.txt 5000 1 sys $(APPEND)
	./$(OUT) sort tmp1_2.txt 2000 1 lib $(APPEND)
	./$(OUT) sort tmp1cp_2.txt 2000 1 sys $(APPEND)
	./$(OUT) sort tmp2.txt 5000 4 lib $(APPEND)
	./$(OUT) sort tmp2cp.txt 5000 4 sys $(APPEND)
	./$(OUT) sort tmp2_2.txt 2000 4 lib $(APPEND)
	./$(OUT) sort tmp2cp_2.txt 2000 4 sys $(APPEND)
	./$(OUT) sort tmp512.txt 5000 512 lib $(APPEND)
	./$(OUT) sort tmp512cp.txt 5000 512 sys $(APPEND)
	./$(OUT) sort tmp512_2.txt 2000 512 lib $(APPEND)		
	./$(OUT) sort tmp512cp_2.txt 2000 512 sys $(APPEND)
	./$(OUT) sort tmp1024.txt 5000 1024 lib $(APPEND)
	./$(OUT) sort tmp1024cp.txt 5000 1024 sys $(APPEND)
	./$(OUT) sort tmp1024_2.txt 2000 1024 lib $(APPEND)
	./$(OUT) sort tmp1024cp_2.txt 2000 1024 sys $(APPEND)
	./$(OUT) sort tmp4096.txt 5000 4096 lib $(APPEND)
	./$(OUT) sort tmp4096cp.txt 5000 4096 sys $(APPEND)
	./$(OUT) sort tmp4096_2.txt 2000 4096 lib $(APPEND)
	./$(OUT) sort tmp4096cp_2.txt 2000 4096 sys $(APPEND)
	./$(OUT) sort tmp8192.txt 5000 8192 lib $(APPEND)
	./$(OUT) sort tmp8192cp.txt 5000 8192 sys $(APPEND)
	./$(OUT) sort tmp8192_2.txt 2000 8192 lib $(APPEND)
	./$(OUT) sort tmp8192cp_2.txt 2000 8192 sys $(APPEND)	
	make clean_test

copy_test:
	make generate_copy
	./$(OUT) copy tmp1.txt tmp1lib.txt 500000 1 lib $(APPEND)
	./$(OUT) copy tmp1.txt tmp1sys.txt 500000 1 sys $(APPEND)
	./$(OUT) copy tmp1_2.txt tmp1lib_2.txt 2000000 1 lib $(APPEND)
	./$(OUT) copy tmp1_2.txt tmp1sys_2.txt 2000000 1 sys $(APPEND)
	./$(OUT) copy tmp2.txt tmp2lib.txt 500000 4 lib $(APPEND)
	./$(OUT) copy tmp2.txt tmp2sys.txt 500000 4 sys $(APPEND)
	./$(OUT) copy tmp2_2.txt tmp2lib_2.txt 2000000 4 lib $(APPEND)
	./$(OUT) copy tmp2_2.txt tmp2sys_2.txt 2000000 4 sys $(APPEND)
	./$(OUT) copy tmp512.txt tmp512lib.txt 500000 512 lib $(APPEND)
	./$(OUT) copy tmp512.txt tmp512sys.txt 500000 512 sys $(APPEND)
	./$(OUT) copy tmp512_2.txt tmp512lib_2.txt 2000000 512 lib $(APPEND)
	./$(OUT) copy tmp512_2.txt tmp512sys_2.txt 2000000 512 sys $(APPEND)
	./$(OUT) copy tmp1024.txt tmp1024lib.txt 500000 1024 lib $(APPEND)
	./$(OUT) copy tmp1024.txt tmp1024sys.txt 500000 1024 sys $(APPEND)
	./$(OUT) copy tmp1024_2.txt tmp1024lib_2.txt 1000000 1024 lib $(APPEND)
	./$(OUT) copy tmp1024_2.txt tmp1024sys_2.txt 1000000 1024 sys $(APPEND)
	./$(OUT) copy tmp4096.txt tmp4096lib.txt 100000 4096 lib $(APPEND)
	./$(OUT) copy tmp4096.txt tmp4096sys.txt 100000 4096 sys $(APPEND)
	./$(OUT) copy tmp4096_2.txt tmp4096lib_2.txt 150000 4096 lib $(APPEND)
	./$(OUT) copy tmp4096_2.txt tmp4096sys_2.txt 150000 4096 sys $(APPEND)
	./$(OUT) copy tmp8192.txt tmp8192lib.txt 30000 8192 lib $(APPEND)
	./$(OUT) copy tmp8192.txt tmp8192sys.txt 30000 8192 sys $(APPEND)
	./$(OUT) copy tmp8192_2.txt tmp8192lib_2.txt 60000 8192 lib $(APPEND)
	./$(OUT) copy tmp8192_2.txt tmp8192sys_2.txt 60000 8192 sys $(APPEND)
	make clean_test

generate_sort:
	./$(OUT) generate tmp1.txt 5000 1 lib
	./$(OUT) generate tmp2.txt 5000 4 lib
	./$(OUT) generate tmp512.txt 5000 512 lib
	./$(OUT) generate tmp1024.txt 5000 1024 lib
	./$(OUT) generate tmp4096.txt 5000 4096 lib
	./$(OUT) generate tmp8192.txt 5000 8192 lib
	./$(OUT) generate tmp1_2.txt 2000 1 sys
	./$(OUT) generate tmp2_2.txt 2000 4 sys
	./$(OUT) generate tmp512_2.txt 2000 512 sys
	./$(OUT) generate tmp1024_2.txt 2000 1024 sys
	./$(OUT) generate tmp4096_2.txt 2000 4096 sys
	./$(OUT) generate tmp8192_2.txt 2000 8192 sys

copy:
	cp tmp1.txt tmp1cp.txt
	cp tmp2.txt tmp2cp.txt
	cp tmp512.txt tmp512cp.txt
	cp tmp1024.txt tmp1024cp.txt
	cp tmp4096.txt tmp4096cp.txt
	cp tmp8192.txt tmp8192cp.txt
	cp tmp1_2.txt tmp1cp_2.txt
	cp tmp2_2.txt tmp2cp_2.txt
	cp tmp512_2.txt tmp512cp_2.txt
	cp tmp1024_2.txt tmp1024cp_2.txt
	cp tmp4096_2.txt tmp4096cp_2.txt
	cp tmp8192_2.txt tmp8192cp_2.txt

clean_test:
	rm tmp*	

generate_copy:
	./$(OUT) generate tmp1.txt 500000 1 lib
	./$(OUT) generate tmp2.txt 500000 4 lib
	./$(OUT) generate tmp512.txt 500000 512 lib
	./$(OUT) generate tmp1024.txt 500000 1024 lib
	./$(OUT) generate tmp4096.txt 100000 4096 lib
	./$(OUT) generate tmp8192.txt 30000 8192 lib
	./$(OUT) generate tmp1_2.txt 2000000 1 sys
	./$(OUT) generate tmp2_2.txt 2000000 4 sys
	./$(OUT) generate tmp512_2.txt 2000000 512 sys
	./$(OUT) generate tmp1024_2.txt 1000000 1024 sys
	./$(OUT) generate tmp4096_2.txt 150000 4096 sys
	./$(OUT) generate tmp8192_2.txt 60000 8192 sys

clean:
	rm $(OUT)
	echo 'Sortowanie oraz kopiowanie korzystające z funkcji systemowych działa szybciej niż korzystające z funkcji bibliotecznych dla dużych rozmiarów plików(dużych czasów).' $(APPEND)
	echo 'Dla małych rozmiarów plików jest odwrotnie, funkcje biblioteczne działają dużo szybciej.' $(APPEND)
	mv komentarz komentarz.txt	
