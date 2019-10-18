#include <pwd.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <linux/input.h>

#define ENCENDER_TODO		"0:101010,1:101010,2:101010,3:101010,4:101010,5:101010,6:101010,7:101010,8:101010"
#define APAGAR_TODO			"0:000000,1:000000,2:000000,3:000000,4:000000,5:000000,6:000000,7:000000,8:000000"

#define DISPOSITIVO_1		"/dev/usb/blinkstick0"
int blinkstick0;			// Descriptor del fichero DISPOSITIVO_1

#define ARCOIRIS(index) for(index = 0; index < 20; ++index)

#ifdef NEW_STICK
	#define DISPOSITIVO_2		"/dev/usb/blinkstick1"
	int blinkstick1;			// Descriptor del fichero DISPOSITIVO_2
#endif

struct region{
    int xR;
    int xL;
};

const char* led1 = "0:0x101010";
const char* led2 = "1:0x101010";
const char* led3 = "2:0x101010";
const char* led4 = "3:0x101010";
const char* led5 = "4:0x101010";
const char* led6 = "5:0x101010";
const char* led7 = "6:0x101010";
const char* led8 = "7:0x101010";
int status_exit=0;

void getScreenSize(int* sizeX, int* sizeY);
struct region* createRegions();
void getMouseCood(int* x);
int inReg(int x, struct region r);
void mouseMovement();
int mostrarMenu();
void help();
int detectarMovimientoRaton();
int arcoiris();
int escribirArcoiris();
int apagarLeds();
int moverHaciaLaDerecha();
int moverHaciaLaIzquierda();
int getVolume();
void volume();
void stop();
int Experimental();
int encenderLed();

int main(){
	int eleccionMenu;

	if ((blinkstick0 = open(DISPOSITIVO_1, O_WRONLY)) < 0)
		return -1;
	#ifdef NEW_STICK
		if ((blinkstick1 = open(DISPOSITIVO_2, O_WRONLY)) < 0)
			return -1;
	#endif
	
	eleccionMenu = mostrarMenu();

	while(eleccionMenu != 0){
		switch(eleccionMenu){
			case 1: help();
					break;
			case 2: arcoiris();
					break;
			case 3: detectarMovimientoRaton();
					break;
			case 4: moverHaciaLaDerecha();
					break;
			case 5: moverHaciaLaIzquierda();
					break;
			case 6: volume();
					break;
			case 7: mouseMovement();
			break;
		}
		if(eleccionMenu != 0) eleccionMenu = mostrarMenu();
	}

	close(blinkstick0);

	#ifdef NEW_STICK
		close(blinkstick1);
	#endif

	return 0;
} 

int mostrarMenu() {
	char * lectura = malloc(10*sizeof(char));
	int opcionEscogida = 1;
	int numCorrectos = 1;
	struct passwd *userInfo = getpwuid(getuid());

	if(userInfo != NULL)
		printf("\n\tBienvenido %s!\n", userInfo->pw_name);


	printf("--------------------------------------------------\n");

	printf("1. Mostrar informacion\n");
	printf("2. Arcoiris\n");
	printf("3. Movimiento raton\n");
	printf("4. Mover hacia la derecha   ----->\n");
	printf("5. Mover hacia la izquierda <-----\n");
	printf("6. Volumen\n\n");
	printf("7. Direccion raton\n");
	printf("0. Salir\n\n");
	printf("\tSeleccionar una opcion: ");

	do{
		if(numCorrectos != 1 || opcionEscogida < 0 || opcionEscogida > 6)
			printf("Opcion escogida erronea.\nIntroduce otra a continuacion: ");
		fgets(lectura,10, stdin);
		numCorrectos = sscanf(lectura,"%d",&opcionEscogida);
	}while(numCorrectos != 1 || opcionEscogida < 0 || opcionEscogida > 6);

	return opcionEscogida;
}

void help() {

	char op1[] = "1. Muestra este mensaje";
	char op2[] = "2. Muestra los colores del arcoiris en cascada";
	char op3[] = "3. Enciende los leds mientras detecte movimiento de raton";
	char op4[] = "4. Movimiento de los leds hacia la derecha -------->";
	char op5[] = "5. Movimiento de los leds hacia la izquierda <------";
	char op6[] = "6. Enciende los leds segun ajustemos el volumen del sistema";

	printf("\n\n%s\n%s\n%s\n%s\n%s\n%s\n\n", op1, op2, op3, op4, op5, op6);
}

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

