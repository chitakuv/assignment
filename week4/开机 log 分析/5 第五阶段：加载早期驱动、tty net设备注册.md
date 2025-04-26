### **1. 系统安全与块/文件系统驱动初始化（812 – 824 行） ​**

**KVM 与受信任密钥环**
	- HYP 模式检查、系统信任密钥环初始化、黑名单密钥注册（812–814 行）​​
**内存缓存子系统**
	- workingset 缓存参数、zbud 压缩页面缓存加载（815–816 行）​​
**压缩与文件系统驱动**
	- squashfs 驱动注册、FUSE 初始化（817–818 行）​​
**完整性与密钥机制**
	- 平台/机器密钥环、非对称密钥注册（819–822 行）​​
**块层与调度器**
	- SCSI 通用块驱动、mq-deadline I/O 调度器注册（823–824 行）

##### 检测虚拟化支持（812 行）

- 日志：kvm [1]: HYP mode not available ​
- 做了什么：ARM64 内核尝试初始化 KVM 虚拟化模块时，检查当前 CPU 是否已切换到 Hypervisor 特权级（HYP/EL2）。如果未切换，就跳过 KVM 初始化并打印此信息。

**关键调用链：**
```
start_kernel()   
└─ do_basic_setup()   
	└─ arm64_hyp_init()         // 判断 HYP 是否可用           
		└─ kvm_arm_mode_init()  // KVM ARM 模式初始化               
			└─ pr_info()        // 打印日志 
```
术语释义：

- HYP 模式：ARM 体系结构中的 Hypervisor 特权级（PL2），提供硬件虚拟化支持。

##### 系统受信任密钥环（813–814 行）

**日志：**
- Initialise system trusted keyrings ​
- Key type blacklist registered ​

**做了什么：**
- 内核调用 system_trusted_keyring_init()（certs/system_keyring.c），创建用于验证内核模块和文件系统完整性的“系统密钥环”。
- 注册“黑名单密钥”类型，用来拒绝特定已废弃或被撤销的公钥。

**调用链：**
```
start_kernel()   
└─ do_initcalls()   
	└─ subsys_initcall(system_trusted_keyring_init)           
		├─ keyring_alloc()           // 分配密钥环对象           
		└─ register_key_type("blacklist") // 注册黑名单密钥类型 
```
**术语释义：**
- Keyring：内核中的密钥管理容器，用于存储公钥、证书和吊销列表。
- 黑名单密钥：一种特殊类型的密钥，当加载模块或文件签名匹配时，阻止其安装或加载。

##### **内存工作集与压缩缓存（815–816 行）**

**日志：**

- **workingset: **timestamp_bits=40 max_order=20 bucket_order=0 ​
- **zbud: **loaded ​

**做了什么：**

- workingset：在 mm/workingset.c 中初始化工作集回收算法的参数，决定如何跟踪最近使用的内存页，帮助优化页回收。
- zbud：在 mm/zbud.c 中注册 zbud 压缩后端，支持将两页压缩存入一页，从而节省 RAM。

**调用链：**
```
start_kernel()   
	└─ do_initcalls()       
		├─ core_initcall(workingset_init)   
		└─ core_initcall(zbud_init) 
```
**术语释义：**
- 工作集（workingset）：衡量应用活跃页集大小的算法，用于内存管理决策。
- zbud：双页内存压缩方案，常用于嵌入式或内存受限场景。

- 压缩文件系统与 FUSE 驱动（817–818 行）

**日志：**

- squashfs: version 4.0 (2009/01/31) Phillip Lougher ​
- fuse: init (API version 7.39) ​

**做了什么：**

- SquashFS：在 fs/squashfs/module.c 中注册，只读压缩文件系统，用于内核内置根文件系统（initramfs）。
- FUSE：在 fs/fuse/inode.c 中注册用户态文件系统框架，允许应用在用户空间实现文件系统。

调用链：
```
start_kernel()   
└─ do_initcalls()       
	├─ fs_initcall(squashfs_init)   
	└─ fs_initcall(fuse_init) 
```
**术语释义：**

