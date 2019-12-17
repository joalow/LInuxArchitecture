#include "headers.h"

#define MAX_CBUFFER_LEN 32

struct list_head mylist; // LISTA ENLAZADA

// NODOS DE LA LISTA
struct list_item {
	int data;
	struct list_head links;
};

struct kfifo cbuffer;		/* Buffer circular*/
struct timer_list my_timer; // Estructura para implementar el temporizador
struct work_struct *work;
struct workqueue_struct* wq;

spinlock_t sp_timer;
struct semaphore sem_list;
struct semaphore queue;

struct proc_dir_entry* timer_proc; // Entrada /proc/modtimer
struct proc_dir_entry* config_proc; // Entrada /proc/modconfig

static unsigned long flags;

static int timer_release(struct inode *inode, struct file *file);
static int timer_open(struct inode *inode, struct file *file);
static ssize_t timer_read(struct file *file, char *buff, size_t len, loff_t *offset);
void timer_function(struct timer_list *);
static void my_work_func(struct work_struct *work);

static struct file_operations timer_fops = {
    .owner = THIS_MODULE,
	.read 	= timer_read,
	.open 	= timer_open,
	.release = timer_release,
};

static void my_work_func(struct work_struct *work)
{
	//copiar del buffer a array, y de array a lspinlockist
	int* array = vmalloc(kfifo_len(&cbuffer));
	struct list_item* item = NULL;
	int i;


	spin_lock_irqsave(&sp_timer,flags);

	kfifo_out(&cbuffer,array,kfifo_len(&cbuffer));
	printk(KERN_INFO "volcado");

	spin_unlock_irqrestore(&sp_timer,flags);
	
	// Procedemos a pasarlo a la lista enlazada
	for(i = 0; i < kfifo_len(&cbuffer); i++){
		item = (struct list_item*) vmalloc(sizeof(struct list_item));
		item->data = array[i];
		if(down_interruptible(&sem_list))
			return -EINTR;
		printk(KERN_INFO "add %d", item->data);
		list_add_tail(&item->links,&mylist);
		up(&sem_list);
		up(&queue); //cola de espera de lista enlazada vacia
	}
}

// Generar numero aleatorio y meterlo al buffer circular 
void my_timer_function(unsigned long data){
	int n = get_random_int() % max_random;

	spin_lock_irqsave(&sp_timer,flags);

	printk(KERN_INFO "numero del timer: %d", n);
	kfifo_in(&cbuffer, n, sizeof(n));

	spin_unlock_irqrestore(&sp_timer,flags);

	if(kfifo_len(&cbuffer)/kfifo_size(&cbuffer) >= emergency_threshold){ //% de ocupacion alcanzado
		queue_work(wq,work);
		flush_work(work);
	}
}

// Funcion que usamos para incializar todo lo relativo al timer
static void init_my_timer(void){
	init_timer(&my_timer);
	printk(KERN_INFO "iniciado timer");
	my_timer.expires = jiffies + timer_period_ms; // EXPIRES ==> Tiempo de activacion del timer ======== timer_period_ms ????
	my_timer.data = 0;
	my_timer.function = my_timer_function;

	printk(KERN_INFO "iniciado timer aun");
	// AQUI ACTIVAMOS EL TIMER POR PRIMERA VEZ, EMPEZAMOS LA CUENTA
	add_timer(&my_timer);
			printk(KERN_INFO "iniciado timer es");
}

