CC = gcc
CFLAGS = -Wall -std=c99
CNAME = library
LIBNAME = searchlib
FOLDERNAME = Libs
OUTPUT = test

all: static shared dll rename

static:
	$(CC) $(CFLAGS) main.c -o $(OUTPUT) -L./$(FOLDERNAME) ./$(FOLDERNAME)/lib$(LIBNAME).a
	echo 'STARTING STATIC TESTS:\n\n' >> results3a
	make tests

shared:
	$(CC) $(CFLAGS) main.c -o $(OUTPUT) -L./$(FOLDERNAME) -l$(LIBNAME) -Wl,-rpath=./$(FOLDERNAME)
	echo '\n\nSTARTING DYNAMIC-SHARED TESTS:\n\n' >> results3a
	make tests

dll:	
	$(CC) $(CFLAGS) main.c -o $(OUTPUT) -L./$(FOLDERNAME) -ldl -Wl,-rpath=./$(FOLDERNAME) -D DLL
	echo '\n\nSTARTING DYNAMIC-DLL TESTS:\n\n' >> results3a
	make tests
	
tests:
	./$(OUTPUT) --create 2 --dir /bin --searchf ss --temporaryf tmp.txt --find >> results3a
	./$(OUTPUT) --create 3 --dir / --searchf passwd --temporaryf tmp2.txt --find >> results3a
	./$(OUTPUT) --create 2 --dir / --searchf bin --temporaryf tmp3.txt --find >> results3a
	./$(OUTPUT) --create 2 --dir / --searchf Findme --temporaryf tmp4.txt --find >> results3a
	./$(OUTPUT) --create 5 --temporaryf tmp.txt --add >> results3a
	./$(OUTPUT) --create 5 --temporaryf tmp2.txt --add >> results3a
	./$(OUTPUT) --create 5 --temporaryf tmp3.txt --add >> results3a
	./$(OUTPUT) --create 5 --temporaryf tmp.txt --add --temporaryf tmp2.txt --add --temporaryf tmp3.txt --add --remove 0 --remove 1 --remove 2 >> results3a
	./$(OUTPUT) --create 5000 --temporaryf tmp.txt --add --temporaryf tmp2.txt --add --temporaryf tmp3.txt --add --add --add --add --add --add --add --add --add --add --add  >> results3a
	./$(OUTPUT) --create 1 --temporaryf tmp4.txt --add --remove 0 --add --remove 0 --add --remove 0 --add --remove 0 --add --remove 0 --add --remove 0 --add --remove 0 --add --remove 0 >> results3a
	make clean

clean:
	rm $(OUTPUT)
	rm -f *.txt

rename:
	mv results3a results3a.txt 


