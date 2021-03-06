#include <linux/module.h>	/* Requerido por todos los módulos */
#include <linux/kernel.h>	/* Definición de KERN_INFO */
#include <linux/proc_fs.h>
#include <linux/vmalloc.h>
#include <asm/uaccess.h>
MODULE_LICENSE("GPL"); 	/*  Licencia del modulo */

struct list_head mylist; // LISTA ENLAZADA
struct proc_dir_entry* my_proc; // Entrada /proc

int sizeList = 0;

// NODOS DE LA LISTA
struct list_item {
	int data;
	struct list_head links;
};

static ssize_t proc_read(struct file *filp, char *buffer, size_t length, loff_t * offset);
static ssize_t proc_write(struct file *filp, const char *buff, size_t len, loff_t * off);

static struct file_operations fops = {
	.read = proc_read,
	.write = proc_write,
};



/************************************************************************************************************************/


/* Función que se invoca cuando se carga el módulo en el kernel */
int modulo_lin_init(void)
{
	my_proc = proc_create("modlist", 0666, NULL, &fops); // Creamos la entrada /proc/modlist
	if(my_proc == NULL){
		printk(KERN_INFO "Fallo al crear la entrada en /proc\n");
		return -EAGAIN; // Error: try again
	}
	else {
		INIT_LIST_HEAD(&mylist); // Inicializamos la lista enlazada
		printk(KERN_INFO "Modulo modlist cargado. Hola kernel.\n");
	}
	/* Devolver 0 para indicar una carga correcta del módulo */
	return 0;
}

/* Función que se invoca cuando se descarga el módulo del kernel */
void modulo_lin_clean(void)
{
	remove_proc_entry("modlist", NULL); // Eliminamos la entrada /proc/modlist

	// Ahora eliminamos el contenido de toda la lista y posteriormente liberamos memoria
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
		printk(KERN_INFO "Lista enlazada eliminada.\n");
	}
	printk(KERN_INFO "Modulo modlist descargado. Adios kernel.\n");
}

/* Funcion que se invoca cuando se desea leer la entrada /proc/modlist */
static ssize_t proc_read(struct file *filp, char *buffer, size_t length, loff_t * offset)
{
	// vmalloc() retorna un puntero a void pero le ponemos que sea puntero a char que es donde se almacena el buffer en el kernel para
	// pasarlo posteriormente al espacio de usuario con copy_to_user()
	char* kbuff = (char*)vmalloc((sizeList*sizeof(int))*2); // Aqui reservamos memoria y guardamos el principio de todo
	char* dest = kbuff; // Igualamos posicion con kbuff y dest es donde iremos sumando

	struct list_item* item = NULL;
	struct list_head* nodo = NULL;


	if(*offset>=sizeList)
		return 0;
	/* Recorremos nuestra lista y para ello deberemos guardar en "list_head* nodo" el puntero que apunta al campo next de la estructura 
		list_head. Luego recuperamos el puntero con list_entry() y guardamos en dest el contenido de item->data (gracias a sprintf())
		para posteriormente pasarlo al espacio de usuario. */
	if(!list_empty(&mylist)){

		list_for_each(nodo, &mylist){
			item = list_entry(nodo, struct list_item, links);
			dest += sprintf(dest, "%d\n", item->data);
		};
		
		// Ahora procedemos a pasar el contenido de kbuff al espacio de usuario, si no se produce error devuelve 0
		if(copy_to_user(buffer, kbuff, dest - kbuff +1) != 0){
			vfree(kbuff); // LIberamos la memoria que reservamos para almacenar la informacion
			return -EINVAL;
		}
	}
	printk(KERN_INFO "%d",*offset);
	vfree(kbuff);
	*offset += dest-kbuff;
	printk(KERN_INFO "%d",*offset);


	return *offset;
}

/* Funcion que se invoca cuando se desea escribir en la entrada /proc/modlist */
static ssize_t proc_write(struct file *filp, const char *buff, size_t len, loff_t * off)
{
	char* kbuff = (char*)vmalloc(len);
	struct list_item* item = NULL;
	struct list_head* cur_node = NULL;
	struct list_head* aux = NULL;
	char* clean = (char*)vmalloc(10);
	char* msg = "ERROR:Comando erroneo\n";
	char* cleanup = "cleanup";
	int n;

	if(copy_from_user(kbuff, buff, len) != 0){
		return -EACCES;
	}

	kbuff[len] = '\0';

	// ADD
	if(sscanf(kbuff, "add %d",&n )==1){
		item = (struct list_item*)vmalloc(sizeof(struct list_item));
		item->data = n;
		INIT_LIST_HEAD(&item->links);
		list_add_tail(&item->links,&mylist);
		sizeList += 1;
	}
	// REMOVE
	else if(sscanf(kbuff,"remove %d",&n)==1){
		list_for_each_safe(cur_node, aux, &mylist) {
			item = list_entry(cur_node, struct list_item, links);
			if(item->data == n){
				list_del(cur_node);
				vfree(item);
				sizeList -= 1;
			}
		};
	}
	// CLEAUNP
	else if(sscanf(kbuff,"%s",clean)==1){
		if(strcmp(clean,cleanup)==0){
			list_for_each_safe(cur_node, aux, &mylist) {
				item = list_entry(cur_node, struct list_item, links);
				list_del(cur_node);
				vfree(item);
				sizeList -= 1;

			};
		}else{
			vfree(kbuff);
			vfree(clean);
		 	return -EINVAL	;
		}
	}
	vfree(kbuff);
	vfree(clean);

	return len;
}

/* Declaración de funciones init y exit */
module_init(modulo_lin_init);
module_exit(modulo_lin_clean);
