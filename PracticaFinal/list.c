#include "headers.h"

/* SEMAFORO PARA CONTROL DE ENTRADAS */
/* SPINLOCK PARA ELEMENTOS DE LISTAS */
struct semaphore semEntries; 
rwlock_t rwl;
DEFINE_RWLOCK(rwl);

struct proc_dir_entry* parent_entry;

int max_entries = 4;
int max_size = 10;

int numEntries = 0;

/* PARAMETROS ENTRADA max_entries y max_size(de listas) */
module_param(max_entries, int, 0000);
MODULE_PARM_DESC(max_entries, "Max lists allowed");
module_param(max_size, int, 0000);
MODULE_PARM_DESC(max_size, "Max size of list allowed");

static struct list_head myEntries;

// DATOS NODO DE UNA ENTRADA
struct data_entry{
	char *name;
	int numElements;
	struct list_head private_list;
};

// NODOS DE LISTA ENTRADAS
struct entry_item{
	struct data_entry* data;
	struct proc_dir_entry* entry;
	struct list_head links;
};

// NODOS DE LAS LISTAS DE CADA ENTRADA
struct list_item {
	char* data;
	struct list_head links;
};

int add_entry(char* name);
int remove_entry(char* name);
void remove_all_entries(void);

static int list_release (struct inode *inode, struct file *file);
static int list_open(struct inode *inode, struct file *file);
static ssize_t list_read(struct file *filp, char *buffer, size_t length, loff_t * offset);
static ssize_t list_write(struct file *filp, const char *buff, size_t len, loff_t * off);

static struct file_operations list_fops = {
	.owner 		= THIS_MODULE,
	.open = list_open,
	.release = list_release,
	.read = list_read,
	.write = list_write,
};


int add_entry(char* name){
	struct entry_item* item = NULL;
	struct list_head* cur_node = NULL;
	struct list_head* aux = NULL;
	int boolean = 0;

	//COMPRUEBA QUE NO HAY OTRA ENTRADA CON ESE NOMBRE
	down(&semEntries);
	if(numEntries>0){ 
		list_for_each_safe(cur_node, aux, &myEntries) {
			item = list_entry(cur_node, struct entry_item, links);
			if( strcmp(item->data->name,name) == 0)
				boolean = 1;
		};

	}if(numEntries >= max_entries)
		boolean = 2;

	if(!boolean){ //Y QUE NO SE SUPERA EL NUMERO MAXIMO DE ENTRADAS
		struct entry_item* entry = (struct entry_item*)vmalloc(sizeof(struct entry_item));
		entry->data = (struct data_entry*)vmalloc(sizeof(struct data_entry));
		entry->data->name = (char*)vmalloc(25);
		entry->data->numElements = 0;
		strcpy(entry->data->name,name);
		INIT_LIST_HEAD(&entry->data->private_list);//INIT LISTA
		entry->entry = proc_create_data(name,0666,parent_entry,&list_fops,entry->data);
		list_add_tail(&entry->links,&myEntries);
		++numEntries;
	}
	up(&semEntries);
	printk(KERN_INFO "entries %d",numEntries);
	
	return boolean;
}

int remove_entry(char* name){
	struct entry_item* item = NULL;
	struct list_item* item2 = NULL;
	struct list_head* cur_node = NULL;
	struct list_head* cur_node2 = NULL;
	struct list_head* aux = NULL;
	struct list_head* aux2 = NULL;
	int boolean = 0;

	//COMPRUEBA QUE HAY UNA ENTRADA CON ESE NOMBRE y LA ELIMINA
	if(numEntries>0){ 
		down(&semEntries);
		list_for_each_safe(cur_node, aux, &myEntries) { //BUCLE ENTRADAS
			item = list_entry(cur_node, struct entry_item, links);
			write_lock(&rwl);
			if(strcmp(item->data->name,name) == 0){ //nombres iguales/coinciden
				write_unlock(&rwl);
				boolean = 1;
				//elimina proc_dir_entry con ese nombre
				remove_proc_entry(item->data->name,parent_entry); 
				write_lock(&rwl);

				//BUCLE LISTA , libera nodos lista
				list_for_each_safe(cur_node2, aux2, &(item->data->private_list) ) { 
					item2 = list_entry(cur_node2, struct list_item, links);
					vfree(item2->data);
					list_del(cur_node2);
					vfree(item2);
				};

				vfree(item->data->name); 
				vfree(item->data);
				list_del(cur_node);
				vfree(item);
				--numEntries;
			}
			write_unlock(&rwl);
		};
		up(&semEntries);
	}
	printk(KERN_INFO "entries %d",numEntries);
	return boolean;
}

void remove_all_entries(void){
	struct entry_item* item = NULL;
	struct list_item* item2 = NULL;
	struct list_head* cur_node = NULL;
	struct list_head* cur_node2 = NULL;
	struct list_head* aux = NULL;
	struct list_head* aux2 = NULL;

	printk(KERN_INFO "Va a entrar a remove all todo");
	if(numEntries>0){ 
		down(&semEntries);
		//LOOP DE ENTRIES
		list_for_each_safe(cur_node, aux, &myEntries) { 
			item = list_entry(cur_node, struct entry_item, links);
			//elimina proc_dir_entry
			remove_proc_entry(item->data->name,parent_entry); 
			write_lock(&rwl);
			vfree(item->data->name); 
			//BUCLE LISTA , libera nodos lista
			list_for_each_safe(cur_node2, aux2, &(item->data->private_list) ) { 
				item2 = list_entry(cur_node2, struct list_item, links);
				vfree(item2->data);
				list_del(cur_node2);
				vfree(item2);
			};
			vfree(item->data);
			write_unlock(&rwl);
			list_del(cur_node);
			vfree(item);
		};
		up(&semEntries);
		numEntries = 0;
	}
	printk(KERN_INFO "entries %d",numEntries);
}

