#include "headers.h"

static struct file_operations control_fops = {
	.owner 		= THIS_MODULE,
    //.read = proc_read,
	.open       = ctrl_open,
    .release    = ctrl_release,
    .write      = ctrl_write,
};

static int ctrl_release (struct inode *inode, struct file *file);
static int ctrl_open(struct inode *inode, struct file *file);
static ssize_t ctrl_read(struct file *file, char *buff, size_t len, loff_t *offset);
static ssize_t ctrl_write(struct file *file, const char *buff, size_t len, loff_t *offset);

/* Se invoca al hacer close() de entrada /proc/control */
static int ctrl_release (struct inode *inode, struct file *file){
	module_put(THIS_MODULE);
	return 0;
}

/* Se invocaal hacer open() de entrada/proc/control  */
static int ctrl_open(struct inode *inode, struct file *file){
	try_module_get(THIS_MODULE);
	return 0;
}

static ssize_t conf_write(struct file *file, const char *buff, size_t len, loff_t *offset){
	char* kbuff = NULL;
	char* name = NULL;

	kbuff = memdup_user_nul(buff, len);
	if (IS_ERR(kbuff))
		return PTR_ERR(kbuff);
    name = vmalloc(MAX_CHARS_NAME);
    if (IS_ERR(name))
		return PTR_ERR(name);

	// ADD
	if(sscanf(kbuff, "create %s",name )==1){
        if(numEntries < max_entries)
            add_entry(name);
	// REMOVE
	}else if(sscanf(kbuff,"delete %s",name)==1){
		remove_entry(name);
	}else{
        return -EINVAL;
    }

	kfree(kbuff)
    vfree(name);
    return MAX_CHARS_NAME;
}
