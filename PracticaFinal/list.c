#include "headers.h"

//rwlock_t rwl;
//DEFINE_RWLOCK(rwl);

struct proc_dir_entry* parent_entry;

int max_entries = 4;
int max_size = 10;

int numEntries = 0;

module_param(max_entries, int, 0000);
MODULE_PARM_DESC(max_entries, "Max lists allowed");
module_param(max_size, int, 0000);
MODULE_PARM_DESC(max_size, "Max size of list allowed");

//struct list_head mylist; // LISTA ENLAZADA
//struct proc_dir_entry *proc_create_data(const char *name, umode_t mode, struct proc_dir_entry *parent, const struct file_operations *ops, void *private_data);
static struct list_head myEntries;

static struct data_entry{
	char *name ;
	int numElements = 0;
	struct list_head private_list;
};

//NODOS DE LISTA ENTRADAS
struct entry_item{
	struct data_entry data;
	struct proc_dir_entry* entry;
	struct list_head links;
};

// NODOS DE LA LISTA
struct list_item {
	#ifdef P_01
		char* data = (char*)vmalloc(1000);
	#else
		int data;
	#endif
	struct list_head links;
};

static int add_entry(char* name);
static int remove_entry(char* name);
static void remove_all_entries();

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


static int add_entry(char* name){
	struct list_item* item = NULL;
	struct list_head* cur_node = NULL;
	struct list_head* aux = NULL;
	int boolean = 0;

	//COMPRUEBA QUE NO HAY OTRA ENTRADA CON ESE NOMBRE
	list_for_each_safe(cur_node, aux, &myEntries) {
		item = list_entry(cur_node, struct entry_item, links);
		if(item.data.name == name)
			boolean = 1;
	};
	if(boolean != 1 && numEntries < max_entries){ //Y QUE NO SE SUPERA EL NUMERO MAXIMO DE ENTRADAS
		struct entry_item entry = vmalloc(sizeof(struct entry_item));
		entry.data = vmalloc(sizeof(struct data_entry));
		entry.data.name = vmalloc(25);
		strcpy(entry.data.name,name);
		INIT_LIST_HEAD(&entry.data.private_list);//INIT LISTA
		entry.entry = proc_create_data(name,0666,parent_entry,list_fops,&entry.data);
		list_add_tail(&item->links,&myEntries);
		++numEntries;
	}
	
	return boolean;
}

static int remove_entry(char* name){
	struct list_item* item = NULL;
	struct list_head* cur_node = NULL;
	struct list_head* aux = NULL;
	int boolean = 0;

	//COMPRUEBA QUE HAY UNA ENTRADA CON ESE NOMBRE y LA ELIMINA
	list_for_each_safe(cur_node, aux, &myEntries) {
		item = list_entry(cur_node, struct entry_item, links);
		if(strcmp(item.data.name,name) == 0){ //nombres iguales
			boolean = 1;
			remove_proc_entry(item.data.name,parent_entry); //elimina proc_dir_entry
			vfree(item.data.name); //libera data.name
			vfree(item.data);
			list_del(cur_node);
			vfree(item);
			--numEntries;
		}
	};

	return boolean;
}

static void remove_all_entries(){
	if(!list_empty(&myEntries)){ 
		struct entry_item* item = NULL,		item2=NULL;
		struct list_head* cur_node = NULL, 	cur_node2=NULL;
		struct list_head* aux = NULL, 		aux2=NULL;

		list_for_each_safe(cur_node, aux, &myEntries) {
			item = list_entry(cur_node, struct entry_item, links);
			remove_proc_entry(item.data.name,parent_entry); //elimina proc_dir_entry
			vfree(item.data.name); //libera data.name
				list_for_each_safe(cur_node2, aux2, &item.data.private_list.links) { //libera nodos lista
					item2 = list_entry(cur_node2, struct list_item, links);
					list_del(cur_node2);
					vfree(item2);
				};
			vfree(item.data);
			list_del(cur_node);
			vfree(item);
		};
		numEntries = 0;
	}
}

