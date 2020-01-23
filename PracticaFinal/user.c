#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

void show(void){
	printf("	 __________________________________________________________________________________________________\n");
	printf("	|                                                                                                  |\n");
	printf("	|                                                                                                  |\n");
	printf("	|                                                        ___                                       |\n");
	printf("	|             ____________________                      /  /                                       |\n");
	printf("	|            /____________________\\                    /  /  X   _____  _______  ______            |\n");
	printf("	|                      | |   ____  _________          /  /   _  | ____||__  ___|| ____|            |\n");
	printf("	|                      | |  |    | |    _   |        /  /   | | | |____   | |   | |____            |\n");
	printf("	|                      | |  |____| |   |_|  |       /  /    | | |___  |   | |   |___  |            |\n");
	printf("	|                      | |         | ______|       /  /     | |  ___| |   | |    ___| |            |\n");
	printf("	|                      | |         |  |           /  /      |_| |_____|   |_|   |_____|            |\n");
	printf("	|                      |_|         |  |          /  /________                                      |\n");
	printf("	|                                  |__|         /____________|                                     |\n");
	printf("	|                                                                                                  |\n");
	printf("	|                                                                                                  |\n");
	printf("	|                                                                                                  |\n");
	printf("	|                                                                                                  |\n");
	printf("	|                                   1: Show lists                                                  |\n");
	printf("	|                                                                                                  |\n");
	printf("	|                                   2: Show list < >                                               |\n");
	printf("	|                                                                                                  |\n");
	printf("	|                                   3: Add  list < >                                               |\n");
    printf("	|                                                                                                  |\n");
	printf("	|                                   4: Add  element< > to list< >                                  |\n");
	printf("	|                                                                                                  |\n");
	printf("	|                                   5: Add  elements< | |..> to list< >                            |\n");
	printf("	|                                                                                                  |\n");
	printf("	|                                   6: Remove list < >                                             |\n");
	printf("	|                                                                                                  |\n");
	printf("	|                                   7: Remove element number < > of list < >                       |\n");
	printf("	|                                                                                                  |\n");
	printf("	|                                   8: Exit                                                        |\n");
	printf("	|                                                                                                  |\n");
	printf("	|                                                                                                  |\n");
	printf("	|                                                                                                  |\n");
	printf("	|__________________________________________________________________________________________________|\n\n\n");
}

void action(int num){
	int fd;

	switch(num){
		case 1:
		
		break;

		case 2:

		break;

		case 3:

		break;

		case 4:

		break;

		case 5:

		break;

		case 6:

		break;

		case 7:

		break;
	}
}

int main(void){
	int num;
	/*int fd_ctrl, k;
	char *buff;

	fd_ctrl = open("/proc/list/default", O_RDWR);
	buff = malloc(300);
	k = read(fd_ctrl,buff,100);

	close(fd_ctrl);*/
	num = -1;	
	do{
		show();
		if(scanf("Nº Operation:  %d",num)){
			if(num < 0 || num > 7)
				printf("ERROR: number selected is not a operation, try again. \n");
			else if(num==0){
				action(num);
			}
		}
	}while(num != 0);

	return 0;
}