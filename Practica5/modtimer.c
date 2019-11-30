#include "headers.h"

#define MAX_CBUFFER_LEN 32

struct list_head mylist; // LISTA ENLAZADA
// NODOS DE LA LISTA
struct list_item {
	int data;
	struct list_head links;
};

struct kfifo cbuffer;/* Buffer circular*/

spinlock_t sp_timer;
struct semaphore sem_list;
struct semaphore queue;

struct proc_dir_entry* timer_proc; // Entrada /proc/modtimer
struct proc_dir_entry* config_proc; // Entrada /proc/modconfig

static int timer_release(struct inode *inode, struct file *file);
static int timer_open(struct inode *inode, struct file *file);
static ssize_t timer_read(struct file *file, char *buff, size_t len, loff_t *offset);

static struct file_operations timer_fops = {
        .owner = THIS_MODULE,
	.read 	= timer_read,
	.open 	= timer_open,
	.release = timer_release,
};

/* Funcionesde inicialización y descargadel módulo*/
int init_module(void){
	kfifo_alloc(&cbuffer,MAX_CBUFFER_LEN,GFP_KERNEL);
	INIT_LIST_HEAD(&mylist); // Inicializamos la lista enlazada

    sema_init(&sem_list,1);
	sema_init(&queue,0);
	spin_lock_init(&sp_timer);

    timer_period_ms = 500;
    max_random = 1000;
    emergency_threshold = 75;

	timer_proc = proc_create("modtimer", 0666, NULL, &timer_fops); // Creamos la entrada /proc/modtimer
	config_proc = proc_create("modconfig", 0666, NULL, &config_fops); // Creamos la entrada /proc/modconfig
	if(config_proc == NULL || timer_proc == NULL)
		goto failed;

	return 0;

	failed:
		kfifo_free(&cbuffer);
		return -EAGAIN; // Error: ty again
}

void cleanup_module(void){
	remove_proc_entry("modtimer",NULL);
	remove_proc_entry("modconfig",NULL);
	if(!list_empty(&mylist)){ // Ponemos if porque la funcion list_for_each_safe() recorre la lista asique no hace falta hacer mas iteraciones
		struct list_item* item = NULL;
		struct list_head* cur_node = NULL;
		struct list_head* aux = NULL;

		 //En aux almacenamos el siguiente nodo de la lista y cur_node es el nodo a borrar (???)
		list_for_each_safe(cur_node, aux, &mylist) {
			item = list_entry(cur_node, struct list_item, links);//&mylist);
			list_del(cur_node);
			vfree(item);
		};
	}
	kfifo_free(&cbuffer);
}

/* Se invocaal hacerclose() de entrada/proc*/
static int timer_release (struct inode *inode, struct file *file){
	//del_timer_sync();
	
	//esperar a que acabe trabajos workquue creada

	kfifo_reset(&cbuffer);
	//vaciar lista
	module_put(THIS_MODULE);
}

/* Se invocaal haceropen() de entrada/proc*/
static int timer_open(struct inode *inode, struct file *file){
	try_module_get(THIS_MODULE);
}

/* Se invocaal hacer read() de entrada/proc*/
static ssize_t timer_read(struct file *file, char *buff, size_t len, loff_t *offset){

}

/* Se invocaal hacer write() de entrada/proc*/
static ssize_t timer_write(struct file *file, const char *buff, size_t len, loff_t *offset){
	
}
