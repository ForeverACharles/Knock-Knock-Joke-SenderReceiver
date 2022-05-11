CFLAGS = -fsanitize=address -std=c99 -Wall -Werror 

all:
	gcc -g -std=c99 -Wall -fsanitize=address server.c server.h -o KKJserver  -lm
clean:
	rm KKJserver