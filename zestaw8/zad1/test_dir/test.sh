 #!/bin/bash

echo 'STARTING TESTING' > ./result/Times.txt
touch result/dummy.pgm
gcc ./test_dir/filter.c -o ./test_dir/filter.out
for thread in 1 2 4 8; do
	for c in 3 10 20 26 39 48 65; do
		for mode in "block" "interleaved"; do
			touch filter/ct$c
			./test_dir/filter.out "./filter/ct$c" "$c" # generate filter | params: path_to_file filter_size
			./main.out $thread $mode ./images/brain.pgm ./filter/ct$c ./result/dummy.pgm >> ./result/Times.txt
		done
    done
done
rm ./result/dummy.pgm
rm ./test_dir/filter.out
rm ./filter/ct*
echo 'ALL TESTS DONE' >> ./result/Times.txt
