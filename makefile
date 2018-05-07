tp:
	gcc Cliente.c -o cliente -lpthread
	gcc Servidor.c -o servidor -lpthread

clean:
	rm cliente servidor
