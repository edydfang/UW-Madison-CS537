wordcount: wordcount.c mapreduce.c
	gcc -o wordcount wordcount.c mapreduce.c -lpthread
	chmod +x wordcount

gdb:wordcount.c mapreduce.c
	gcc -g -o wordcount wordcount.c mapreduce.c -lpthread
	chmod +x wordcount
	gdb --args ./wordcount test1.txt test2.txt

test:wordcount
	./wordcount test1.txt test2.txt