- SquashFS：一种高压缩比的只读文件系统。
- FUSE：Filesystem in Userspace，将文件系统操作代理到用户态进程。

##### 完整性度量与公钥体系（819–822 行）

**日志：**

- integrity: Platform Keyring initialized ​
- integrity: Machine keyring initialized ​
- Key type asymmetric registered ​
- Asymmetric key parser 'x509' registered ​

**做了什么：**

- IMA/Integrity 子系统分别初始化“平台密钥环”和“机器密钥环”，用于存储信任根和运行时度量数据。
- 注册 asymmetric 密钥类型及其 X.509 解析器，支持加载和验证数字证书。

**调用链：**
```
start_kernel()   
└─ do_initcalls()       
	├─ integrity_init()            // 平台 & 机器密钥环       
	└─ core_initcall(asymmetric_keys_init)           
		├─ register_key_type("asymmetric")   
		└─ register_key_parser("x509") 
```
**术语释义：**

- IMA/Integrity：完整性度量架构，用于在运行时验证文件和模块的完整性。
- X.509：公钥证书标准，用于签名与身份验证。

- 块层与 I/O 调度器（823–824 行）

**日志：**

- Block layer SCSI generic (bsg) driver version 0.4 loaded (major 242) ​
- io scheduler mq-deadline registered ​

**做了什么：**

- BSG 驱动：在 drivers/scsi/sg.c 中注册，为用户空间提供直接发送 SCSI 命令的通道。
- mq-deadline：在 block/mq-deadline-iosched.c 中注册多队列 Deadline 调度器，优化延迟敏感型 I/O。

**调用链：**
```
start_kernel()   
└─ do_initcalls()       
	├─ device_initcall(bsg_init)   
	└─ core_initcall(mq_deadline_init) 
```
**术语释义：**

- BSG（Block SCSI Generic）：允许用户空间直接与 SCSI 设备通信。
- Deadline 调度器：保证 I/O 请求在一定时间内得到服务，减少延迟。

### **2. PCIe 热插拔与 PME 信号设置（825 – 889 行） ​**

###### **使能 PME（Power Management Event）并绑定中断（825–833 行）**

1. 探测到根端口  
    当内核扫描到每个 PCIe 根端口（0000:00:15.2、00:15.3……），它会在底层驱动 pcie_portdrv_probe() 中调用  
    pci_enable_pme(dev);  
      
    使能该端口产生 PME 信号。
2. 注册中断线  
    pci_enable_pme() 会请求并绑定一个 IRQ Line，然后打印：  
    pcieport 0000:00:15.2: PME: Signaling with IRQ 14  
      
    表示该端口的 PME 事件将通过中断号 14 通知内核。随后依次为端口 15.3 用 IRQ 15、15.4 用 IRQ 16，以此类推。
3. 调用链  
```
start_kernel()  
  └─ pci_init()  
	  └─ pci_scan_root_bus()  
		  └─ pcie_portdrv_probe(dev)  
			  └─ pci_enable_pme(dev)  
				  └─ request_irq() → pr_info("PME: Signaling with IRQ %d")
```
**术语释义**

- PME：Power Management Event，用于通知系统“电源状态变化”或“热插拔请求”。
- IRQ：中断请求线，数字标识硬件中断源。

##### **槽位（Slot）能力注册与监控（825–888 行）**

在使能 PME 之后，端口驱动会调用 热插拔子系统（pciehp）为每个槽位（Slot）建立监控，打印详细的槽位特性位图：

pcieport 0000:00:15.2: pciehp: Slot #162 AttnBtn+ PwrCtrl+ MRL- AttnInd- PwrInd- HotPlug+ Surprise- Interlock- NoCompl+ IbPresDis- LLActRep+

**哪些功能？**

- AttnBtn+：Attention Button（手动插拔按键）支持
- PwrCtrl+：Power Control（电源控制）支持
- MRL-：No MRL Sensor（机械保持释放传感器）
- AttnInd- / PwrInd-：无状态指示灯
- HotPlug+：支持热插拔
- Surprise-：不支持“意外移除”
- Interlock-：无机械互锁
- NoCompl+：无完整性检测协议
- IbPresDis-：无插入检测
- LLActRep+：Link Active Reporting（链路状态报告）