int detectarMovimientoRaton(){
	int raton;
	int MOUSE_WAIT;

	//Abrimos el descriptor del raton en modo READONLY, para que al hacer read se bloquee la ejecucion del programa
	//hasta que se mueva el raton.
	if ((MOUSE_WAIT = open("/dev/input/mice", O_RDONLY)) < 0)
		return -1;

	//Aqui abrimos en modo READONLY Y NONBLOCK, para que si o si lea, aunque el raton no se mueva
	if ((raton = open("/dev/input/mice", O_RDONLY | O_NONBLOCK)) < 0)
		return -1;

	printf("En esta funcion, los led se encenderan mientras el raton este en movimiento.\n");
	printf("Para salir, pulse el BOTON IZQUIERDO del raton.\n");
	printf("***Es posible que se requieran 2 clicks, en algunos casos.\n\n");

	//Struct que contiene los datos del raton (botones, x, y)
	struct input_event * ie = malloc(sizeof(struct input_event));
	
	unsigned char botonGeneral, botonIzq;

	char x = '\0',y = '\0';

	//Guardamos posiciones antiguas de x e y, para poder comparar si cambian o no
	char xOld = '\n',yOld = '\n';

	//Esperamos a que el raton se mueva para continuar con el programa;
	//Mientras tanto, estara bloqueado.
	read(MOUSE_WAIT,NULL,0);
	close(MOUSE_WAIT);

	int pulsado = 0;

	while(read(raton, ie, sizeof(struct input_event))){
		
		unsigned char * ptr = (unsigned char*) ie;

		botonGeneral = ptr[0];

		botonIzq = botonGeneral & 0x1;

		x = (char) ptr[1];
		y = (char) ptr[2];

		//Esta parte de aqui es debida a un bug que hay al hacer read del raton. A veces se inicia el boton izquierdo
		//como pulsado, y hasta que no se pulsa, no se pone a 0.
		if(botonIzq == 0)
			pulsado = 1;
		if(pulsado == 1 && botonIzq == 1)
			break;

		usleep(50000);

		if(x == xOld && y == yOld){
			//Si el raton se detiene, apagamos las luces y lo dejamos en espera
			//hasta que se vuelva a mover.
			if(write(blinkstick0, "\n", 1) < 0)
				return -1;

			#ifdef NEW_STICK
				if(write(blinkstick1, "\n", 1) < 0)
					return -1;
			#endif
			
			if ((MOUSE_WAIT = open("/dev/input/mice", O_RDONLY)) < 0)
				return -1;

			read(MOUSE_WAIT,NULL,0);
			close(MOUSE_WAIT);
			
		}

		if(write(blinkstick0, ENCENDER_TODO, 80) < 0)
			return -1;

		#ifdef NEW_STICK
			if(write(blinkstick1, ENCENDER_TODO, 80) < 0)
				return -1;
		#endif

		xOld = x;
		yOld = y;

	}

	apagarLeds();

	free(ie);
	close(raton);
	
	return 0;
}

int moverHaciaLaDerecha(){
	int vuelta, led;
	
	for (vuelta = 0; vuelta < 10; ++vuelta){
		for (led = 0; led < 8; ++led){
			char* tmp = malloc(10*sizeof(char));
			*tmp = led + '0';
			strcat(tmp, ":0x110000");
			if(write(blinkstick0, tmp, 10) < 0)
				return -1;
			#ifdef NEW_STICK
				if(write(blinkstick1, tmp, 10) < 0)
					return -1;
			#endif
			usleep(100000);
		}
	}

	apagarLeds();

	return 0;
}

int moverHaciaLaIzquierda(){
	int vuelta, led;
	
	for (vuelta = 0; vuelta < 10; ++vuelta){
		for (led = 7; led >= 0; --led){
			char* tmp = malloc(10*sizeof(char));
			*tmp = led + '0';
			strcat(tmp, ":0x001100");
			if(write(blinkstick0, tmp, 10) < 0)
				return -1;
			#ifdef NEW_STICK
				if(write(blinkstick1, tmp, 10) < 0)
					return -1;
			#endif
			usleep(100000);
		}
	}

	apagarLeds();

	return 0;
}

