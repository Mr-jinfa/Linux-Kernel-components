#if 0
创建一个队列就会有一个内核线程，一般不要轻易创建队列,它位于进程上下文--->可以睡眠
定义:
	struct work_struct work;

初始化:
	INIT_WORK(struct work_struct *work, void (*func)(struct work_struct *work));

定义并初始化:
	DECLARE_WORK(name, void (*func)(struct work_struct *work)); 

调度:
	int schedule_work(struct work_struct *work);
	返回1成功, 0已经添加在队列上

延迟调度:
	int schedule_delayed_work(struct work_struct *work, unsigned long delay);

创建新队列和新工作者线程:
	struct workqueue_struct *create_workqueue(const char *name);	//多少个cpu就多少个工作者线程
	struct workqueue_struct *create_singlethread_workqueue(const char *name);

调度指定队列:
	int queue_work(struct workqueue_struct *wq, struct work_struct *work);

延迟调度指定队列:
	int queue_delayed_work(struct workqueue_struct *wq, struct work_struct *work, unsigned long delay);

销毁队列:
	void destroy_workqueue(struct workqueue_struct *wq);
#endif

#include <linux/module.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/kdev_t.h>
#include <linux/cdev.h>
#include <linux/kernel.h>
#include <linux/interrupt.h>
#include <linux/delay.h>



struct workqueue_struct *p_ctrl_queue;

//定义并初始化
//static DECLARE_WORK(ctrl_work, ctrl);

//定义一个工作队列结构体
struct work_struct ctrl_work;

static void ctrl(struct work_struct *work)
{
	msleep(1000);
	printk(KERN_ERR "schedule ctrl\n");
//手动调度
//	schedule_work(&ctrl_work);
}
static int __init test_init(void)
{
	//	Create work queue
	p_ctrl_queue = create_singlethread_workqueue("ctrl");
	if (!p_ctrl_queue)
	{
		return -ENODEV;
	}
	INIT_WORK(&ctrl_work,ctrl);
//由定时器自动调度
	queue_work(p_ctrl_queue, &ctrl_work);

    return 0;
}  

static void __exit test_exit(void)
{
	// Release work queue
	flush_workqueue(p_ctrl_queue);
	destroy_workqueue(p_ctrl_queue);
    printk(KERN_ALERT "test_exit \n");
}
MODULE_LICENSE("GPL");  

module_init(test_init);
module_exit(test_exit);