**源码调用  **
    在 drivers/pci/pcie/portdrv.c 中，pcie_portdrv_probe() 会调用：  
    `pciehp_init_slot(dev, slot_number);`
	该函数读取 PCIe 槽位能力寄存器（Slot Capabilities Register）和 ACPI 方法，生成上述带“+”/“-”的特性列表，并通过 pr_info() 打印。
**槽位号如何确定？  **
    内核根据设备号（如 15.2 对应槽位 162，通过 (bus – 1) * 32 + port 计算）来给出唯一的“Slot #”。

##### **标准热插拔控制器驱动加载（889 行）**

**shpchp: **Standard Hot Plug PCI Controller Driver version: 0.4

**做了什么？**
- 内核加载并初始化 Standard Hot Plug Controller 驱动（shpchp），这是 ACPI 定义的通用 PCI 热插拔管理器，负责：
	- 监听 PME 事件
	- 控制槽位电源（PwrCtrl）
	- 响应 Attention Button
	- 管理插拔后的资源重分配
**调用链  **
```
start_kernel()  
└─ do_initcalls()  
	└─ subsys_initcall(shpchp_init)  
		└─ register_driver(&shpchp_driver)  
			└─ pr_info("Standard Hot Plug PCI Controller Driver version: %s")
```
**术语释义**

- shpchp：Standard Hot-Plug PCI Controller，根据 ACPI Hot Plug Specification 实现的通用热插拔驱动。
- Slot：物理插槽，PCIe 卡实际插入的地方，由根端口（Root Port）管理。

### **3. 外设与控制台驱动注册（890 – 915 行） ​**

#####  串口与通用字符设备驱动
Serial: 8250/16550 driver, 32 ports, IRQ sharing enabled（890）

**含义：**在 8250_init()（drivers/tty/serial/8250/8250.c）中注册标准 PC UART 驱动，支持 32 个串口，并启用中断共享。

**调用链：**  
```
start_kernel()  
└─ do_initcalls()  
	└─ tty_driver_init()  
		└─ uart_register_driver(&serial8250_reg)
```

**术语：**IRQ 共享，允许多个设备共用同一中断线。

##### msm_serial: driver initialized（891）

- **含义：**初始化 Qualcomm MSM SoC 上特有的串口控制器驱动，来自 drivers/tty/serial/msm/。

##### SuperH (H)SCI(F) driver initialized（892）

- **含义：**注册 SuperH 架构上的 SCI/UART 驱动（HSCI/FSCI），来自 drivers/tty/serial/sh-sci/。

##### loop: module loaded（893）

**含义：**loop_init()（drivers/block/loop.c）注册环回设备驱动，允许把文件当块设备挂载。
**术语：**loop 设备，将普通文件映射为块设备，用于测试或容器环境。

##### **图形接口与虚拟网络设备**

 ACPI: bus type drm_connector registered（894）

**含义：**drm_connector_init() 在 drivers/gpu/drm/drm_connector.c 中注册 DRM Connector 类型，用于显示输出和热插拔检测。
**术语：**DRM Connector，表示显示器连接器（如 HDMI、DP）。

##### tun: Universal TUN/TAP device driver, 1.6（895）

**含义：**tun_init()（drivers/net/tun.c）注册 TUN/TAP 虚拟网络接口，版本 1.6。用于用户空间网络隧道。

##### PPP generic driver version 2.4.2（896）

**含义：**ppp_init()（drivers/net/ppp/）注册点对点协议栈，版本 2.4.2。

##### **USB 主机控制器初始化**

 ehci-pci 0000:01:02.0: EHCI Host Controller（897）

**含义：**PCI 子系统探测到 EHCI (USB 2.0) 控制器，调用 ehci_pci_probe()（drivers/usb/host/ehci-pci.c）。