/* Función que se invoca cuando se carga el módulo en el kernel */
int modulo_lin_init(void)
{
	if(max_entries < 1 || max_size <= 0)
		return -EAGAIN;
	sema_init(&semEntries,1);
	rwlock_init(&rwl);
	INIT_LIST_HEAD(&myEntries);//INIT LISTA
	parent_entry = proc_mkdir("list", NULL);
	control_entry = proc_create_data("control",0666,parent_entry,&control_fops,NULL);
	if(parent_entry == NULL)
		return -EAGAIN; // Error: try again
	else
		add_entry("default");
	
	printk(KERN_INFO "Modulo cargado correctamente, max_entries=%d y max_size=%d.",max_entries,max_size);
	return 0;
}

/* Se invoca al hacer close() de entrada /proc/"entry" */
static int list_release (struct inode *inode, struct file *file){
	module_put(THIS_MODULE);
	return 0;
}

/* Se invocaal hacer open() de entrada/proc/entry  */
static int list_open(struct inode *inode, struct file *file){
	try_module_get(THIS_MODULE);
	return 0;
}

/* Función que se invoca cuando se descarga el módulo del kernel */
void modulo_lin_clean(void)
{
	remove_all_entries();

	remove_proc_entry("control",parent_entry);
	remove_proc_entry("list", NULL); // Eliminamos la entrada /proc/list

	printk(KERN_INFO "Modulo descargado correctamente.");
}

/* Funcion que se invoca cuando se desea leer la entrada /proc/list/(list) */
static ssize_t list_read(struct file *filp, char *buffer, size_t length, loff_t * offset)
{
	char* kbuff = NULL; 
	char* dest = NULL;
	struct data_entry* private_data = NULL;

	struct list_item* item = NULL;
	struct list_head* nodo = NULL;


	if(*offset>0)
	   return 0;

	kbuff = (char*)vmalloc(1024);
	dest = kbuff;
	
	read_lock(&rwl);
	private_data = (struct data_entry*)PDE_DATA(filp->f_inode);
	list_for_each(nodo, &private_data->private_list){
		item = list_entry(nodo, struct list_item, links);
		dest += sprintf(dest, "%s\n", item->data);
	};
	read_unlock(&rwl);
		
	if(copy_to_user(buffer, kbuff, dest - kbuff) != 0){
		vfree(kbuff);
		return -EACCES;
	}
	vfree(kbuff);
	*offset += dest-kbuff;

	return dest-kbuff;
}

/* Funcion que se invoca cuando se desea escribir en la entrada /proc/modlist */
static ssize_t list_write(struct file *filp, const char *buff, size_t len, loff_t * off)
{
	char* kbuff = NULL;
	struct list_item* item = NULL;
	struct list_head* cur_node = NULL;
	struct list_head* aux = NULL;
	struct data_entry* private_data = NULL;
	int n;


	kbuff = (char*)vmalloc(len);
	if(copy_from_user(kbuff, buff, len) != 0){
		vfree(kbuff);
		return -EACCES;
	}

	n = 0;
	kbuff[len] = '\0';
	private_data = (struct data_entry*)PDE_DATA(filp->f_inode);

	// ADD
	if(sscanf(kbuff, "add %s*",kbuff )==1){
		if(private_data->numElements >= max_size){
			vfree(kbuff);
			return -EACCES;
		}
		item = (struct list_item*)vmalloc(sizeof(struct list_item));
		item->data = vmalloc(sizeof(char)*len);
		strcpy(item->data,kbuff);

		write_lock(&rwl);
		list_add_tail(&item->links,&private_data->private_list);
		private_data->numElements++;
		write_unlock(&rwl);
	}
	
	// REMOVE
	else if(sscanf(kbuff,"remove %s",kbuff)==1){
		write_lock(&rwl);
		list_for_each_safe(cur_node, aux, &private_data->private_list) {
			item = list_entry(cur_node, struct list_item, links);
			if(strcmp(kbuff,item->data)==0){
				n = 1;
				vfree(item->data);
				list_del(cur_node);
				vfree(item);
				private_data->numElements--;
			}
		}
		write_unlock(&rwl);
		if(n != 1){	
			vfree(kbuff);
			return -EFAULT;
		}
	}

	// CLEAUNP
	else if(strcmp(kbuff,"cleanup\n")==0){
		write_lock(&rwl);
		list_for_each_safe(cur_node, aux, &private_data->private_list) {
			item = list_entry(cur_node, struct list_item, links);
			vfree(item->data);
			list_del(cur_node);
			vfree(item);
		}
		write_unlock(&rwl);
		
	}else{
		vfree(kbuff);
		return -EPERM;
	}
		vfree(kbuff);
		return len;
}

/* Declaración de funciones init y exit */
module_init(modulo_lin_init);
module_exit(modulo_lin_clean);