
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/errno.h>
#include <linux/kfifo.h>

// TODO preguntar sobre MAXCBUFF Y KBUFF

struct kfifo cbuffer;/* Buffer circular*/
int prod_count = 0;/* Númerode procesos que abrieron la entrada/proc para escritura(productores) */
int cons_count = 0;/* Númerode procesos que abrieron la entrada/proc para lectura(consumidores) */

struct semaphore mtx;/* para garantizar Exclusión Mutua*/
struct semaphore sem_prod;/* cola de espera para productor(es) */
struct semaphore sem_cons;/* cola de espera para consumidor(es) */
 
int nr_prod_waiting = 0;/* Número de procesos productores esperando*/
int nr_cons_waiting = 0;/* Número de procesos consumidores esperando*/

struct proc_dir_entry* my_proc; // Entrada /proc

static struct file_operations fops = {
	.read 	= fifoproc_read,
	.write 	= fifoproc_write,
	.open 	= fifoproc_open,
	.close 	= fifoproc_release
};

/* Funcionesde inicialización y descargadel módulo*/
int init_module(void){
	kfifo_alloc(&cbuffer,MAX_CBUFFER_LEN,GFP_KERNEL);
    sema_init(&mtx,1);
	sema_init(&sem_prod,0);
	sema_init(&sem_cons,0);Detectar fin de
	my_proc = proc_create("prodcons", 0666, NULL, &fops); // Creamos la entrada /proc/modlist
	if(my_proc == NULL)
		return -EAGAIN; // Error: ty again
	
	return 0;
}

void cleanup_module(void){
	remove_proc_entry("prodcons",NULL);
	kfifo_free(&cbuffer);
}

/* Se invocaal hacerclose() de entrada/proc*/
static int fifoproc_release(struct inode *inode, struct file *file, ){
	if(file->f_mode & FMODE_READ)
        cons_count--;
    else
        prod_count--;

    if(cons_count == 0 && prod_count == 0)
        kfifo_reset(&cbuffer);
}

/* Se invocaal haceropen() de entrada/proc*/
static int fifoproc_open(struct inode *inode, struct file *file){
    if (file->f_mode & FMODE_READ){
        /* Un consumidorabrió el FIFO*/
        cons_count++;
	    if(down_interruptible(&mtx))
	      return -EINTR;

	    while(prod_count == 0){
	        nr_cons_waiting++;
	        up(&mtx); /*Libera el 'mutex' */
	        /*se bloquea en la cola */
	        if(down_interruptible(&sem_cons)){
	           down(&mtx);
	           nr_cons_waiting--;
	           up(&mtx);
	           return -EINTR;
	        }
	        /* Adquiere el mutx */
	        if(down_interruptible(&mtx))
	            return -EINTR;
	    }
    	/* Libera el mute */
  	  	up(&mtx);

  	  	if(down_interruptible(&mtx))
     		 return -EINTR;

	    if(nr_cons_waiting > 0){
	        /* Despierta 1 de los hilos bloqueados */
	        up(&sem_cons);
	        nr_cons_waiting--;
	    }
	    up(&mtx);
    	//cond_signal(sem_cons, &nr_cons_waiting);

    } else{
	        /* Un productorabrió el FIFO*/
	     prod_count++;
	     if(down_interruptible(&mtx))
	      return -EINTR;

	    while(cons_count == 0 ){
	       nr_prod_waiting++;
	        up(&mtx); /*Libera el 'mutex' */
	        /*se bloquea en la cola */
	        if(down_interruptible(&sem_prod)){
	            down(&mtx);
	            nr_prod_waiting--;
	            up(&mtx);
	            return -EINTR;
	        }
	        /* Adquiere el mutx */
	        if(down_interruptible(&mtx))
	            return -EINTR;
	    }
	    /* Libera el mute */
	    up(&mtx);

	    if(down_interruptible(&mtx))
     		 return -EINTR;

	    if(nr_prod_waiting > 0){
	        /* Despierta 1 de los hilos bloqueados */
	        up(&sem_prod);
	        nr_prod_waiting--;
	    }
	    up(&mtx);
	    //cond_signal(sem_prod, &nr_prod_waiting);
    }
    return 0;
}

/* Se invocaal hacer read() de entrada/proc*/
static ssize_t fifoproc_read(struct file *file, char *str, size_t len, loff_t *offset){
	char kbuffer[MAX_KBUF];

    if (len > MAX_CBUFFER_LEN || len > MAX_KBUF) 
        return -ENOBUFS;

     if(down_interruptible(&mtx))
      return -EINTR;

    while(kfifo_len(&cbuffer)<len && prod_count>0){
        nr_cons_waiting++;
        up(&mtx); /*Libera el 'mutex' */
        /*se bloquea en la cola */
        if(down_interruptible(&sem_cons)){
            down(&mtx);
            nr_cons_waiting--;
            up(&mtx);
            return -EINTR;
        }
        /* Adquiere el mutx */
        if(down_interruptible(&mtx))
            return -EINTR;
    }

    if (prod_count == 0 && kfifo_is_empty(&cbuffer)) {
        up(&mtx);
        return 0;
    }

    kfifo_out(&cbuffer,kbuffer,len);/* Despertar a posible productor bloqueado*/
    
    //cond_signal(prod);
    /* Libera el mute */
    if(nr_prod_waiting > 0){
        /* Despierta 1 de los hilos bloqueados */
        up(&sem_prod);
        nr_prod_waiting--;
    }
    up(&mtx);
    return len;
}

/* Se invocaal hacer write() de entrada/proc*/
int fifoproc_write(char* buff,int len) {
	char kbuffer[MAX_KBUF];
	if (len> MAX_CBUFFER_LEN || len> MAX_KBUF) {
		return -ENOBUFS;
	}if (copy_from_user(kbuffer,buff,len)) {
		return -ENOSPC;
	}	

	if(down_interruptible(&mtx))
      return -EINTR;

    while(kfifo_avail(&cbuffer)<len && cons_count>0){
        nr_prod_waiting++;
        up(&mtx); /*Libera el 'mutex' */
        /*se bloquea en la cola */
        if(down_interruptible(&sem_prod)){
            down(&mtx);
            nr_prod_waiting--;
            up(&mtx);
            return -EINTR;
        }
        /* Adquiere el mutx */
        if(down_interruptible(&mtx))
            return -EINTR;
    }
	/* Detectar fin de comunicación por error(consumidor cierra FIFO antes) */
	if (cons_count==0) {
		unlock(mtx);
		return -EPIPE;
	}
	kfifo_in(&cbuffer,kbuffer,len);
	/* Despertar a posible consumidor bloqueado*/
	
	//cond_signal(cons,&nr_cons_waiting);
	if(nr_cons_waiting > 0){
        /* Despierta 1 de los hilos bloqueados */
        up(&sem_cons);
        nr_cons_waiting--;
    }

    /* Libera el mute */
    up(&mtx);

	return len;
}

module_init(init_module);
module_exit(cleanup_module);