##### ehci-pci 0000:01:02.0: new USB bus registered, assigned bus number 1（898）
##### ehci-pci 0000:01:02.0: irq 44, io mem 0x3d408000（899）
**作用：**
	分别分配 USB 总线号 1、绑定中断号 44、映射 MMIO 资源。

##### uhci_hcd 0000:01:00.0: init 0000:01:00.0 fail, -16（900）
##### uhci_hcd: probe of 0000:01:00.0 failed with error -16（901）
**含义：**UHCI (USB 1.x) 驱动在 uhci_pci_probe() 中因资源冲突或缺失返回 -EBUSY，无法初始化。
**术语：**-16 对应 -EBUSY，表示设备繁忙或资源已被占用。

##### **输入设备与实时时钟**

 mousedev: PS/2 mouse device common for all mice（902）

**含义：**mousedev_init()（drivers/input/mouse/mousedev.c）注册通用 PS/2 鼠标字符设备 /dev/input/mouseX。

##### rtc-efi rtc-efi.0: registered as rtc0（903）
##### rtc-efi rtc-efi.0: setting system clock to 2025-04-23T02:39:38 UTC …（904）

**含义：**rtc_efi_probe()（drivers/rtc/rtc-efi.c）注册 EFI 实时时钟，为系统设置当前时间戳。

##### **I²C、Device-Mapper 与 LED 触发**

 i2c_dev: i2c /dev entries driver（905）

**含义：**i2c_dev_init()（drivers/i2c/i2c-dev.c）在 /dev 下注册原始 I²C 设备接口，允许用户空间访问 I²C 总线。

##### device-mapper: core: CONFIG_IMA_DISABLE_HTABLE is disabled…（906）
##### device-mapper: uevent: version 1.0.3（907）
##### device-mapper: ioctl: 4.48.0-ioctl … initialised（908）

**含义：**dm_init()（drivers/md/dm-mod.c）初始化 Device-Mapper 框架，打印配置和版本信息，支持 LVM、加密等功能。

##### ledtrig-cpu: registered to indicate activity on CPUs（909）

**含义：**ledtrig_cpu_init()（drivers/leds/trigger/ledtrig-cpu.c）注册 LED 触发器，当 CPU 活跃时闪烁。

##### **简易 Framebuffer 与控制台切换**

 Initialized simpledrm 1.0.0 … for simple-framebuffer.0（910）

**含义：**simpledrm_init()（drivers/gpu/drm/simple/simple-framebuffer.c）注册基于设备树或 ACPI 描述的简单 Framebuffer DRM 驱动。

##### Console: switching to colour frame buffer device 128x48（911）
##### simple-framebuffer simple-framebuffer.0: [drm] fb0: simpledrmdrmfb frame buffer device（912）

**做了什么：**早期虚拟的 printk 控制台切换到图形 Framebuffer 控制台，分辨率 128×48。

##### **网络监控与 IPv6 注册**

 drop_monitor: Initializing network drop monitor service（913）

**含义：**drop_monitor_init()（net/drops/drops.c）启动网络丢包监控子系统，可跟踪被防火墙或路由丢弃的包。

##### NET: Registered PF_INET6 protocol family（914）
**含义：**inet6_init()（net/ipv6/af_inet6.c）注册 IPv6 协议族，支持原生 IPv6 网络。

##### **EHCI USB 2.0 启动完成**

 ehci-pci 0000:01:02.0: USB 2.0 started, EHCI 1.00（915）

**含义：**在 ehci_run() 中正式启动 USB 2.0 控制器，打印 EHCI 版本号 1.00，表示总线已可用。

### 4. USB1/EHCI 枚举 & initrd 释放 & 网络核心功能（916 – 975 行） ​

##### **EHCI Host 控制器及根集线器枚举（916–922 行）**

**驱动子系统：**drivers/usb/host/ehci-pci.c → EHCI PCIe 主机控制器；核心函数 ehci_init_controller() 调用 usb_add_hcd()，再由 USB 核心（usb/core/hub.c）遍历根端口。

