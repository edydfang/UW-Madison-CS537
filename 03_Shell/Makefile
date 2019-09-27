all: mysh

mysh: mysh.c
	gcc mysh.c -o mysh
	chmod +x mysh

test: mysh
	./mysh arg1 arg2 || true
	./mysh arg1 || true
	./mysh test.sh
