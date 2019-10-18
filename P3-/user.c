#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h> //para en la llamada open utilizar las macros
#include <pthread.h> 
#include <string.h>

int status_exit=0;

#define DISPOSITIVO_1       "/dev/usb/blinkstick0"
int blinkstick0; // Descriptor del blinkstick0 DISPOSITIVO_1

#ifdef NEW_STICK
    #define DISPOSITIVO_2       "/dev/usb/blinkstick1"
    int blinkstick1;            // Descriptor del blinkstick0 DISPOSITIVO_2
#endif

const char* led1 = "0:0x27222F";
const char* led2 = "1:0x27222F";
const char* led3 = "2:0x27222F";
const char* led4 = "3:0x27222F";
const char* led5 = "4:0x27222F";
const char* led6 = "5:0x27222F";
const char* led7 = "6:0x27222F";
const char* led8 = "7:0x27222F";

struct region{
    int xR;
    int xL;
};

char fich[] = "/dev/usb/blinkstick0";
int blinkstick0;

void getScreenSize(int* sizeX, int* sizeY){
    int num;
    FILE *fp;
    char* buffer = malloc(100);  

    fp = popen("xrandr | grep '*' | cut -d 'x'  -f1 | cut --bytes=4-8","r"); //get sizeX
    if(fp){
        while (fgets(buffer, sizeof(buffer)-1, fp) != NULL) { }
        if(sscanf(buffer,"%d",&num)!=1)
            printf("ERROR\n");
        *sizeX = num;
        pclose(fp);
    }
    fp = popen("xrandr | grep '*' | cut -d x -f2 | cut -d ' ' -f1","r"); //get sizeY
    if(fp){
        while (fgets(buffer, sizeof(buffer)-1, fp) != NULL) { }
        if(sscanf(buffer,"%d",&num)!=1)
            printf("ERROR\n");
        *sizeY = num;
        pclose(fp);
    }
    free(buffer);
}

struct region* createRegions(){
    struct region* regions = malloc(sizeof(struct region)*16);
    int* sizeX = malloc(sizeof(int));
    int* sizeY = malloc(sizeof(int));
    int space;
    int xfirst,xsecond;

    xfirst = 0, xsecond = 0;
    getScreenSize(sizeX,sizeY);
    space = *sizeX / 16;
    int i =0;
    while(i < 16){
        struct region reg;
        xfirst = (i*space);
        xsecond += space;
        reg.xL = xfirst;
        reg.xR = xsecond;
        regions[i] = reg;
        i++;
    }
    free(sizeX);
    free(sizeY);
    return regions;
}

void getMouseCood(int* x/*, int* y*/){
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
    /*fp = popen("xdotool getmouselocation | cut -d y -f2 | cut -d : -f2 | cut -d s -f1","r"); //get Y
    if(fp){
        while (fgets(buffer, sizeof(buffer)-1, fp) != NULL) { }
        if(sscanf(buffer,"%d",&num)!=1)
            printf("ERROR\n");
        //printf("%d\n",num);
        //*y = num;
        pclose(fp);
    }*/
    free(buffer);
}

