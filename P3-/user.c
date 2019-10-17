#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h> //para en la llamada open utilizar las macros
#include <pthread.h> 
#include <string.h>

int status_exit=0;

const char* led1 = "0:0xB7F24F";
const char* led2 = "1:0xB7F24F";
const char* led3 = "2:0xB7F24F";
const char* led4 = "3:0xB7F24F";
const char* led5 = "4:0xB7F24F";
const char* led6 = "5:0xB7F24F";
const char* led7 = "6:0xB7F24F";
const char* led8 = "7:0xB7F24F";

char fich[] = "/dev/usb/blinkstick0";
int fichero;

void getScreenSize(int* sizeX, int* sizeY){
    int num;
    FILE *fp;
    char* buffer = malloc(80);  

    strcat(buffer," ");
    fp = popen("xrandr | grep '*' | cut -d x -f1","r"); //get sizeX
    if(fp){
        while (fgets(buffer, sizeof(buffer)-1, fp) != NULL) { }
        printf("%s\n",buffer );
        if(sscanf(buffer,"%d",&num)!=1)
            printf("ERROR\n");
        printf("%d\n",num);
        //*x = num;
        pclose(fp);
    }
    fp = popen("xrandr | grep '*' | cut -d x -f2 | cut -d ' ' -f1","r"); //get sizeY
    if(fp){
        while (fgets(buffer, sizeof(buffer)-1, fp) != NULL) { }
        printf("%s\n",buffer );
        if(sscanf(buffer,"%d",&num)!=1)
            printf("ERROR\n");
        printf("%d\n",num);
        //*x = num;
        pclose(fp);
    }
}

void getMouseCood(int* x, int* y){
    int num;
    FILE *fp;
    char* buffer = malloc(80);  

    fp = popen("xdotool getmouselocation | cut -d : -f2 | cut -d y -f1","r"); //get X
    if(fp){
        while (fgets(buffer, sizeof(buffer)-1, fp) != NULL) { }
        if(sscanf(buffer,"%d",&num)!=1)
            printf("ERROR\n");
        //printf("%d\n",num);
        *x = num;
        pclose(fp);
    }
    fp = popen("xdotool getmouselocation | cut -d y -f2 | cut -d : -f2 | cut -d s -f1","r"); //get Y
    if(fp){
        while (fgets(buffer, sizeof(buffer)-1, fp) != NULL) { }
        if(sscanf(buffer,"%d",&num)!=1)
            printf("ERROR\n");
        //printf("%d\n",num);
        *y = num;
        pclose(fp);
    }
}

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
    char *l = malloc(300);

    while(!status_exit){
        strcpy(l," ");
        
        n = getVolume();

        if(n<2){
            if(use != 0){
                write(fichero,"\n",1);
                use = 0;
            }
        }else if(n > 2 && n < 22){
            if(use != 1){
                strcat(l,led1); 
                strcat(l,"\n");
                write(fichero,l,11);
                use=1;
            }
        }else if(n>=22 && n < 40){
            if(use != 2){
                strcat(l,led1);
                strcat(l,",");
                strcat(l,led2);
                strcat(l,"\n");
                write(fichero,l,22);
                use = 2;
            }
        }else if(n>=40 && n<55){
            if(use != 3){
                strcat(l,led1);
                strcat(l,",");
                strcat(l,led2);
                strcat(l,",");
                strcat(l,led3);
                strcat(l,"\n");
                write(fichero,l,33);
                use = 3;
            }
        }else if(n>=55 && n<70){
            if(use != 4){
                strcat(l,led1);
                strcat(l,",");
                strcat(l,led2);
                strcat(l,",");
                strcat(l,led3);
                strcat(l,",");
                strcat(l,led4);
                strcat(l,"\n");
                write(fichero,l,44);
                use = 4;
            }
        }else if(n>=70 && n<85){
            if(use != 5){
                strcat(l,led1);
                strcat(l,",");
                strcat(l,led2);
                strcat(l,",");
                strcat(l,led3);
                strcat(l,",");
                strcat(l,led4);
                strcat(l,",");
                strcat(l,led5);
                strcat(l,"\n");
                write(fichero,l,55);
                use = 5;
            }
        }else if(n>=85 && n<100){
            if(use != 6){
                strcat(l,led1);
                strcat(l,",");
                strcat(l,led2);
                strcat(l,",");
                strcat(l,led3);
                strcat(l,",");
                strcat(l,led4);
                strcat(l,",");
                strcat(l,led5);
                strcat(l,",");
                strcat(l,led6);
                strcat(l,"\n");
                write(fichero,l,66);
                use = 6;
            }
        }else if(n>=100 && n<115){
            if(use != 7){
                strcat(l,led1);
                strcat(l,",");
                strcat(l,led2);
                strcat(l,",");
                strcat(l,led3);
                strcat(l,",");
                strcat(l,led4);
                strcat(l,",");
                strcat(l,led5);
                strcat(l,",");
                strcat(l,led6);
                strcat(l,",");
                strcat(l,led7);
                strcat(l,"\n");
                write(fichero,l,77);
                use = 7;
            }
        }else if(n>=115){
            if(use != 8){
                strcat(l,led1);
                strcat(l,",");
                strcat(l,led2);
                strcat(l,",");
                strcat(l,led3);
                strcat(l,",");
                strcat(l,led4);
                strcat(l,",");
                strcat(l,led5);
                strcat(l,",");
                strcat(l,led6);
                strcat(l,",");
                strcat(l,led7);
                strcat(l,",");
                strcat(l,led8);
                strcat(l,"\n");
                write(fichero,l,88);
                use = 8;
            }
        }else{
            write(fichero,"\n",1);
        }
        
        usleep(100000);
    }
    free(l);
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