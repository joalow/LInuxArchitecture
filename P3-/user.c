#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h> //para en la llamada open utilizar las macros
#include <pthread.h> 
#include <string.h>

int status_exit=0;

const char* led1 = "0:0xB7F24F";
#define led2 "1:0xB7F24F"
#define led3 "2:0xB7F24F"
#define led4 "3:0xB7F24F"
#define led5 "4:0xB7F24F"
#define led6 "5:0xB7F24F"
#define led7 "6:0xB7F24F"
#define led8 "7:0xB7F24F"

char fich[] = "/dev/usb/blinkdrv";
int fichero;

int getVolume(){

    int num;
    FILE *fp;
    char* buffer = malloc(80);
    char aux[80];

    fp = popen("pactl list sinks | grep '^[[:space:]]Volume:' | \head -n $(( $SINK + 1 )) |  cut -d / -f2 | cut -d % -f1","r");
    if(fp){
        while (fgets(buffer, sizeof(buffer)-1, fp) != NULL) { }
        while(*buffer == ' ') {buffer++;}
        strncpy(aux,buffer,strlen(buffer));
        strcpy(aux,buffer);
        if(sscanf(aux,"%d",&num)!=1)
            printf("ERROR\n");
        printf("%d\n",num);
        pclose(fp);
    }

     return num;
}

void volume(){
    int n;
	int use=-1;
	int maxUse=0;

    while(!status_exit){
        n = getVolume();
        printf("%d\n",n);

        if(n<2){
            write(fichero,"\n",1);
        }else if(n > 2 && n < 22){
            printf("yessss");
            char *l = malloc(100);
            strcpy(l,"");
            strcat(l,led1); 
            strcat(l,"\n");
            write(fichero,l,11);
        }else if(n>=22 && n < 40){
        	write(fichero,strcat(led1,strcat(",",strcat(led2,"\n"))),3);
        }else if(n>=40 && n<55){
        	if(use != 3){
        		write(fichero,"321\n",4);
        		use = 3;
        	}
        }else if(n>=55 && n<70){
            if(use != 3){
                write(fichero,"321\n",4);
                use = 3;
            }
        }else if(n>=70 && n<85){
            if(use != 3){
                write(fichero,"321\n",4);
                use = 3;
            }
        }else if(n>=85 && n<100){
            if(use != 3){
                write(fichero,"321\n",4);
                use = 3;
            }
        }else if(n>=100 && n<115){
            if(use != 3){
                write(fichero,"321\n",4);
                use = 3;
            }
        }else if(n>=115){
            if(use != 3){
                write(fichero,"321\n",4);
                use = 3;
            }
        }
        
        usleep(50000);
        //printf("%d\n",n);
    }
}

void stop(){
    printf("Press 'Enter' to exit the program: ... ");
    while ( getchar() != '\n');
}

int main(){
	fichero = open(fich,O_WRONLY | O_CREAT | O_TRUNC);

    volume();
    
	close(fichero);

	return 0;
}