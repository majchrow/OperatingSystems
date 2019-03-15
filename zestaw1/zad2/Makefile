CC = gcc
CFLAGS = -Wall -std=c99
CNAME = library
LIBNAME = searchlib
FOLDERNAME = Libs
OUTPUT = test

all: compile tests clean

compile:
	$(CC) $(CFLAGS) main.c -o $(OUTPUT) -L./$(FOLDERNAME) -l$(LIBNAME) -Wl,-rpath=./$(FOLDERNAME)

tests:
	./$(OUTPUT) --create 2 --dir /bin --searchf ss --temporaryf tmp.txt --find >> raport2
	./$(OUTPUT) --create 3 --dir / --searchf passwd --temporaryf tmp2.txt --find >> raport2
	./$(OUTPUT) --create 2 --dir / --searchf bin --temporaryf tmp3.txt --find >> raport2
	./$(OUTPUT) --create 2 --dir / --searchf Findme --temporaryf tmp4.txt --find >> raport2
	./$(OUTPUT) --create 5 --temporaryf tmp.txt --add >> raport2
	./$(OUTPUT) --create 5 --temporaryf tmp2.txt --add >> raport2
	./$(OUTPUT) --create 5 --temporaryf tmp3.txt --add >> raport2
	./$(OUTPUT) --create 5 --temporaryf tmp.txt --add --temporaryf tmp2.txt --add --temporaryf tmp3.txt --add --remove 0 --remove 1 --remove 2 >> raport2
	./$(OUTPUT) --create 5000 --temporaryf tmp.txt --add --temporaryf tmp2.txt --add --temporaryf tmp3.txt --add --add --add --add --add --add --add --add --add --add --add  >> raport2
	./$(OUTPUT) --create 1 --temporaryf tmp4.txt --add --remove 0 --add --remove 0 --add --remove 0 --add --remove 0 --add --remove 0 --add --remove 0 --add --remove 0 --add --remove 0 >> raport2
	
clean:
	rm $(OUTPUT)
	rm -f *.txt
	mv raport2 raport2.txt 

