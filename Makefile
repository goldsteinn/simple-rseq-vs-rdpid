all:
	gcc -static  -O3 -march=native  -mtune=native test_rseq.c -o test_rseq -lrseq -ldl

clean:
	rm *.o *~ test_rseq