int arcoiris(){
	char decision;
	int indice = 0, ret;

	printf("\n\t¡¡¡ATENCION!!! Esta opcion podria deslumbrar por contener colores de fuerte intensidad.\n");
	printf("\t¿Continuar? [y/n]: ");
	scanf("%c", &decision);

	if(decision == 'y'){
		ARCOIRIS(indice){
			ret = escribirArcoiris();
		}

		apagarLeds();
	}

	return (ret < 0) ? -1 : 0;
}
int escribirArcoiris(){	
	if(write(blinkstick0, "0:0x110000", 10) < 0) return -1;
	if(write(blinkstick0, "1:0xC38748", 10) < 0) return -1;
	if(write(blinkstick0, "2:0xC3B848", 10) < 0) return -1;
	if(write(blinkstick0, "3:0x001100", 10) < 0) return -1;
	if(write(blinkstick0, "4:0x1C9B7E", 10) < 0) return -1;
	if(write(blinkstick0, "5:0x000011", 10) < 0) return -1;
	if(write(blinkstick0, "6:0x291B66", 10) < 0) return -1;
	if(write(blinkstick0, "7:0x5F2471", 10) < 0) return -1;

	#ifdef NEW_STICK
		if(write(blinkstick1, "0:0x110000", 10) < 0) return -1;
		if(write(blinkstick1, "1:0xC38748", 10) < 0) return -1;
		if(write(blinkstick1, "2:0xC3B848", 10) < 0) return -1;
		if(write(blinkstick1, "3:0x001100", 10) < 0) return -1;
		if(write(blinkstick1, "4:0x1C9B7E", 10) < 0) return -1;
		if(write(blinkstick1, "5:0x000011", 10) < 0) return -1;
		if(write(blinkstick1, "6:0x291B66", 10) < 0) return -1;
		if(write(blinkstick1, "7:0x5F2471", 10) < 0) return -1;
	#endif

	return 0;
}

int apagarLeds(){
	if(write(blinkstick0, APAGAR_TODO, 80) < 0)
		return -1;

	#ifdef NEW_STICK
		if(write(blinkstick1, APAGAR_TODO, 80) < 0)
			return -1;
	#endif

	return 0;
}

/* FUNCIONES PARA CONTROLAR LOS LEDS SEGUN AJUSTEMOS EL VOLUMEN DEL SISTEMA */
void stop(){
    printf("Press 'Enter' to exit the program: ... ");
    while ( getchar() != '\n');
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
        //printf("%d\n",num);
        pclose(fp);
    }

     return num;
}

void volume(){
    int n;
    int use=-1;
    //int maxUse=0;
    char *l = malloc(300);

    while(!status_exit){
        strcpy(l," ");
        
        n = getVolume();

        if(n<2){
            if(use != 0){
                write(blinkstick0,"\n",1);
                #ifdef NEW_STICK
                write(blinkstick1,"\n",1);
                #endif
                use = 0;
            }
        }else if(n > 2 && n < 22){
            if(use != 1){
                strcat(l,led1); 
                strcat(l,"\n");
                write(blinkstick0,l,11);
                #ifdef NEW_STICK
                write(blinkstick1,l,11);
                #endif
                use=1;
            }
        }else if(n>=22 && n < 40){
            if(use != 2){
                strcat(l,led1);
                strcat(l,",");
                strcat(l,led2);
                strcat(l,"\n");
                write(blinkstick0,l,22);
                #ifdef NEW_STICK
                write(blinkstick1,l,22);
                #endif
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
                #ifdef NEW_STICK
                write(blinkstick1,l,33);
                #endif
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
                #ifdef NEW_STICK
                write(blinkstick1,l,44);
                #endif
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
                #ifdef NEW_STICK
                write(blinkstick1,l,55);
                #endif
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
                #ifdef NEW_STICK
                write(blinkstick1,l,66);
                #endif
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
                #ifdef NEW_STICK
                write(blinkstick1,l,77);
                #endif
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
                #ifdef NEW_STICK
                write(blinkstick1,l,88);
                #endif
                use = 8;
            }
        }else{
            write(blinkstick0,"\n",1);
            #ifdef NEW_STICK
            write(blinkstick1,"\n",1);
            #endif
        }
        
        usleep(100000);
    }
    free(l);
}