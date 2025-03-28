#include <linux/types.h>       // 基本类型定义，如 u32 等
#include <linux/kernel.h>      // 内核函数，如 printk()
#include <linux/module.h>  // 所有模块必须的头文件
#include <linux/init.h>   // __init 和 __exit 宏
#include <linux/fs.h>
#include <linux/uaccess.h> // copy_to_user / copy_from_user 函数


#define CHRDEVBASE_NAME "chrdevbase"    /* 设备名 */

static int major = 0;    /*主设备号（动态分配）*/

static char readbuf[100];   /* 读缓冲区 */
static char writebuf[100];  /* 写缓冲区 */
static char kerneldata[] = {"kernel data!"};  /* 内核要返回的数据 */


/*
 * @description		: 打开设备
 * @param - inode 	: 传递给驱动的inode
 * @param - filp 	: 设备文件，file结构体有个叫做private_data的成员变量
 * 					  一般在open的时候将private_data指向设备结构体。
 * @return 			: 0 成功;其他 失败
 */
static bool data_read = false;  // 添加标志变量,防止read死循环，cat：只读一次数据，自动退出 ✅
static int chrdevbase_open(struct inode *inode, struct file *file){
    data_read = false;  // 每次打开设备，重置读取标志
    printk("chrdevbase: device opened.\n");
    return 0;
}


/* 读取设备数据：内核 -> 用户 */
/*
 * @description		: 从设备读取数据 
 * @param - filp 	: 要打开的设备文件(文件描述符)
 * @param - buf 	: 返回给用户空间的数据缓冲区
 * @param - cnt 	: 要读取的数据长度
 * @param - offt 	: 相对于文件首地址的偏移
 * @return 			: 读取的字节数，如果为负值，表示读取失败
 */

static ssize_t chrdevbase_read(struct file *file, char __user *buf, size_t cnt, loff_t *offt){
    int ret;
    size_t datalen;
    /* 向用户空间发送数据 */

    if (data_read) {
        return 0;  // 已读过，返回 0，表示 EOF
    }

    /* 1. 把内核数据写入中转缓冲区 */
    memset(readbuf, 0, sizeof(readbuf));           // 清空缓冲区
    strcpy(readbuf, kerneldata);                   // 拷贝数据到缓存区

    /* 2. 安全：限制 cnt 不超过实际数据长度 */
    datalen = strlen(readbuf);
    if (cnt > datalen){
        cnt = datalen;
    }
        
    // 把数据从内核空间拷贝到用户空间
    ret = copy_to_user(buf, readbuf, cnt);
    if(ret == 0){
        printk("chrdevbase: read %zu bytes to user.\n", cnt);
        data_read = true;  // 第一次读完就标记为“读过了”
        return cnt;
    }else {
        printk("chrdevbase: failed to read to user.\n");
		return -EFAULT;
    }
}

/* 写入设备数据：用户 -> 内核 */
/*
 * @description		: 向设备写数据 
 * @param - filp 	: 设备文件，表示打开的文件描述符
 * @param - buf 	: 要写给设备写入的数据
 * @param - cnt 	: 要写入的数据长度
 * @param - offt 	: 相对于文件首地址的偏移
 * @return 			: 写入的字节数，如果为负值，表示写入失败
 */

static ssize_t chrdevbase_write(struct file *file, const char __user *buf, size_t cnt, loff_t *offt){
	int ret;

	// 从用户空间拷贝数据到内核缓冲区
	ret = copy_from_user(writebuf, buf, cnt);
	if (ret == 0) {
		printk("chrdevbase: received %zu bytes: %s\n", cnt, writebuf);
		return cnt;
	} else {
		printk("chrdevbase: failed to receive data.\n");
		return -EFAULT;
	}   
}

/*
 * @description		: 关闭/释放设备
 * @param - filp 	: 要关闭的设备文件(文件描述符)
 * @return 			: 0 成功;其他 失败
 */
static int chrdevbase_release(struct inode *inode, struct file *filp)
{
	printk("chrdevbase release！\r\n");
	return 0;
}


 /*
 * 设备操作函数结构体
 */
static struct file_operations chrdevbase_fops = {
	.owner = THIS_MODULE,              // 这是一个模块
	.open = chrdevbase_open,          // 打开
	.read = chrdevbase_read,          // 读
	.write = chrdevbase_write,        // 写
	.release = chrdevbase_release,    // 关闭
};

/*
 * @description	: 驱动入口函数 
 * @param 		: 无
 * @return 		: 0 成功;其他 失败
 */
static int __init chrdevbase_init(void){

    int ret;

    /* 注册字符设备驱动 */
    ret = register_chrdev(0, CHRDEVBASE_NAME, &chrdevbase_fops);
    if(ret < 0) {
        printk("chrdevbase: failed to register device.\n");
        return ret;
    }

    printk("chrdevbase: init successful. major = %d\n", ret);

    return 0;

}


/*
 * @description	: 驱动出口函数
 * @param 		: 无
 * @return 		: 无
 */
static void __exit chrdevbase_exit(void){
        /* 注销字符设备驱动 */
        unregister_chrdev(major, CHRDEVBASE_NAME);
        printk("chrdevbase: exit.\n");
}

/* 模块入口 */
module_init(chrdevbase_init);

/* 模块出口 */
module_exit(chrdevbase_exit);



/* 
 * LICENSE和作者信息
 */
MODULE_LICENSE("GPL");
MODULE_AUTHOR("SAKI");
MODULE_DESCRIPTION("QEMU下测试的字符设备驱动");











