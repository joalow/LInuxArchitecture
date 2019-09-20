#include <linux/module.h>	/* Requerido por todos los módulos */
#include <linux/kernel.h>	/* Definición de KERN_INFO */
#include <linux/vmalloc.h>
MODULE_LICENSE("GPL"); 	/*  Licencia del modulo */

struct list_head mylist; //lista enlazada

//Nodos de la lista
struct list_item{
	int data;
	struct list_head links;
};

static int list_size = 0; 
static int Device_Open = 0;	/* Is device open?

/* Función que se invoca cuando se carga el módulo en el kernel */
int modulo_lin_init(void)
{
	INIT_LIST_HEAD(&mylist);

	printk(KERN_INFO "Modulo LIN cargado. Hola kernel.\n");

	/* Devolver 0 para indicar una carga correcta del módulo */
	return 0;
}

/* Función que se invoca cuando se descarga el módulo del kernel */
void modulo_lin_clean(void)
{
	struct list_item pos;

	printk(KERN_INFO "Modulo LIN descargado. Adios kernel.\n");
	for(pos = (mylist).next; pos != (head); pos = pos.links->next)
		list_del(pos->next);
		//vfree(pos);
	}
	vfree(mylist);
}

static int device_open(struct inode *, struct file *)
{
	if (Device_Open)
        return -EBUSY;

    Device_Open++;

    printk(KERN_INFO  "Device opened\n");

    return SUCCESS;
}

static int device_release(struct inode *inode, struct file *file)
{
    if(Device_Open)
        Device_Open--;
    return 0;
}

static ssize_t device_read(struct file *filp, char *buffer,size_t length, loff_t * offset)
{
	char* dst = malloc(list_size);
	char* kbuf = dst;
	struct list_head pos;

	list_for_each(pos,mylist){
		dst += sprintf(dst, "%d\n",pos->data);
	}
	copy_to_user(buffer,kbuf,dst-kbuf);

	return dst-kbuf;
}


static ssize_t device_write(struct file *filp, const char *buff, size_t len, loff_t * off)
{
	char* kbuf = kmalloc(len*sizeof(char),GFP_KERNEL);
	list_item node = vmalloc(sizeof(list_item));
	int n;

	if(len > 0) {
		if(copy_from_user(kbuf,buff,len))
			return -EFAULT;

		if(sscanf(kbuf,"add %d",&n) == 1){
			node.data = n;	node.links = mylist;
			list_add_tail(node,mylist);
			list_size += sizeof(int) + sizeof(char);
		}if(sscanf(kbuf,"remove %d", &n) == 1){
			node.data = n;	node.links = mylist;
			list_del(node,mylist);
			list_size -= sizeof(int) + sizeof(char);
		}
	}	
	return len == 0 ? 4 : len;
}

/* Declaración de funciones init y exit */
module_init(modulo_lin_init);
module_exit(modulo_lin_clean);

