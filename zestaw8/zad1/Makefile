CC = gcc 
CFLAGS = -Wall -lpthread -lm
MAIN = main.c
MAIN_OUT = main.out

all: compile tests clean

compile:
	$(CC) $(MAIN) $(CFLAGS) -o $(MAIN_OUT) 
	
tests:	
	touch result/block_filtr.pgm result/inter_filtr.pgm # create empty files if not exists
	./$(MAIN_OUT) 4 block       images/brain.pgm filter/c3_block result/block_filtr.pgm # (Number of threads) (block/interleaved) (Input image file name) (Input filter file name) (Output file name)
	./$(MAIN_OUT) 8 interleaved images/brain.pgm filter/c9_inter result/inter_filtr.pgm # (Number of threads) (block/interleaved) (Input image file name) (Input filter file name) (Output file name)
# 	./test_dir/test.sh # Used for generate Times.txt (some of the main.c printf was commented to improve readability)
clean:
	rm *.out 
	