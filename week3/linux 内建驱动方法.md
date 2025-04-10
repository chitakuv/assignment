# linux 内建驱动方法

### 第一步:集成你的新驱动到内核源码

#### 将驱动.c 放到合适的目录

一般将字符设备驱动放在 `drivers/char/` 下，因此创建一个新目录("mydriver")
```bash
cd linux-6.6.84
mkdir -p drivers/char/mydriver
```
然后**复制**驱动到该目录
```bash
cp /path/to/your/mydriver.c drivers/char/mydriver/
```

#### 编写 Kconfig 文件与 Makefile 文件

######首先需要在**mydriver目录创建一个 `Kconfig` 文件** 
用于 menuconfig(图形化菜单界面) 选择 编译选项
```makefile
menu "My Character Device Driver"  #创建一个文件夹

config MYCHARDEV
    tristate "My Custom Character Device Driver"   #显示的名字
    default m   #默认选择模块化驱动
    help
      Enable support for my custom character device driver.
      'm' will build it as a loadable module,      #模块化驱动
      'y' will build it directly into the kernel.   #写进内核的驱动

endmenu
```
用于内核配置

- 这里选择 `tristate`，默认 `y` 表示内建。如果你改为 `m`（模块），内核启动后需手动加载驱动，但这里我们用**内建方式。**

**然后还需要再在上一级目录 `drivers/char/Kconfig` 文件中，添加下面的内容**

```makefile
source "drivers/char/mydriver/Kconfig"  
```
这样 在 `make menuconfig` 中，drivers/char/Kconfig 可以链接到 新驱动的配置选项(mydriver/Kconfig)

##### 修改Makefile

在 `drivers/char/` 目录下的 Makefile（或你新创建目录的 Makefile）中，添加编译规则。例如，在 `drivers/char/Makefile` 里增加：
```makefile
obj-$(CONFIG_MYCHARDEV) += mydriver/
```
或者直接写：
```makefile
obj-y += mydriver/chrdevbase.o
```
第二种方法就不需要在进行下一步了

**下一步**
在 `drivers/char/mydriver` 下创建一个 `Makefile`, 内容如下：
```makefile
obj-y += chrdevbase.o
```
这样当你编译内核时，这个驱动就会被包含进来。

### 第二步:配置内核


回到 内核源码根目录,,如 linux-6.6.9/ ，执行以下语句：
```bash
make mrproper  #清理旧配置
make deconfig  #生成默认配置
```

然后运行内核配置界面：
```bash
make menuconfig
```

在菜单中找到 **Device Drivers → Character devices**（或者你新加的选项所在的位置），选中 “My Character Device Driver”。确保你选择为内建（Y）而不是模块（M）。

保存退出后，确认 `.config` 文件中有类似 `CONFIG_MYDRIVER2=y` 的配置行

### 第三步:编译内核
在内核源码根目录下执行
```bash
make -j$(nproc)
```
这将根据配置编译内核，包括你内建的驱动。编译结束后，生成的内核映像（例如 `arch/arm64/boot/Image`）中将包含你的新驱动。

### 第四步:生成根文件系统（rootfs）
准备好 BusyBox、启动脚本以及必要的设备文件。确保你的 rootfs 目录结构完备，例如：
```csharp
rootfs/
├── bin/      # BusyBox 安装后的二进制文件
├── etc/      # 配置文件
├── init      # 启动脚本（必须为可执行文件）
├── dev/      # 创建设备节点：/dev/console, /dev/null, /dev/tty 等
├── proc/     # 挂载点，启动时将挂载 proc
├── sys/      # 挂载点，启动时将挂载 sysfs
...
```
利用 BusyBox 的 `make install CONFIG_PREFIX=/path/to/rootfs` 命令将 BusyBox 安装进去，并确保在 `init` 脚本中挂载 `/dev`, `/proc`, `/sys`。同时确保 `/dev` 中已经存在 `/dev/console` 等关键设备（或通过挂载 devtmpfs 自动生成）。

**注意：因为 驱动已经打包进内核，roofts中不需要有 驱动文件了！**

### 第五步:打包根文件系统
在你的 rootfs 目录中生成 initramfs：
```bash
cd /path/to/rootfs
find . | cpio -o --format=newc | gzip > ../rootfs.cpio.gz
cd ..
```

### 第六步:启动测试
使用 QEMU 启动系统，新内核自动加载内建驱动，无需手动 insmod：
```bash
qemu-system-aarch64   -M virt   -cpu cortex-a53   -nographic   -m 512M   -kernel ~/Dev/qemu/linux-6.6.84/arch/arm64/boot/Image   -initrd rootfs.cpio.gz   -append "console=ttyAMA0 rdinit=/init"
```

### 第七步:验证
- 登录 QEMU 系统后，通过 `dmesg | grep "mydriver"` 检查内核日志中是否有你驱动的打印信息，例如：
```csharp
chrdevbase: init successful. major = 237
```

可以看到不需要 insmod，设备已经注册

但是还是需要 创建设备节点