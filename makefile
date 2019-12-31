all: control.o write.o
	gcc -o control control.o
	gcc -o write write.o
write.o: write.c
	gcc -c write.c
control.o: control.c
	gcc -c control.c
clean:
	rm "control"
	rm "write"
	rm *.o
