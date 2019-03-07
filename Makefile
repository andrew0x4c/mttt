all:
	gcc -Wall -O3 --std=c99 mttt.c -o mttt

4:
	gcc -Wall -O3 --std=c99 -DN=4 mttt.c -o mttt4

5:
	gcc -Wall -O3 --std=c99 -DN=5 mttt.c -o mttt5

multi: all 4 5

clean:
	rm -f mttt
	rm -f mttt4
	rm -f mttt5
