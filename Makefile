all:
	gcc -lX11 main.c -o chibiwm

clean:
	rm -f chibiwm

install:
	sudo cp chibiwm /usr/bin/chibiwm