**关键日志：**
- **usb usb1:** New USB device found…：发现根集线器设备，idVendor=1d6b（USB 根集线器厂商 ID）、idProduct=0002（产品 ID）、bcdDevice=6.08（EHCI 版本号）​​
- **usb usb1: **Product/Manufacturer/SerialNumber：从设备描述符读取的产品名、驱动版本与序列号​​
- **hub 1-0:1.0: **USB hub found：根集线器驱动在端口 1-0:1.0 上初始化完成
- **hub 1-0:1.0: **6 ports detected：该根集线器支持 6 个下游端口​​

##### **释放 initrd（923 行）**

**函数路径：**init/initramfs.c → free_initrd()
**日志含义：**

- **Freeing initrd memory:** 68592K：内核已将 initramfs（临时根文件系统）解包到 tmpfs，此处释放原始 initrd 镜像占用的物理内存，约 68 MB​​

#####  网络核心功能注册（924–928 行）

初始化完 USB 后，开始注册网络协议与统计功能：

1. **IPv6 段路由**
	- Segment Routing with IPv6：启用 IPv6 段路由（SRv6）功能，由 net/ipv6/seg6.c 中的 seg6_init() 注册​​
2. **原位运维（IOAM）**
	- In-situ OAM (IOAM) with IPv6：启用在 IPv6 数据报中嵌入运维信息，可用于网络可视化与故障定位，由 net/ipv6/ioam.c 中的 ioam_init() 注册​​
3. **PF_PACKET 套接字**
	- NET: Registered PF_PACKET protocol family：底层驱动网络原始套接字接口，供抓包等用途，由 net/packet/af_packet.c 中的 packet_init() 执行​​
4. **DNS 解析器密钥类型**
	- Key type dns_resolver registered：注册用于用户空间调用内核 DNS 解析服务的密钥类型，由 security/keys/dns_resolver.c 中的 dns_resolver_init() 实现​​
5. **任务统计**
	- registered taskstats version 1：启用 taskstats 接口，用于统计进程/线程在网络、磁盘等子系统的延迟和字节数，由 kernel/taskstats.c 中的 taskstats_init() 注册​​

##### **编译内置 X.509 根证书加载（929–939 行）**
**子系统：**Security → Common capability certificates

**调用链：**内核启动时，init/init_security.c 中的 init_security() 调用 load_builtin_certificates()。

**日志含义：**

- **Loading compiled-in X.509 certificates：**开始加载编译进内核的证书列表
- **随后多条 Loaded X.509 cert '…'：**逐个加载 Canonical、Microsoft、VMware 等签名证书，用于模块签名验证和受信任引导​​

##### **文件系统加密与密钥类型注册（942–944 行）**

- **注册流程：**位于 fs/crypto/ 子目录中，各自的 __init 函数在安全子系统初始化阶段被调用。
**日志：**
- **Key type .fscrypt registered：**注册 ext4/UBIFS 等使用的 fscrypt 加密密钥类型
- **Key type fscrypt-provisioning registered：**注册用于生成 fscrypt 密钥的预配类型
- **Key type encrypted registered：**注册通用加密密钥类型  
    （均来自 fs/crypto/fscrypt.c、fs/crypto/provision.c、fs/crypto/encrypted-keys.c）​​

##### **AppArmor 哈希策略与 Integrity 子系统加载/吊销（945–975 行）**

- **AppArmor**
	- AppArmor: AppArmor sha256 policy hashing enabled：启用对 AppArmor 配置文件的 SHA256 哈希校验，由 security/apparmor/hash.c 中的初始化函数输出​​
- **Integrity (IMA/ EVM)**
	- 多条 integrity: Loading X.509 certificate: UEFI:db & Loaded X.509 cert '…'：从 UEFI db 和 dbx 加载供应商证书和吊销列表，用于 IMA 完整性度量，由 security/integrity/ima/ima_main.c 和 evm/evm_main.c 中的 integrity_init() 执行​​
	- integrity: Revoking X.509 certificate: UEFI:dbx、blacklist: Revoked X.509 cert '…'：根据 dbx 吊销列表撤销证书信任
	- ima: secureboot mode disabled：未检测到 Secure Boot 激活，IMA 进入被动模式
	- ima: No TPM chip found, activating TPM-bypass!：找不到可信平台模块，启用绕过模式
	- ima: Allocated hash algorithm: sha256、ima: No architecture policies found：初始化 IMA 哈希算法与策略
	- evm: Initialising EVM extended attributes: 后续多行 evm: security.*：设置 EVM（扩展验证模块）支持的安全属性列表，包括 SELinux、SMACK、AppArmor、IMA 等