/* Función que se invoca cuando se carga el módulo en el kernel */
int modulo_lin_init(void)
{
	rwlock_init(&rwl);

	parent_entry = proc_mkdir("list", NULL);
	if(parent_entry == NULL)
		return -EAGAIN; // Error: try again
	else{ 
		add_entry("default");
	}
	/* Devolver 0 para indicar una carga correcta del módulo */
	printk(KERN_INFO "Modulo cargado correctamente.");
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

	remove_proc_entry("list", NULL); // Eliminamos la entrada /proc/list

	printk(KERN_INFO "Modulo descargado correctamente.");
}

/* Funcion que se invoca cuando se desea leer la entrada /proc/modlist */
static ssize_t list_read(struct file *filp, char *buffer, size_t length, loff_t * offset)
{
/*	// vmalloc() retorna un puntero a void pero le ponemos que sea puntero a char que es donde se almacena el buffer en el kernel para
	// pasarlo posteriormente al espacio de usuario con copy_to_user()
	char* kbuff = (char*)vmalloc(1024); // Aqui reservamos memoria y guardamos el principio de todo
	char* dest = kbuff; // Igualamos posicion con kbuff y dest es donde iremos sumando

	struct list_item* item = NULL;
	struct list_head* nodo = NULL;


	if(*offset>0)
	   return 0;
	
	read_lock(&rwl);
	list_for_each(nodo, &mylist){
		item = list_entry(nodo, struct list_item, links);
		dest += sprintf(dest, "%d\n", item->data);
	};
	read_unlock(&rwl);
		
		// Ahora procedemos a pasar el contenido de kbuff al espacio de usuario, si no se produce error devuelve 0
	if(copy_to_user(buffer, kbuff, dest - kbuff) != 0){
		vfree(kbuff); // LIberamos la memoria que reservamos para almacenar la informacion
		return -EINVAL;
	}
	vfree(kbuff);
	*offset += dest-kbuff;

	return dest-kbuff;*/
	return 0;
}

/* Funcion que se invoca cuando se desea escribir en la entrada /proc/modlist */
static ssize_t list_write(struct file *filp, const char *buff, size_t len, loff_t * off)
{
	/*char* kbuff = (char*)vmalloc(len);
	struct list_item* item = NULL;
	struct list_head* cur_node = NULL;
	struct list_head* aux = NULL;
	int n;

	if(copy_from_user(kbuff, buff, len) != 0){
		return -EACCES;
	}

	kbuff[len] = '\0';

	// ADD
	#ifdef P_01
	if(sscanf(kbuff, "add %s",kbuff )==1){
		item = (struct list_item*)vmalloc(sizeof(struct list_item));
		strcpy(item->data,kbuff);

		write_lock(&rwl);
		list_add_tail(&item->links,&mylist);
		write_unlock(&rwl);
	}
	#else
	if(sscanf(kbuff, "add %d",&n )==1){
		item = (struct list_item*)vmalloc(sizeof(struct list_item));
		write_unlock(&rwl);
		item->data = n;
		list_add_tail(&item->links,&mylist);
		write_unlock(&rwl);
	}
	#endif
	
	// REMOVE
	#ifdef P_01
	else if(sscanf(kbuff,"remove %s",kbuff)==1){
		write_lock(&rwl);
		list_for_each_safe(cur_node, aux, &mylist) {
			item = list_entry(cur_node, struct list_item, links);
			if(strcmp(kbuff,item->data)==0){
				list_del(cur_node);
				vfree(item);
			}
		}
		write_unlock(&rwl);
	}
	#else
	else if(sscanf(kbuff,"remove %d",&n)==1){
		write_lock(&rwl);
		list_for_each_safe(cur_node, aux, &mylist) {
			item = list_entry(cur_node, struct list_item, links);
			if(item->data == n){
				list_del(cur_node);
				vfree(item);
			}
		}
		write_unlock(&rwl);
	}
	#endif

	// CLEAUNP
	else if(strcmp(kbuff,"cleanup\n")==0){
		write_lock(&rwl);
		list_for_each_safe(cur_node, aux, &mylist) {
			item = list_entry(cur_node, struct list_item, links);
			list_del(cur_node);
			vfree(item);
		}
		write_unlock(&rwl);
	}else{
		vfree(kbuff);
		return -EINVAL;
	}

	vfree(kbuff);


	return len;*/
	return 0;
}

/* Declaración de funciones init y exit */
module_init(modulo_lin_init);
module_exit(modulo_lin_clean);