#ifndef _HEADERS_H_
#define _HEADERS_H_

#include <linux/module.h>	/* Requerido por todos los módulos */
#include <linux/moduleparam.h>
#include <linux/kernel.h>	/* Definición de KERN_INFO */
#include <linux/proc_fs.h>
#include <linux/vmalloc.h>
#include <linux/slab.h>
#include <linux/seq_file.h>
#include <asm/uaccess.h>
#include <linux/delay.h>
#include <linux/rwlock.h>
MODULE_LICENSE("GPL");

#define MAX_CHARS_NAME 25

extern int max_entries;
extern int max_size;
extern int numEntries;

extern struct proc_dir_entry* control_entry;
extern struct file_operations control_fops;

extern int add_entry(char* name);
extern int remove_entry(char* name);
extern void remove_all_entries(void);

extern int ctrl_release (struct inode *inode, struct file *file);
extern int ctrl_open(struct inode *inode, struct file *file);
extern ssize_t ctrl_read(struct file *file, char *buff, size_t len, loff_t *offset);
extern ssize_t ctrl_write(struct file *file, const char *buff, size_t len, loff_t *offset);

#endif