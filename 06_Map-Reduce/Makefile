wordcount: wordcount.c mapreduce.c
	gcc -g -o wordcount wordcount.c mapreduce.c -lpthread
	chmod +x wordcount

sort:sort.c mapreduce.c
	gcc -g -o sort sort.c mapreduce.c -lpthread
	chmod +x sort

gdb-wordcount:wordcount.c mapreduce.c
	gcc -g -o wordcount wordcount.c mapreduce.c -lpthread
	chmod +x wordcount
	gdb --args ./wordcount test1.txt test2.txt 10 10 10

test-wordcount:wordcount
	./wordcount test1.txt test2.txt 10 10 10

gdb-sort:sort.c mapreduce.c
	gcc -g -o sort sort.c mapreduce.c -lpthread
	chmod +x sort
	gdb --args ./sort  tests/1.txt tests/2.txt tests/3.txt 3 1 4

test-sort:sort
	./sort  tests/1.txt tests/2.txt tests/3.txt 3 1 4

valgrind:wordcount sort
	valgrind --track-origins=yes --leak-check=full --show-leak-kinds=all ./wordcount test1.txt test2.txt 10 10 10
	valgrind --track-origins=yes --leak-check=full --show-leak-kinds=all ./sort  tests/1.txt tests/2.txt tests/3.txt 3 1 4
