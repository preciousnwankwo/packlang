CC = cc
CFLAGS = -Wall -Wextra -std=c99

packc: packc.c
	$(CC) $(CFLAGS) -o packc packc.c

clean:
	rm -f packc

test: packc
	./packc examples/hello.pack | head -1
	./packc --emit-c examples/hello.pack | gcc -x c - -o /tmp/packtest
	/tmp/packtest; echo "exit: $$?"
	./packc --emit-c examples/conditional.pack | gcc -x c - -o /tmp/packtest
	/tmp/packtest; echo "exit: $$?"
	./packc --emit-c examples/while.pack | gcc -x c - -o /tmp/packtest 2>/dev/null
	/tmp/packtest; echo "exit: $$?"
	./packc --emit-c examples/scope.pack | gcc -x c - -o /tmp/packtest 2>/dev/null
	/tmp/packtest; echo "exit: $$?"
	./packc --emit-c examples/fn.pack | gcc -x c - -o /tmp/packtest 2>/dev/null
	/tmp/packtest; echo "exit: $$?"
	./packc --emit-c examples/typed.pack | gcc -x c - -o /tmp/packtest 2>/dev/null
	/tmp/packtest; echo "exit: $$?"
	./packc --emit-c examples/str.pack | gcc -x c - -o /tmp/packtest 2>/dev/null
	/tmp/packtest; echo "exit: $$?"
	./packc --emit-c examples/arr.pack | gcc -x c - -o /tmp/packtest 2>/dev/null
	/tmp/packtest; echo "exit: $$?"
