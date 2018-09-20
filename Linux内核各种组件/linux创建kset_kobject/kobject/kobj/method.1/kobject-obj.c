/*
 * Sample kobject implementation
 *
 * Copyright (C) 2004-2007 Greg Kroah-Hartman <greg@kroah.com>
 * Copyright (C) 2007 Novell Inc.
 *
 * Released under the GPL version 2 only.
 *
 * log:2018.9.19 by jinfa
 ------------------------修改:-------------------
 在原基础上增加一个kobject到另一个kobject的软连接
 创建属性组由device_register来做
 ------------------------------------------------
 */
#include <linux/kobject.h>
#include <linux/string.h>
#include <linux/sysfs.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/errno.h>
#include <linux/device.h>
#include <linux/fs.h>
#include <linux/input.h>


/*
 * This module shows how to create a simple subdirectory in sysfs called
 * /sys/kernel/kobject-example  In that directory, 2 files are created:
 * "baz", and "bar".  If an integer is written to these files, it can be
 * later read out of it.
 */

static int baz;
static int bar;

static struct kobject *link_kobj;

static struct input_dev *input_dev;

/*
 * More complex function where we determine which variable is being accessed by
 * looking at the attribute for the "baz" and "bar" files.
 */
static ssize_t b_show(struct kobject *kobj, struct kobj_attribute *attr,
		      char *buf)
{
	int var;

	if (strcmp(attr->attr.name, "baz") == 0)
		var = baz;
	else
		var = bar;
	return sprintf(buf, "%d\n", var);
}

static ssize_t b_store(struct kobject *kobj, struct kobj_attribute *attr,
		       const char *buf, size_t count)
{
	int var;

	sscanf(buf, "%du", &var);
	if (strcmp(attr->attr.name, "baz") == 0)
		baz = var;
	else
		bar = var;
	return count;
}

//定义若干属性文件
static struct kobj_attribute baz_attribute =
	__ATTR(baz, 0666, b_show, b_store);
static struct kobj_attribute bar_attribute =
	__ATTR(bar, 0666, b_show, b_store);

/*
 * Create a group of attributes so that we can create and destroy them all
 * at once.
 */
static struct attribute *attrs[] = {
	&baz_attribute.attr,
	&bar_attribute.attr,
	NULL,	/* need to NULL terminate the list of attributes */
};

/*
 * An unnamed attribute group will put all of the attributes directly in
 * the kobject directory.  If we specify a name, a subdirectory will be
 * created for the attributes with the directory being the name of the
 * attribute group.
 */
static struct attribute_group attr_group = {
//	.name = "attr",	//设置名字就多一个目录
	.attrs = attrs,
};

static const struct attribute_group *kobj_attr_groups[] = {
	&attr_group,
	NULL,
};

static int __init s_kobject_init(void)
{
	int retval=0;
	char __path[32] = {0};
	char *path = __path;
	char path_dst[32] = {0};

	
	/*向系统要一个input dev*/
	input_dev = input_allocate_device();
	if(!input_dev)
	{
		printk("input_allocate_device fail\n");
		return -EBUSY;
	}

	input_dev->name			= "Sample kobject";
	input_dev->phys			= "none";
	input_dev->id.bustype	= BUS_HOST;
	input_dev->evbit[0]		= BIT_MASK(EV_SYN);
	input_dev->id.vendor 	= 0x0000;
	input_dev->id.product 	= 0x0000;
	input_dev->id.version 	= 0x0000;
	input_dev->open			= NULL;
	input_dev->close		= NULL;

	//根据源码目录下的document/device.txt描述:指定属性文件要放到注册设备之前实现
	input_dev->dev.groups = kobj_attr_groups;
	//注册,顺便将dev_attr_groups属性文件给生成了。
	if(input_register_device(input_dev))
		printk("input_register_device fail\n");

	/*
	 * Create a simple kobject with the name of "kobject_example",
	 * located under /sys/kernel/
	 *
	 * As this is a simple directory, no uevent will be sent to
	 * userspace.  That is why this function should not be used for
	 * any type of dynamic kobjects, where the name and number are
	 * not known ahead of time.
	 */

	path = kobject_get_path(&input_dev->dev.kobj, GFP_KERNEL);
	sprintf(path_dst, "/sys%s", path);
	printk(KERN_ERR "kobject path:%s", path_dst);

	link_kobj = kobject_create_and_add("link", NULL);
	if (!link_kobj)
		goto put1;

	retval = sysfs_create_link(link_kobj, &input_dev->dev.kobj, "kobj_link");//link_kobj连接到example_kobj
	if (retval) {
		printk(KERN_ERR "sysfs_create_link error:");
		goto put0;
	}

	printk(KERN_ERR "hello kobj_drv: \n");
	return 0;
put1:
	kobject_put(link_kobj);
put0:

	return retval;
}

static void __exit s_kobject_exit(void)
{
	input_unregister_device(input_dev);
	input_free_device(input_dev);			// 卸载这个驱动,注意了卸载驱动时同时会卸载属性文件
	sysfs_remove_link(link_kobj, "kobj_link");
	kobject_put(link_kobj);
}

module_init(s_kobject_init);
module_exit(s_kobject_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("JinFa <2112201069@qq.com>");