- 后续内存与启动
	- clk: Disabling unused clocks：关闭多余时钟源
	- Freeing unused kernel memory: 12224K：释放内核初始化阶段剩余内存
	- Checked W+X mappings: passed, no W+X pages found：内核自检，确认无可执行且可写页面
	- 切换到用户态：Run /init as init process → 执行真正的 init 程序，环境变量包括 HOME=/、TERM=linux、BOOT_IMAGE=/vmlinuz-6.8.0-55-generic​​

**小结：** 916–960 行涵盖从 USB 根集线器枚举、initrd 释放，到网络协议注册，再到安全子系统（证书、加密密钥、AppArmor、IMA/EVM）的完整初始化流程，各功能模块按 initcall 顺序依次完成，最终将控制权交给用户态的 /init。

### **5. 切换到用户态（976 – 985行） ​**

##### **时钟与内存清理（976–977 行） ​**

- 禁用未使用的时钟  
```
clk: Disabling unused clocks
```
- **源码调用：** clk_disable_unused() 在 init/main.c 的 late init 阶段被调用，遍历所有由 Common Clock Framework 注册的时钟，并关闭没有被驱动器使用的那些，以节省功耗。
- 释放内核初始化内存  
```
Freeing unused kernel memory: 12224K
```

**调用链：** free_initmem()（init/init_elf.c）释放 .init.text、.init.data 等仅在引导时使用的内存区域，回收给通用伙伴系统。

##### **可执行权限检查（978 行） ​**

Checked W+X mappings: passed, no W+X pages found

**源码位置：**check_memory() 在 init/main.c，扫描所有内核映射，确保不存在同时设置了“写（W）”和“可执行（X）”权限的页面，以防止可写自执行（W^X）安全风险。

##### **切换到用户态 init（979–985 行） ​**
**运行 /init 进程  **
```
Run /init as init process  
with arguments:  
/init  
with environment:  
HOME=/  
TERM=linux  
BOOT_IMAGE=/vmlinuz-6.8.0-55-generic
```
**源码调用：**kernel_init()（init/main.c）完成内核所有初始化后，调用 run_init_process("/init")，它最终执行 do_execve()，启动第一个用户空间进程。

**术语：**

- **init process：**PID 1，所有其他用户进程的祖先，通常由 systemd 或其他 init 系统替代。
- **环境变量 HOME、TERM、BOOT_IMAGE：**为用户空间过程提供基本运行环境与内核镜像路径信息。

### **6. USB3/xHCI 枚举、存储与网络驱动探测（986 – 1030 行） ​**

##### **xHCI 主机控制器枚举与根集线器探测（991–1009 行） ​**

**探测 xHCI 控制器硬件**

**日志  **
    xhci_hcd 0000:0a:00.0: hcc params 0x0388f081 hci version 0x120 quirks 0x0000000200000010  
    xhci_hcd 0000:0a:00.0: xHCI Host Controller  
    xhci_hcd 0000:0a:00.0: new USB bus registered, assigned bus number 3  
    xhci_hcd 0000:0a:00.0: Host supports USB 3.2 Enhanced SuperSpeed

**做了什么**
- 驱动 xhci_pci_probe()（drivers/usb/host/xhci-pci.c）调用 xhci_init_controller()，读取控制器能力寄存器（HCCPARAMS、HCIVERSION）并在内核注册一个新的 USB 总线（bus 3）。

**调用链  **
```
pci_init()  
└─ pci_scan_root_bus()  
	└─ xhci_pci_probe(dev)  
		└─ xhci_init_controller(hcd)  
			└─ usb_add_hcd(hcd) → pr_info("xHCI Host Controller") 
```

