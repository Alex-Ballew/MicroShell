myshell: myshell.o argparse.o builtin.o
		 gcc -g -o myshell myshell.o argparse.o builtin.o -Werror -Wall

myshell.o: myshell.c argparse.h builtin.h
		gcc -g -c myshell.c -Werror -Wall

argparse.o: argparse.c argparse.h
		gcc -g -c argparse.c -Werror -Wall

builtin.o: builtin.c builtin.h
		gcc -c -g builtin.c -Werror -Wall

clean:
		rm -f *.o *~* *.exe