int getVolume(){

    int num;
    FILE *fp;
    char* buffer = malloc(80);
    char aux[80];

    fp = popen("pactl list sinks | grep '^[[:space:]]Volume:' | head -n $(( $SINK + 1 )) |  cut -d / -f2 | cut -d % -f1","r");
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

int inReg(int x, struct region r){
    if(x >= r.xL && x < r.xR){
        return 1;
    }
    return  0;
}

void mouseMovement(){
    int x;
    int use;
    struct region* regions = malloc(sizeof(struct region)*16);
    char *l = malloc(40);

    use = -1;
    regions = createRegions();
    while(!status_exit){
        usleep(100000);
        strcpy(l," ");
        getMouseCood(&x);
        printf("%d\n",x);

        if(inReg(x,regions[0])>0){
           if(use != 1){
            write(blinkstick1,"\n",1);
                strcat(l,led1); 
                strcat(l,"\n");
                write(blinkstick0,l,11);
                use=1;
            }
        }else if(inReg(x,regions[1])>0){
            if(use != 2){
                write(blinkstick1,"\n",1);
                strcat(l,led2);
                strcat(l,"\n");
                write(blinkstick0,l,11);
                use = 2;
            }
        }else if(inReg(x,regions[2])>0){
           if(use != 3){
            write(blinkstick1,"\n",1);
                strcat(l,led3);
                strcat(l,"\n");
                write(blinkstick0,l,11);
                use = 3;
            }
        }else if(inReg(x,regions[3])>0){
            if(use != 4){
                write(blinkstick1,"\n",1);
                strcat(l,led4);
                strcat(l,"\n");
                write(blinkstick0,l,11);
                use = 4;
            }
        }else if(inReg(x,regions[4])>0){
            if(use != 5){
                write(blinkstick1,"\n",1);
                strcat(l,led5);
                strcat(l,"\n");
                write(blinkstick0,l,11);
                use = 5;
            }
        }else if(inReg(x,regions[5])>0){
            if(use != 6){
                write(blinkstick1,"\n",1);
                strcat(l,led6);
                strcat(l,"\n");
                write(blinkstick0,l,11);
                use = 6;
            }
        }else if(inReg(x,regions[6])>0){
            if(use != 7){
                write(blinkstick1,"\n",1);
                strcat(l,led7);
                strcat(l,"\n");
                write(blinkstick0,l,11);
                use = 7;
            }
        }else if(inReg(x,regions[7])>0){
            if(use != 8){
                write(blinkstick1,"\n",1);
                strcat(l,led8);
                strcat(l,"\n");
                write(blinkstick0,l,11);
                use = 8;
            }
        }else if(inReg(x,regions[8])>0){
                if(use != 9){
                    write(blinkstick0,"\n",1);
                    strcat(l,led8);
                    strcat(l,"\n");
                    write(blinkstick1,l,11);
                    use = 9;
                }
            }else if(inReg(x,regions[9])>0){
                if(use != 10){
                    write(blinkstick0,"\n",1);
                    strcat(l,led7);
                    strcat(l,"\n");
                    write(blinkstick1,l,11);
                    use = 10;
                }

            }else if(inReg(x,regions[10])>0){
                if(use != 11){
                    write(blinkstick0,"\n",1);
                    strcat(l,led6);
                    strcat(l,"\n");
                    write(blinkstick1,l,11);
                    use = 11;
                }
            }else if(inReg(x,regions[11])>0){
                if(use != 12){
                    write(blinkstick0,"\n",1);
                    strcat(l,led5);
                    strcat(l,"\n");
                    write(blinkstick1,l,11);
                    use = 12;
                }
            }else if(inReg(x,regions[12])>0){
                if(use != 13){
                    write(blinkstick0,"\n",1);
                    strcat(l,led4);
                    strcat(l,"\n");
                    write(blinkstick1,l,11);
                    use = 13;
                }
            }else if(inReg(x,regions[13])>0){
                if(use != 14){
                    write(blinkstick0,"\n",1);
                    strcat(l,led3);
                    strcat(l,"\n");
                    write(blinkstick1,l,11);
                    use = 14;
                }
            }else if(inReg(x,regions[14])>0){
                if(use != 15){
                        write(blinkstick0,"\n",1);
                    strcat(l,led2);
                    strcat(l,"\n");
                    write(blinkstick1,l,11);
                    use = 15;
                }
            }else if(inReg(x,regions[15])>0){
                if(use != 16){
                    write(blinkstick0,"\n",1);
                    strcat(l,led1);
                    strcat(l,"\n");
                    write(blinkstick1,l,11);
                    use = 16;
                }
            }else{

                write(blinkstick0,"\n",1);
                write(blinkstick1,"\n",1);
            }
        
   }

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
                write(blinkstick0,"\n",1);
                use = 0;
            }
        }else if(n > 2 && n < 22){
            if(use != 1){
                strcat(l,led1); 
                strcat(l,"\n");
                write(blinkstick0,l,11);
                use=1;
            }
        }else if(n>=22 && n < 40){
            if(use != 2){
                strcat(l,led1);
                strcat(l,",");
                strcat(l,led2);
                strcat(l,"\n");
                write(blinkstick0,l,22);
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
                write(blinkstick0,l,33);
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
                write(blinkstick0,l,44);
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
                write(blinkstick0,l,55);
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
                write(blinkstick0,l,66);
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
                write(blinkstick0,l,77);
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
                write(blinkstick0,l,88);
                use = 8;
            }
        }else{
            write(blinkstick0,"\n",1);
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
    if ((blinkstick0 = open(DISPOSITIVO_1, O_WRONLY)) < 0)
        return -1;
    
    #ifdef NEW_STICK
        if ((blinkstick1 = open(DISPOSITIVO_2, O_WRONLY)) < 0)
            return -1;
    #endif

    //mouseMovement();
    volume();
    
    close(blinkstick0);

    return 0;
}