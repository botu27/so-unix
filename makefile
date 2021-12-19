#MAKE FILE FACILITA NOTEVOLMENTE LA COMPILAZIONE DEL CODICE, SOPRATTUTTO SE E' SUDDIVISO IN MODULI
#Struttura generale:
#REGOLA: parametro1 parametro2 parametroN
#<TAB> comando da eseguire

#FLAG DEFINISCE STANDARD E MODALITA' PEDANTIC
STD=-c89 -pedantic
master:	Master17Dic.c
	gcc Master17Dic.c -o Master.out