**术语**

- HCCPARAMS：Host Controller Capability 参数，描述硬件支持特性。
- Enhanced SuperSpeed：USB 3.2 标准下最高 20 Gbps 的传输模式。

**根集线器（Root Hub）发现**

**日志  **
    usb usb2: New USB device found… idVendor=1d6b, idProduct=0002, bcdDevice=6.08  
    usb usb2: Product: xHCI Host Controller   
    hub 2-0:1.0: USB hub found   
    hub 2-0:1.0: 4 ports detected 
**做了什么**
- USB 核心（usb/core/hub.c）调用 usb_hub_probe() 注册虚拟根集线器设备（vendor 1d6b 通用根 hub）、并探测其下游端口数量（4）。

**术语**

- **Root Hub：**内置在主控中的虚拟集线器，为上层提供下游物理端口。

**第二组 xHCI 枚举与 LPM 禁用**

**日志  **
    usb usb3: We don't know the algorithms for LPM for this host, disabling LPM.   
    usb usb3: New USB device found…   
    hub 3-0:1.0: USB hub found   
    hub 3-0:1.0: 4 ports detected 
**做了什么**

- 驱动无法确定链路低功耗管理（LPM）策略，因而在 xhci_hcd_link_init() 中调用 xhci_disable_lpm()。
- 随后又注册了第二个 USB 总线（bus 3 对应超级速度模式），并同样探测到一个 4 端口的根 hub。

##### **AHCI/SCSI 主机适配器注册（1010–1030 行） ​**

1. **AHCI 驱动注册**
- 日志示例  
	scsi host0: ahci   
	scsi host1: ahci   
	…   
	scsi host14: ahci 
- 做了什么
	- PCI 子系统探测到多个 SATA 控制器后，调用 ahci_pci_probe()（drivers/ata/ahci/ahci.c）。
	- AHCI 驱动为每个控制器实例分配一个 SCSI host（host0–host14），并通过 scsi_add_host() 向 SCSI 核心注册，随后调用 scsi_scan_host() 枚举连接的磁盘。
- 术语
	- AHCI：Advanced Host Controller Interface，SATA 控制器规范。
	- SCSI host：操作系统中表示一个 SCSI 总线的抽象，负责管理底层设备。
- NVMe 控制器与分区探测（1018, 1021–1023 行） ​

2. **NVMe 控制器注册**
- 日志  
    nvme nvme0: pci function 0000:12:00.0   
    nvme nvme0: 2/0/0 default/read/poll queues 
- 做了什么
	- 驱动 nvme_init_controller()（drivers/nvme/host/nvme-pci.c）在 PCI 探测回调中调用，创建 nvme0 控制器实例，并根据设备能力（I/O 队列对数）初始化请求队列。
- 术语
	- IO Queues：NVMe 框架中并行处理 I/O 的专用队列；default/read/poll 三种队列类型用于不同的请求场景。

2. **NVMe 命名空间与分区**

- 日志  
    nvme0n1: p1 p2 p3 
- 做了什么
	- NVMe 驱动通过 nvme_probe_namespaces() 枚举命名空间，并为每个命名空间注册块设备（nvme0n1），再读取分区表，发现 p1、p2、p3 三个分区。

##### **e1000e 网络驱动加载与中断节流（1015–1020 行） ​**

1. 驱动加载

- 日志  
    e1000e: Intel(R) PRO/1000 Network Driver   
    e1000e: Copyright(c) … 
- 做了什么
	- PCI 核心调用 e1000e_probe()（drivers/net/ethernet/intel/e1000e），注册网络设备 eth0 等，并设置 MAC 与硬件寄存器。

2. 动态中断节流
- 日志  
    e1000e 0000:02:00.0: Interrupt Throttling Rate … dynamic conservative mode 
- 做了什么
	- 驱动根据当前网络负载调用 e1000e_configure_itr()，为接收和发送中断设置动态保守（conservative）节流参数，以平衡延迟与 CPU 占用。
