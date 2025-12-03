all: main

main: try2.c
	cc -Werror -Wextra -Wall try2.c -o $@

clean: 
	rm main

re: clean main


