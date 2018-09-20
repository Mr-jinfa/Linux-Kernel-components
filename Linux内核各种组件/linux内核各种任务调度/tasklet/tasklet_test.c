/*
创建tasklet有两种方法
1.动态创建
2.静态创建
PS:tasklet是基于软中断上实现,故其服务函数不能执行阻塞操作.
*/

#include <linux/module.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/kdev_t.h>
#include <linux/cdev.h>
#include <linux/kernel.h>
#include <linux/interrupt.h>
#include <linux/delay.h>


struct tasklet_struct my_tasklet;

void tasklet_handler (unsigned long data);
#define STATIC
#ifdef STATIC
DECLARE_TASKLET(my_tasklet, tasklet_handler, 20);
#endif
void tasklet_handler (unsigned long data)
{
        printk(KERN_ALERT "%d:tasklet_handler is running.\n", data);
        mdelay(1000);
        tasklet_schedule(&my_tasklet);
}  

static int __init test_init(void)
{
#ifndef STATIC
        tasklet_init(&my_tasklet, tasklet_handler, 0);
#endif
        tasklet_schedule(&my_tasklet);
        return 0;
}  

static void __exit test_exit(void)
{
//		tasklet_disable(&my_tasklet);     /* 小任务现在被禁止,这个小任务不能运行 */
/* 这个函数首先等待该小任务执行完毕，然后再将它移去。*/
        tasklet_kill(&my_tasklet);
        printk(KERN_ALERT "test_exit running.\n");
}
MODULE_LICENSE("GPL");  

module_init(test_init);
module_exit(test_exit);
