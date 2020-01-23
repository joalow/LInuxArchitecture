#include "headers.h"

struct file_operations control_fops = {
	.owner 		= THIS_MODULE,
    //.read = proc_read,
	.open       = ctrl_open,
    .release    = ctrl_release,
    .write      = ctrl_write,
};

struct proc_dir_entry* control_entry;

int ctrl_release (struct inode *inode, struct file *file);
int ctrl_open(struct inode *inode, struct file *file);
ssize_t ctrl_read(struct file *file, char *buff, size_t len, loff_t *offset);
ssize_t ctrl_write(struct file *file, const char *buff, size_t len, loff_t *offset);

/* Se invoca al hacer close() de entrada /proc/control */
int ctrl_release (struct inode *inode, struct file *file){
	module_put(THIS_MODULE);
	return 0;
}

/* Se invocaal hacer open() de entrada/proc/control  */
int ctrl_open(struct inode *inode, struct file *file){
	try_module_get(THIS_MODULE);
	return 0;
}

ssize_t ctrl_write(struct file *file, const char *buff, size_t len, loff_t *offset){
	char* kbuff = NULL;
	char* name = NULL;
	int boolean;

	kbuff = memdup_user_nul(buff, len);
	if (IS_ERR(kbuff))
		return PTR_ERR(kbuff);
    name = vmalloc(MAX_CHARS_NAME);
    if (IS_ERR(name))
		return PTR_ERR(name);

	// ADD
	if(sscanf(kbuff, "create %s",name )==1){
        boolean = add_entry(name);
        if(boolean == 1)
        	return -EEXIST;
        else if(boolean==2)
        	return -EPERM;
	// REMOVE
	}else if(sscanf(kbuff,"delete %s",name)==1){
		if(!remove_entry(name))
			return -ENOENT;
	}else{
        return -EPERM;
    }

	kfree(kbuff);
    vfree(name);
    return MAX_CHARS_NAME;
}
