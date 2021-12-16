sem_00_gen:	bin_sem.o sem_00_gen.o
	gcc bin_sem.o sem_00_gen.o -o sem_00_gen

sem_00_gen.o: bin_sem_head.h sem_00_gen.c
	gcc -c sem_00_gen.c

sem_01_use:	bin_sem.o sem_01_use.o
	gcc bin_sem.o sem_01_use.o -o sem_01_use

sem_01_use.o: bin_sem_head.h sem_01_use.c
	gcc -c sem_01_use.c

sem_02_use:	bin_sem.o sem_02_use.o
	gcc bin_sem.o sem_02_use.o -o sem_02_use

sem_02_use.o: bin_sem_head.h sem_02_use.c
	gcc -c sem_02_use.c

sem_03_gen: sem_03_gen.o
	gcc sem_03_gen.o -o sem_03_gen

sem_03_gen.o: sem_03_gen.c
	gcc -c sem_03_gen.c

sem_padre_figlio: bin_sem.o sem_padre_figlio.o
	gcc bin_sem.o sem_padre_figlio.o -o sem_padre_figlio

sem_padre_figlio.o: bin_sem_head.h sem_padre_figlio.c
	gcc -c sem_padre_figlio.c

sem_proc_coop_1: bin_sem.o sem_proc_coop_1.o
	gcc bin_sem.o sem_proc_coop_1.o -o sem_proc_coop_1

sem_proc_coop_1.o: bin_sem_head.h sem_proc_coop_1.c
	gcc -c sem_proc_coop_1.c

sem_proc_coop_2: bin_sem.o sem_proc_coop_2.o
	gcc bin_sem.o sem_proc_coop_2.o -o sem_proc_coop_2

sem_proc_coop_2.o: bin_sem_head.h sem_proc_coop_2.c
	gcc -c sem_proc_coop_2.c

sem_padre_3_figli: bin_sem.o sem_padre_3_figli.o
	gcc bin_sem.o sem_padre_3_figli.o -o sem_padre_3_figli

sem_padre_3_figli.o: bin_sem_head.h sem_padre_3_figli.c
	gcc -c sem_padre_3_figli.c

sem_padre_figlio_notify: bin_sem.o sem_padre_figlio_notify.o
	gcc bin_sem.o sem_padre_figlio_notify.o -o sem_padre_figlio_notify

sem_padre_figlio_notify.o: bin_sem_head.h sem_padre_figlio_notify.c
	gcc -c sem_padre_figlio_notify.c

bin_sem.o: bin_sem_head.h bin_sem.c
	gcc -c bin_sem.c

clear:
	rm -f *.o *.out
