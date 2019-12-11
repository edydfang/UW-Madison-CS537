fscheck : fscheck.c 
	gcc -o fscheck fscheck.c
	chmod +x fscheck
clean :
	rm -f fscheck.o

test: fscheck
	bash test.sh


