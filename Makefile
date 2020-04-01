all: break-blktrace

break-blktrace: break-blktrace.c
	gcc -o break-blktrace break-blktrace.c -Wall

clean:
	rm -f break-blktrace
