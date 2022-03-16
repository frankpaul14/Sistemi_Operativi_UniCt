#include<stdlib.h>
#include<stdio.h>
#include<sys/wait.h>
#include<sys/sem.h>
#include<sys/shm.h>
#include<sys/ipc.h>
#include<unistd.h>
#include<string.h>
#include<time.h>
#include<stddef.h>
#include<sys/msg.h>
#include<sys/types.h>
#include <ctype.h>

#define MAX_LEN 1024

int WAIT(int sem_des, int num_semaforo) {
    struct sembuf operazioni[1] = {{num_semaforo,-1,0}};
    return semop(sem_des, operazioni, 1);
}

int SIGNAL(int sem_des, int num_semaforo) {
    struct sembuf operazioni[1] = {{num_semaforo,+1,0}};
    return semop(sem_des, operazioni, 1);
}

int applyFilter(char* p, char* filter){
    if ( filter[0]== '^'){
        char*word= filter+1;   //prende tutta la parola di filter togliendo il simbolo
     
        int lenword= strlen(word);

        if(lenword==0){
            return -1;
        }

        char* temp= p;
        char* wnow;

        while((wnow=strstr(temp,word))!=NULL){
            for ( int i = 0 ; i<lenword; i++){
                wnow[i]=toupper(wnow[i]);
            }
            temp=wnow+lenword;
        }
        return 0;
    }
    

    if(filter[0]=='_'){
        char*word= filter+1;
        int lenword=strlen(word);

        char*tmp= p; // mettiamo in tmp tutto il testo del file 
        char* wnow; // mettiamo la parola che dobbiamo cambiare in questa variabile

        while((wnow=strstr(tmp,word))!=NULL){
            for(int i= 0 ; i <lenword;i++){
                wnow[i]=tolower(wnow[i]);
            }
            tmp=wnow+lenword;
        }
        return 0;
    }

    if(filter[0]=='%'){
        char*str= filter+1;
        int strleng=strlen(str);
        char* parola1;
         

        for(int i = 0 ; i<strleng;i++){
            
            if(str[i]!='|'){
                parola1[i]= str[i];
            }
            else{
                i=strleng;
            }
        }

        int lenp1=strlen(parola1);
        char* parola2= str+lenp1+1;
        int lenp2=strlen(parola2);
        char*tmp=p;
        char*wnow;
        while((wnow=strstr(tmp,parola1))!=NULL){
            for ( int i=0 ;i<strleng;i++){
                
                if(i<lenp2){
                    wnow[i]=parola2[i];
                }
                else{
                    wnow[i]=wnow[i];
                }
            }
            tmp=wnow+lenp2;
        }
        
        //printf(" parola : %s , parola : %s \n",parola1,parola2);
        return 0;

    }




    return -1;
}





int filter( int i , char* filter , char* p, int sem){
    //devo attendere il permesso per poter leggere dal fiel
    int process=1 ;
    while(process){
    
    WAIT(sem,i);
    if( p[0]==0 && p[1]=='E'){
        process=0 ;    
    }
    else{
        int filterResult = applyFilter(p,filter);
        if(filterResult!=0){
            printf("Errore nella applicazione filtro\n");
            exit(1);
        }
    }
        SIGNAL(sem,i+1);
    }
    //dico al prossimo di entrare in memeoria 
    
    return 0 ;
}

int readingFile(char* file, char *p, int pos, int sem){
  
  FILE* stream;
  if((stream=fopen(file,"r"))==NULL){
      perror("open");
      exit(1);
  }

  while(fgets(p,MAX_LEN,stream)!=NULL){
      //adesso che ho letto il file e l'ho inserito nella memoria condivisa devo poter avvisare i filtri 

      SIGNAL(sem,0);

      // attendo un segnale dai fitlri 

      WAIT(sem,pos);

      printf("%s \n",p);
  }

  //una volta che ho finito dico che sono stati finiti

    p[0]= 0;
    p[1]='E';

    SIGNAL(sem,0);

    WAIT(sem,pos);
    if(fclose(stream)!=0){
        perror("close");
        exit(1);
    }

   return 1;
}


int main(int argc, char* argv[]){

    if(argc <3)
    {
        printf("Usage: ./a.out file.txt filter-1 filter-n \n  ");
        exit(1);
    }

    int  sem, shm;
    char* p;
    // memoria 
    if((shm = shmget(IPC_PRIVATE,sizeof(char)*MAX_LEN,IPC_CREAT|0600))==-1){
        perror("shmget");
        exit(1);
    }
    //semafori 
    if((sem=semget(IPC_PRIVATE,argc-1,IPC_CREAT|0600))==-1){
        perror("semget");
        exit(1);
    }
    //annettere mermoria 
    if((p = (char*)shmat(shm,NULL,0))==(char*)-1){
        perror("shmat");
        exit(1);
    }

    pid_t pid; 

    for ( int i = 0 ; i< argc-2; i++){
        pid=fork();

        if(pid==0){
            return filter(i,argv[2+i],p,sem);
        }
    }

    int rent = readingFile(argv[1], p , argc-2, sem);





//distruttori 
shmctl(shm,IPC_RMID,NULL);
semctl(sem,0,IPC_RMID,0);
//return rent;


}