/* Funciones de inicialización y descarga del módulo*/
int init_modul(void){
	// CREAMOS Y DEFINIMOS EL BUFFER CIRCULAR
	printk(KERN_INFO "iniciando");
	kfifo_alloc(&cbuffer,MAX_CBUFFER_LEN,GFP_KERNEL);

	// INICIALIZAMOS LA LISTA ENLAZADA
	INIT_LIST_HEAD(&mylist);
	printk(KERN_INFO "iniciado aun");
    sema_init(&sem_list,1);
	sema_init(&queue,0);
	spin_lock_init(&sp_timer);

	// CREAMOS LA WORKQUEUE
	INIT_WORK(work,my_work_func);
	wq = create_workqueue("bottom_half");

	// INICIALIZAMOS LAS VARIABLES
    timer_period_ms = 500;
    max_random = 300;
    emergency_threshold = 75;

	timer_proc = proc_create("modtimer", 0666, NULL, &timer_fops); // Creamos la entrada /proc/modtimer
	config_proc = proc_create("modconfig", 0666, NULL, &config_fops); // Creamos la entrada /proc/modconfig
	if(config_proc == NULL || timer_proc == NULL)
		goto failed;

	// CREAMOS E INICIALIZAMOS EL TIMER
	init_my_timer();
	printk(KERN_INFO "iniciado");
	return 0;

	failed:
		printk(KERN_INFO "no iniciado");
		kfifo_free(&cbuffer);
		return -EAGAIN; // Error: ty again
}

void cleanup_modul(void){
	struct list_item* item = NULL;
	struct list_head* cur_node = NULL;
	struct list_head* aux = NULL;
	
	del_timer_sync(&my_timer);

	// Esperar a que acaben los trabajos de la workqueue creada
	flush_scheduled_work();

	// ELIMINAMOS LAS ENTRADAS /proc
	remove_proc_entry("modtimer",NULL);
	remove_proc_entry("modconfig",NULL);

	if(!list_empty(&mylist)){ // Ponemos if porque la funcion list_for_each_safe() recorre la lista asique no hace falta hacer mas iteraciones

		//En aux almacenamos el siguiente nodo de la lista y cur_node es el nodo a borrar (???)
		list_for_each_safe(cur_node, aux, &mylist) {
			item = list_entry(cur_node, struct list_item, links);//&mylist);
			list_del(cur_node);
			vfree(item);
		};
	}
	kfifo_free(&cbuffer);
}

/* Se invoca al hacer close() de entrada /proc */
static int timer_release (struct inode *inode, struct file *file){
	// DESACTIVAMOS EL TEMPORIZADOR
	struct list_item* item = NULL;
	struct list_head* cur_node = NULL;
	struct list_head* aux = NULL;
	del_timer_sync(&my_timer);
	
	// ESPERAR A QUE ACABEN TODOS LOS TRABAJOS DE LA WORKQUEUE
	flush_scheduled_work();

	// VACIAMOS EL BUFFER CIRCULAR (supone eliminar todos sus elementos)
	kfifo_reset(&cbuffer);

	// VACIAR LA LISTA
	list_for_each_safe(cur_node, aux, &mylist){
		item = list_entry(cur_node, struct list_item, links);
		list_del(cur_node);
		vfree(item);
	}

	// DECREMENTAR EN UNO EL CONTADOR INTERNO DE REFERENCIAS
	module_put(THIS_MODULE);
}

/* Se invocaal hacer open() de entrada /proc */
static int timer_open(struct inode *inode, struct file *file){
	// AUMENTAMOS EN UNO EL CONTADOR INTERNO DE REFERENCIAS
	try_module_get(THIS_MODULE);

	// ¿¿¿ ACTIVAMOS EL TIMER AQUI ???
	//add_timer(&my_timer);

	return 0;
}

/* Se invocaal hacer read() de entrada /proc */
static ssize_t timer_read(struct file *file, char *buff, size_t len, loff_t *offset){
	struct list_item* item = NULL;
	struct list_head* aux = NULL;
	struct list_head* cur_node = NULL;
	//char* kbuff = vmalloc(sizeof(int));

	list_for_each_safe(cur_node, aux, &mylist) {
		char* kbuff = vmalloc(sizeof(int));
		item = list_entry(cur_node, struct list_item, links);
		sprintf(kbuff,"%d",item->data);
		copy_to_user(buff,kbuff,sizeof(int));
		list_del(cur_node);
		vfree(item);
		vfree(kbuff);
	};
	
}

module_init(init_modul);
module_exit(cleanup_modul);