- 术语
	- Interrupt Throttling Rate (ITR)：限制中断频率，减少 CPU 上下文切换开销。
	- Dynamic Conservative Mode：根据中断负载自动调低节流阈值，优先保证稳定性。

3. Early userspace (systemd) 监听套接字（1199 – 1208 行） ​

4. multipathd 控制套接字
- 日志（1199 行）  
    [    1.998224] systemd[1]: Listening on multipathd.socket - multipathd control socket.
- 功能  
    multipathd 是多路径存储守护进程，这个 UNIX domain 套接字（默认路径 /run/multipathd/control）用于客户端工具（如 multipath CLI）与守护进程之间的命令与状态交互。
- 调用链  
```
manager_load_unit()   
└─ unit_start()   
	└─ socket_unit_start()   
		└─ socket_unit_open()   
			└─ create_listen_socket() → socket() → bind() → listen()   
				└─ log_unit_listening()  // 输出 “Listening on multipathd.socket”
```

5. Syslog 套接字

- 日志（1200 行）  
    [    1.999321] systemd[1]: Listening on syslog.socket - Syslog Socket.
- 功能  
    这是 syslog API 的 UNIX domain 套接字（通常 /run/systemd/journal/dev-log 或 /dev/log），用户态程序通过它把日志消息发给 journald。
- 术语
	- UNIX domain socket：跨进程在同一主机上通信的文件系统路径套接字。
	- Syslog：标准日志协议，允许不同进程统一发送日志。

6. fsckd 通信套接字

- 日志（1201 行）  
    [    2.000409] systemd[1]: Listening on systemd-fsckd.socket - fsck to fsckd communication Socket.
- 功能  
    当系统启动过程中需要文件系统检查（fsck）时，systemd-fsckd.socket 提供内核 fsck 子系统与 systemd-fsckd.service 之间的控制通道。

7. initctl 兼容套接字

- 日志（1202 行）  
    [    2.000986] systemd[1]: Listening on systemd-initctl.socket - initctl Compatibility Named Pipe.
- 功能  
    兼容 SysV initctl 接口（/run/systemd/initctl/fifo），允许老式脚本或工具使用 initctl 命令与 systemd 交互。
- 术语
	- Named Pipe (FIFO)：一个特殊文件，用于进程间半双工通信。

8. Journal 日志套接字

- 日志  
    [    2.001395] systemd[1]: Listening on systemd-journald-dev-log.socket - Journal Socket (/dev/log).  
    [    2.001861] systemd[1]: Listening on systemd-journald.socket - Journal Socket.
- 功能
	- systemd-journald-dev-log.socket 对应 /dev/log，处理传统 syslog 消息。
	- systemd-journald.socket 则是 journald 的专用 API 套接字，用于本地服务读取二进制日志。

9. networkd Netlink 套接字

- 日志（1205 行）  
    [    2.003518] systemd[1]: Listening on systemd-networkd.socket - Network Service Netlink Socket.
- 功能  
    systemd-networkd 通过这个 Netlink 套接字（AF_NETLINK, NETLINK_ROUTE）接收内核网络状态变化事件、路由与地址配置更新。

10. 条件未满足的 PCREXTEND 套接字

- 日志（1206 行）  
    [    2.004112] systemd[1]: systemd-pcrextend.socket - skipped due to an unmet condition check (ConditionSecurity=measured-uki).
- 含义  
    pcrextend 为 UEFI 平台测量扩展服务提供套接字，但因启动时未满足 ConditionSecurity=measured-uki（未启用度量的 UKI 模式），所以跳过打开。

11. udev 控制与内核事件套接字

- 日志  
    [    2.004563] systemd[1]: Listening on systemd-udevd-control.socket - udev Control Socket.  
    [    2.004968] systemd[1]: Listening on systemd-udevd-kernel.socket - udev Kernel Socket.
- 功能
	- udevd-control.socket：向 systemd-udevd 守护进程发送控制命令（如触发事件、动态规则加载）。
	- udevd-kernel.socket：接收内核发来的 uevent 通知（设备热插拔、属性更改），路径 /run/udev/kernel（netlink）。