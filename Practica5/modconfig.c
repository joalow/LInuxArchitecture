#include "headers.h"

#define MAX_CBUFFER_LEN 32

static struct file_operations timer_fops = {
	.read 	= conf_read,
	.open 	= conf_open,
	.release 	= conf_release,
    .write  = conf_write
};

int timer_period_ms;
int max_random;
int emergency_threshold;

static int conf_release (struct inode *inode, struct file *file);
static int conf_open(struct inode *inode, struct file *file);
static ssize_t conf_read(struct file *file, char *buff, size_t len, loff_t *offset);
static ssize_t conf_write(struct file *file, const char *buff, size_t len, loff_t *offset);

/* Se invocaal hacerclose() de entrada/proc*/
static int conf_release (struct inode *inode, struct file *file){
	module_put(THIS_MODULE);
}

/* Se invocaal haceropen() de entrada/proc*/
static int conf_open(struct inode *inode, struct file *file){
    timer_period_ms = 500;
    max_random = 1000;
    emergency_threshold = 75;

	try_module_get(THIS_MODULE);
}

/* Se invocaal hacer read() de entrada/proc*/
static ssize_t conf_read(struct file *file, char *buff, size_t len, loff_t *offset){
	char* kbuff = (char*)vmalloc(len);
	kbuff = "timer_period_ms="+timer_period_ms+'\n'+
			"max_random="+max_random+'\n'+
			"emergency_thresold="+emergency_threshold+'\n';
	return copy_to_user(buff,kbuff,len);		
}

/* Se invocaal hacer write() de entrada/proc*/
static ssize_t conf_write(struct file *file, const char *buff, size_t len, loff_t *offset){
	char* kbuff = NULL;
	char* msg = "ERROR:Comando erroneo\n";
	int n;

	kbuff = memdup_user_nul(buff, len);
	if (IS_ERR(kbuf))
		return PTR_ERR(kbuf);

	// ADD
	if(sscanf(kbuff, "time_period_ms %d",&n )==1){
		if(n>=0){
			time_period_ms = n;
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