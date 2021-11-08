app: moltiplicazione.o somma.o app.o
	gcc $(CFLAGS) somma.o moltiplicazione.o app.o -o app
app.o: app.c
	gcc -c app.c
moltiplicazione.o: moltiplicazione.c moltiplicazione.h
	gcc -c moltiplicazione.c
somma.o: somma.c somma.h
	gcc -c somma.c
clean:
	rm -f *.o
cleanApp:
	rm app
cleanAll:
	rm -f *.o | rm app
run:
	./app
