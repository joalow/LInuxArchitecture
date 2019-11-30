#ifndef _HEADERS_H_
#define _HEADERS_H_

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/errno.h>
#include <linux/proc_fs.h>
#include <linux/vmalloc.h>
#include <linux/slab.h>
#include <asm/uaccess.h>
#include <linux/spinlock.h>
#include <linux/random.h>
#include <linux/kfifo.h>

extern int time_period_ms;
extern int max_random;
extern int emergency_threshold;

#endif