#include "headers.h"

int timer_period_ms;
int max_random;
int emergency_threshold;

int conf_release (struct inode *inode, struct file *file);
int conf_open(struct inode *inode, struct file *file);
ssize_t conf_read(struct file *file, char *buff, size_t len, loff_t *offset);
ssize_t conf_write(struct file *file, const char *buff, size_t len, loff_t *offset);

struct file_operations config_fops = {
        .owner = THIS_MODULE,
	.read 	= conf_read,
	.open 	= conf_open,
	.release 	= conf_release,
    .write  = conf_write
};

/* Se invocaal hacerclose() de entrada/proc*/
int conf_release (struct inode *inode, struct file *file){
	module_put(THIS_MODULE);
}

/* Se invocaal haceropen() de entrada/proc*/
int conf_open(struct inode *inode, struct file *file){
	printk(KERN_INFO "im here");
	try_module_get(THIS_MODULE);
printk(KERN_INFO "im there");
return 0;
}

/* Se invocaal hacer read() de entrada/proc*/
ssize_t conf_read(struct file *file, char *buff, size_t len, loff_t *offset){
	char* kbuff = (char*)vmalloc(len);	
	int n;
	char* timer = (char*)vmalloc(50);
	char* random = (char*)vmalloc(50);
	char* emergency = (char*)vmalloc(50);
	
	printk(KERN_INFO "init okay");
	sprintf( timer, "%d", timer_period_ms);
	sprintf( random, "%d", max_random);
	sprintf( emergency, "%d", emergency_threshold);
	printk(KERN_INFO "printf okay");

	strcat(kbuff,"timer_period_ms=");		strcat(kbuff,timer);		strcat(kbuff,"\n");
	strcat(kbuff,"max_random=");			strcat(kbuff,random);		strcat(kbuff,"\n");
	strcat(kbuff,"emergency_threshold=");	strcat(kbuff,emergency);	strcat(kbuff,"\n");
	printk(KERN_INFO "str okay");
	//printk(kbuff);
	vfree(timer);
	vfree(random);
	vfree(emergency);
	if(copy_to_user(buff, kbuff, 400) != 0){
			vfree(kbuff); // LIberamos la memoria que reservamos para almacenar la informacion
			return -EINVAL;
		}
	offset += len;
	vfree(kbuff);
	return 0;
}

/* Se invocaal hacer write() de entrada/proc*/
ssize_t conf_write(struct file *file, const char *buff, size_t len, loff_t *offset){
	char* kbuff = NULL;
	int n;

	kbuff = memdup_user_nul(buff, len);
	if (IS_ERR(kbuff))
		return PTR_ERR(kbuff);

	// ADD
	if(sscanf(kbuff, "timer_period_ms %d",&n )==1){
		if(n>=0){
			timer_period_ms = n;
		}
	}
	// REMOVE
	else if(sscanf(kbuff,"max_random %d",&n)==1){
		if(n >= 0){
			max_random = n;
		}
	}
	// CLEAUNP
	else if(sscanf(kbuff,"emergency_threshold %d",&n)==1){
		if(n >= 0 && n <= 100){
			emergency_threshold = n;
		}
	}
	kfree(kbuff);

	return len;
}
