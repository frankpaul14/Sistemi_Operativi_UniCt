/*
    Homework n.1

    Scrivere un programma in linguaggio C che permetta di copiare un numero
    arbitrario di file regolari su una directory di destinazione preesistente.

    Il programma dovra' accettare una sintassi del tipo:
     $ homework-1 file1.txt path/file2.txt "nome con spazi.pdf" directory-destinazione
*/

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <libc.h>
#include <string.h>
#include <libgen.h>
#define SIZE 1024

int main(int argc, char* argv[]){ // durante la compilazione bisogna passare il i file da copiare e sa directory di destinazione es. frank.txt cartella/
    int fd,fd1;  
    int rd,wr;
    char buffer[SIZE];
    int size;
    char *name;
    int len;

    
    if(argc<3){    // verifico che gli argomenti passati siano almeno minori di 3, perchè altrimenti non è possibile eseguire andare avanti
        printf("USAGE: ./a.out file1.txt path/ \n");
        exit(1);
    }
   
    for ( int i = 1 ; i < argc-1 ; i++){ //eseguo un ciclo prendendo in considerazione tutti i file che devo copiare,int i=1 cosi si prende il argomento argv[1] per poi proseguire con argv[2] ... argv[n]
        if((fd=open(argv[i],O_RDONLY))==-1){  //apro il file argv[i] in modalità lettura 
            perror("open");
            exit(1);
        }

        //creo un buffer per salvare il percorso, dove andrà salvato il file
        // nel percorso devo inserire il nome del file che deve essere copiato 
        //prendendo in considerazione  ./a.out frank.txt cartella/ , il mio buffer deve essere cartella/frank.txt
        strcpy(buffer,argv[argc-1]); //copio nel buffer argv[argc-1] (in questo caso il percorso dove deve essere salvato il file)  buffer vale -> cartella/
        len = strlen(buffer); //salvo la dimensione del buffer poichè devo incrementarla per poter continuare a salvare il percorso 
        strcpy(buffer+len,"/");
        len = strlen(buffer); //salvo la dimensione del buffer poichè devo incrementarla per poter continuare a salvare il percorso 
        name=basename(argv[i]); //può capitare che il file stia dentro una directory (folder/frank.txt), per poter prendere frank.txt uso la funzione basename
        strcpy(buffer+len,name); // sommo al buffer la dimensione salvata precedentemente e concateno il nome del file da copiare
        //printf("buffer -> %s\n",buffer);

        if((fd1=open(buffer,O_RDWR|O_CREAT|O_TRUNC,0660))==-1){ //creo il nuovo file nella directory di destinazione
            perror("Usa / nel percorso dove salvare il file");
            exit(1);
        }

        do{ 
            if((rd=read(fd,buffer,SIZE))==-1){ // leggo il file da copiare e lo salvo in un buffer 
                perror("read");
                exit(1);
            }

            if((wr=write(fd1,buffer,rd))==-1){//prendo il buffer che era stato scritto precedentemente e copio il contenuto nel nuovo file nella directory di destinazione
                perror("write");
                exit(1);
            }
        
        }while(SIZE==rd);
        printf("\n");
        printf("Ho copiato il file -> %s \n",name);
    }
    printf("\n");
    printf("I file sono stati copiati con successo\n");
        

}