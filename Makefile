CC = cc
CFLAGS = -Wall -Wextra -std=c99

packc: packc.c
	$(CC) $(CFLAGS) -o packc packc.c

clean:
	rm -f packc

test: packc
	./packc examples/hello.pack
