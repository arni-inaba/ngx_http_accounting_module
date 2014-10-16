CC=gcc-4.7

test: build
	$(CC) test_accounting_id.o ngx_http_accounting_prefix.o -o ./test
	./test

build: test_accounting_id.c
	$(CC) -DTESTING -c test_accounting_id.c -o test_accounting_id.o
	$(CC) -DTESTING -c ../src/ngx_http_accounting_prefix.c

clean:
	rm -f ./test
	rm -f *.o
	rm -f ../src/ngx_http_accounting_prefix.o
