CC = gcc
CFLAGS = -Wall -std=c99
CNAME = library
LIBNAME = searchlib
FOLDERNAME = Libs
OUTPUT = test


all: O0 Os O3 rename


O0:	
	echo 'STARTING O0 OPTIMALIZATION TESTS:\n\n' >> results3b	
	$(CC) -O0 $(CFLAGS) main.c -o $(OUTPUT) -L./$(FOLDERNAME) ./$(FOLDERNAME)/lib$(LIBNAME).a
	echo 'STARTING STATIC TESTS:\n\n' >> results3b
	make tests
	$(CC) -O0 $(CFLAGS) main.c -o $(OUTPUT) -L./$(FOLDERNAME) -l$(LIBNAME) -Wl,-rpath=./$(FOLDERNAME)
	echo '\n\nSTARTING DYNAMIC-SHARED TESTS:\n\n' >> results3b
	make tests
	$(CC) -O0 $(CFLAGS) main.c -o $(OUTPUT) -L./$(FOLDERNAME) -ldl -Wl,-rpath=./$(FOLDERNAME) -D DLL
	echo '\n\nSTARTING DYNAMIC-DLL TESTS:\n\n' >> results3b
	make tests

Os:	
	echo '\n\nSTARTING Os OPTIMALIZATION TESTS:\n\n' >> results3b	
	$(CC) -Os $(CFLAGS) main.c -o $(OUTPUT) -L./$(FOLDERNAME) ./$(FOLDERNAME)/lib$(LIBNAME).a
	echo 'STARTING STATIC TESTS:\n\n' >> results3b
	make tests
	$(CC) -Os $(CFLAGS) main.c -o $(OUTPUT) -L./$(FOLDERNAME) -l$(LIBNAME) -Wl,-rpath=./$(FOLDERNAME)
	echo '\n\nSTARTING DYNAMIC-SHARED TESTS:\n\n' >> results3b
	make tests
	$(CC) -Os $(CFLAGS) main.c -o $(OUTPUT) -L./$(FOLDERNAME) -ldl -Wl,-rpath=./$(FOLDERNAME) -D DLL
	echo '\n\nSTARTING DYNAMIC-DLL TESTS:\n\n' >> results3b
	make tests

O3:	
	echo '\n\nSTARTING O3 OPTIMALIZATION TESTS:\n\n' >> results3b	
	$(CC) -O3 $(CFLAGS) main.c -o $(OUTPUT) -L./$(FOLDERNAME) ./$(FOLDERNAME)/lib$(LIBNAME).a
	echo 'STARTING STATIC TESTS:\n\n' >> results3b
	make tests
	$(CC) -O3 $(CFLAGS) main.c -o $(OUTPUT) -L./$(FOLDERNAME) -l$(LIBNAME) -Wl,-rpath=./$(FOLDERNAME)
	echo '\n\nSTARTING DYNAMIC-SHARED TESTS:\n\n' >> results3b
	make tests
	$(CC) -O3 $(CFLAGS) main.c -o $(OUTPUT) -L./$(FOLDERNAME) -ldl -Wl,-rpath=./$(FOLDERNAME) -D DLL
	echo '\n\nSTARTING DYNAMIC-DLL TESTS:\n\n' >> results3b
	make tests
	
tests:
	./$(OUTPUT) --create 2 --dir /bin --searchf ss --temporaryf tmp.txt --find >> results3b
	./$(OUTPUT) --create 3 --dir / --searchf passwd --temporaryf tmp2.txt --find >> results3b
	./$(OUTPUT) --create 2 --dir / --searchf bin --temporaryf tmp3.txt --find >> results3b
	./$(OUTPUT) --create 2 --dir / --searchf Findme --temporaryf tmp4.txt --find >> results3b
	./$(OUTPUT) --create 5 --temporaryf tmp.txt --add >> results3b
	./$(OUTPUT) --create 5 --temporaryf tmp2.txt --add >> results3b
	./$(OUTPUT) --create 5 --temporaryf tmp3.txt --add >> results3b
	./$(OUTPUT) --create 5 --temporaryf tmp.txt --add --temporaryf tmp2.txt --add --temporaryf tmp3.txt --add --remove 0 --remove 1 --remove 2 >> results3b
	./$(OUTPUT) --create 5000 --temporaryf tmp.txt --add --temporaryf tmp2.txt --add --temporaryf tmp3.txt --add --add --add --add --add --add --add --add --add --add --add  >> results3b
	./$(OUTPUT) --create 1 --temporaryf tmp4.txt --add --remove 0 --add --remove 0 --add --remove 0 --add --remove 0 --add --remove 0 --add --remove 0 --add --remove 0 --add --remove 0 >> results3b
	make clean

clean:
	rm $(OUTPUT)
	rm -f *.txt

rename:
	mv results3b results3b.txt 


