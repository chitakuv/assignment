####  **1. [ 0.000000] Booting Linux on physical CPU 0x0000000000 [0x610f0000]**

- **源文件：**`arch/arm64/kernel/head.S`
- **函数：**
	ENTRY(secondary_startup)      # ARM 架构
	ENTRY(startup_32 / startup_64)      # x86 架构

- **解释：**

	- 最开始的汇编入口，内核启动入口
	- 硬件切换到 EL1（内核权限），确定由 CPU0 进入
	- MIDR = Main ID Register，读 CPU 标识

#### **2. [ 0.000000] Linux version 6.8.0-55-generic ...**

- **调用过程：**
```
- start_kernel()    [init/main.c] 
	└──pr_notice("%s", linux_banner);    [include/linux/printk.h] 
		#define pr_notice(fmt, ...) \
			printk(KERN_NOTICE pr_fmt(fmt), ##__VA_ARGS__)
	└──const char linux_banner[] =
		"Linux version " UTS_RELEASE " (" LINUX_COMPILE_BY "@"
		LINUX_COMPILE_HOST ") (" LINUX_COMPILER ") " UTS_VERSION "\n";     [init/version.c ]
```


- **解释：**

- linux_banner 是一个全局字符串变量，它存着内核版本信息

#### **3. [ 0.000000] KASLR disabled due to lack of seed**

- 调用过程：
```
start_kernel()                    // init/main.c
	└── setup_arch()                 // arch/arm64/kernel/setup.c
	    └── kaslr_init()      // arch/arm64/kernel/kaslr.c
```


- **解释：** KASLR（Kernel Address Space Layout Randomization）是内核的一种安全机制：需要 EFI 或 ACPI 提供随机种子。它的作用是：在每次开机时随机内核的加载地址，让攻击者猜不到关键数据结构位置，从而增强内核安全。
- 这是 ARM64 架构下的 KASLR（地址空间随机化）判定逻辑
```c
#define MIN_KIMG_ALIGN  (2 << 20)   // 2MB  // 内核镜像在物理内存中最小对齐粒度，也是内核允许的最小随机化偏移。

if (kaslr_offset() < MIN_KIMG_ALIGN) {      # 如果 kaslr_offset < 2MB，说明压根没启用随机地址。

pr_warn("KASLR disabled due to lack of seed\n");

return;
```


- 本次启动缺少随机种子，因此关闭地址随机化

#### [ 0.000000] Machine model: linux,dummy-virt

- 源文件：arch/arm64/kernel/setup.c
- 函数：setup_arch() -> unflatten_device_tree()
- 解释：

- 读取 Device Tree（设备树）中 model 字段，确定模拟机型：linux,dummy-virt

### **4. [ 0.000000] efi: EFI v2.7 by EDK II**

- 调用过程：
```
start_kernel()   // init/main.c
	└── setup_arch() // arch/arm64/kernel/setup.c
		└── efi_init()   // drivers/firmware/efi/efi init.c  arch/arm64/include/asm/efi.h
			└── uefi_init()            // drivers/firmware/efi/efi-init.c
				└── efi_systab_report_header()      // drivers/firmware/efi/efi-init.c
					└──  pr_info        // 打印 “efi: EFI v2.7 by EDK II”
```


**解释：**

- 检测到启动是通过 UEFI (EDK II) 环境进行

**UEFI 是什么？**

UEFI（Unified Extensible Firmware Interface，统一可扩展固件接口）是一种现代化的固件接口标准，用于在操作系统启动之前初始化硬件和提供运行时服务。它是传统 BIOS（Basic Input/Output System）的替代方案，具有以下特点：

1. 模块化设计：支持更灵活的硬件初始化和扩展。
2. 图形化界面：相比传统 BIOS 的文本界面，UEFI 支持更高级的用户界面。
3. 支持大容量存储：支持 GPT（GUID Partition Table）分区表，能够处理超过 2TB 的硬盘。
4. 安全启动：提供 Secure Boot 功能，防止未经授权的软件加载。
5. 跨平台支持：支持多种架构（如 x86、x86_64、ARM 等）。

UEFI 的主要目标是提供更快的启动时间、更好的硬件兼容性和更强的安全性。

**EDK II 是什么？**

EDK II（EFI Development Kit II）是一个开源的 UEFI 固件开发框架，由 TianoCore 项目维护。它是开发和构建 UEFI 固件的基础，广泛用于硬件厂商和开发者社区。EDK II 提供了以下内容：

1. UEFI 规范实现：包含对 UEFI 规范的完整实现。
2. 模块化架构：支持开发可复用的固件模块。
3. 参考实现：为开发者提供了构建 UEFI 固件的基础代码。
4. 跨平台支持：支持多种硬件架构（如 x86、ARM 等）。
5. 开源社区：由 TianoCore 社区维护，代码托管在 GitHub 上。

EDK II 是许多硬件厂商（如 Intel、AMD）开发 UEFI 固件的基础，也是开源固件项目（如 Coreboot 和 OVMF）的核心组件。

**总结**

- UEFI 是一种现代化的固件接口标准，用于替代传统 BIOS。
- EDK II 是一个开源的开发框架，用于构建符合 UEFI 规范的固件。

### **5. [    0.000000] efi: ACPI 2.0=0x17c080000 SMBIOS 3.0=0x17c020000 MEMATTR=0x17e6b5a98 MOKvar=0x17f5b0000 INITRD=0x17c133718 MEMRESERVE=0x17c133698**

表示的是从 EFI 系统表（System Table）中提取到的几个重要配置表地址。

**调用过程：**
```
start_kernel()   // init/main.c
└── setup_arch() // arch/arm64/kernel/setup.c
	└── efi_init() // drivers/firmware/efi/efi-init.c    arch/arm64/include/asm/efi.h
		└── uefi_init() // drivers/firmware/efi/efi-init.c
			└── efi_config_parse_tables()// drivers/firmware/efi/efi.c
				└──  match_config_table // 函数会检查 EFI 配置表的 GUID 和地址，并根据匹配结果打印相关信息。
```

**具体代码如下：**
```c
unsigned long table,const efi_config_table_type_t *table_types)
{
    ...
    if (table_types[i].name[0])
        pr_cont("%s=0x%lx ", table_types[i].name, table);
                     // pr_cont 用于在同一行打印表的名称和地址，例如 ACPI 2.0=0x17c080000。
    ...
}
static __init int match_config_table(const efi_guid_t *guid,
```



这些表的名称和地址来源于// drivers/firmware/efi/efi.c --- common_tables 数组中的定义：

这些 GUID 映射定义告诉内核：如果 EFI config table 中包含某个 GUID，就把它的地址绑定给对应指针变量，比如 efi.acpi20、efi.smbios3 等。

```c
static const efi_config_table_type_t common_tables[] __initconst = {

    {ACPI_20_TABLE_GUID,                      &efi.acpi20,        "ACPI 2.0"  },

    {ACPI_TABLE_GUID,                                &efi.acpi,                 "ACPI"      },

    {SMBIOS_TABLE_GUID,                              &efi.smbios,        "SMBIOS"    },

    {SMBIOS3_TABLE_GUID,                      &efi.smbios3,       "SMBIOS 3.0"    },

    {EFI_SYSTEM_RESOURCE_TABLE_GUID,    &efi.esrt,                 "ESRT"      },

    {EFI_MEMORY_ATTRIBUTES_TABLE_GUID,  &efi_mem_attr_table,   "MEMATTR"   },

    {LINUX_EFI_RANDOM_SEED_TABLE_GUID,  &efi_rng_seed,      "RNG"       },

    {LINUX_EFI_TPM_EVENT_LOG_GUID,      &efi.tpm_log,      "TPMEventLog"   },

    {LINUX_EFI_TPM_FINAL_LOG_GUID,      &efi.tpm_final_log,"TPMFinalLog"   },

    {LINUX_EFI_MEMRESERVE_TABLE_GUID,   &mem_reserve,      "MEMRESERVE"    },

    {LINUX_EFI_INITRD_MEDIA_GUID,       &initrd,                 "INITRD"    },

    {EFI_RT_PROPERTIES_TABLE_GUID,      &rt_prop,                "RTPROP"    },

#ifdef CONFIG_EFI_RCI2_TABLE

    {DELLEMC_EFI_RCI2_TABLE_GUID,       &rci2_table_phys            },

#endif

#ifdef CONFIG_LOAD_UEFI_KEYS

    {LINUX_EFI_MOK_VARIABLE_TABLE_GUID, &efi.mokvar_table,  "MOKvar"    },

#endif

#ifdef CONFIG_EFI_COCO_SECRET

    {LINUX_EFI_COCO_SECRET_AREA_GUID,   &efi.coco_secret,  "CocoSecret"    },

#endif

#ifdef CONFIG_UNACCEPTED_MEMORY

    {LINUX_EFI_UNACCEPTED_MEM_TABLE_GUID,   &efi.unaccepted, "Unaccepted"  },

#endif

#ifdef CONFIG_EFI_GENERIC_STUB

    {LINUX_EFI_SCREEN_INFO_TABLE_GUID,  &screen_info_table          },

#endif

    {},

};
```


**✅ ACPI 2.0=0x17c080000   ACPI（高级配置与电源接口）**

- 含义：EFI 提供的 ACPI 2.0 表地址
- 用途：为 Linux 提供系统拓扑、设备、中断控制器等基础信息（比如 GIC、CPU 节点）
- 赋值代码：

**✅ SMBIOS 3.0=0x17c020000**

- 含义：提供主板信息、厂商、BIOS版本等（System Management BIOS）
- 用途：用户空间如 dmidecode 工具会从这里 读取厂商 和 设备数据

**✅ MEMATTR=0x17e6b5a98**

- 含义：EFI Memory Attributes Table
- 用途：内核可以查看某些物理页的属性（如可执行、写保护等）
- 赋值变量：efi_mem_attr_table

**✅ MOKvar=0x17f5b0000**

- 含义：Machine Owner Key 变量区域
- 用途：用于 Secure Boot / shim，验证内核是否受信任
- GUID：LINUX_EFI_MOK_VARIABLE_TABLE_GUID
- 初始化函数：efi_mokvar_table_init()

**✅ INITRD=0x17c133718**

- 含义：initrd 镜像起始地址
- 用途：用户空间初始根文件系统，initramfs 的载入位置
- 宏 / GUID：LINUX_EFI_INITRD_MEDIA_GUID

**✅ MEMRESERVE=0x17c133698**

- 含义：保留的 EFI memory 块，不应映射给一般用途
- 用途：防止一些特殊内存被误用，比如运行时服务区、ACPI NVS区域等

### **7-16. ACPI（Advanced Configuration and Power Interface）是一种标准，用于让操作系统了解主板、CPU、内存、时钟、中断控制器、驱动加载等硬件的结构和配置。ACPI 表来自 BIOS 或 EFI 固件提供的物理地址。**

Linux 内核在 drivers/acpi/ 中对这些表进行解析。

**调用过程：**
```
start_kernel()                         // init/main.c
└── setup_arch()                      // arch/arm64/kernel/setup.c
	└── acpi_boot_table_init()               // arch/arm64/kernel/acpi.c
```


`[    0.000000] ACPI: Early table checksum verification disabled`

`[    0.000000] ACPI: RSDP 0x000000017C080000 000024 (v02 VMWARE)`

- **全称：** Root System Description Pointer
- **作用：** ACPI 的入口地址，告诉内核去哪里找其他表。
- **定义结构：** include/acpi/actbl1.h 中的 struct acpi_table_rsdp
- **解析函数：** acpi_find_root_pointer() → acpi_os_map_memory()

`[    0.000000] ACPI: XSDT 0x000000017C070000 000054 (v01 VMWARE VMWVBSA! 20201221 VMW  00000001)`

- **全称：** Extended System Description Table
- **作用：** 替代传统 RSDT，列出所有 ACPI 子表地址（FACP、DSDT、APIC 等）
- **结构体：** struct acpi_table_xsdt
- **解析位置：** drivers/acpi/tables/tbxface.c → acpi_tb_parse_root_table()

`[    0.000000] ACPI: FACP 0x000000017C060000 000114 (v06 VMWARE VMWVBSA! 20201221 VMW  00000001)`

- **全称：** Fixed ACPI Description Table
- **作用：** 提供固定硬件信息（比如电源管理的寄存器地址）
- **结构体：** struct acpi_table_fadt
- **作用示例：** 控制电源状态切换（如关机）

`[    0.000000] ACPI: DSDT 0x000000017C030000 000ED4 (v01 VMWARE VMWVBSA! 01343F06 INTL 20130823)`

- **全称：** Differentiated System Description Table
- **作用：** 包含 AML（ACPI Machine Language）脚本，定义设备
- **被谁解释：** ACPI 解释器（ACPICA 子系统）
- **函数入口：** acpi_load_tables()

`[    0.000000] ACPI: GTDT 0x000000017C050000 000068 (v03 VMWARE VMWVBSA! 20201221 VMW  00000001)`

- **全称：** Generic Timer Description Table
- **作用：** 提供 ARM Generic Timer 的信息（GIC timer 等）
- **使用者：** drivers/clocksource/arm_arch_timer.c 中的初始化逻辑

`[    0.000000] ACPI: MCFG 0x000000017C040000 00003C (v01 VMWARE VMWVBSA! 20201221 VMW  00000001)`

- **全称：** Memory Mapped Configuration Table
- **作用：** 提供 PCIe 配置空间的起始地址范围
- **使用者：** PCI 子系统 → drivers/pci/setup-bus.c

`[    0.000000] ACPI: APIC 0x000000017C000000 00010C (v04 VMWARE VMWVBSA! 20201221 VMW  00000001)`

- **全称：** Advanced Programmable Interrupt Controller table
- **作用：** 描述中断控制器的拓扑、CPU中断分配
- **解析函数：** acpi_parse_madt() → arch/x86/kernel/acpi/boot.c 或 ARM 版本在 drivers/acpi/arm64

`[    0.000000] ACPI: SSDT 0x000000017BFE0000 000073 (v02 VMWARE VMWVBSA! 20201221 VMW  00000001)`

- **全称：** Secondary System Description Table
- **作用：** 扩展 DSDT，用于定义额外设备（比如 PCI 扩展卡）
- **解释器：** 同样由 AML 子系统解析执行

`[    0.000000] ACPI: PPTT 0x000000017BFC0000 0000B8 (v02 VMWARE VMWVBSA! 20201221 VMW  00000001)`

- **全称：** Processor Properties Topology Table
- **作用：** 描述 CPU 核的拓扑结构（比如哪个核属于哪个 cluster）
- **使用场景：** CPU 缓存层级、线程拓扑的识别
- **相关源码：** drivers/acpi/pptt.c

### **17-19. NUMA（Non-Uniform Memory Access）是指多 CPU 系统中，每个 CPU 核心访问本地内存的速度快于访问远程 CPU 的内存。对于 QEMU、VMware 这些虚拟机来说，默认并不会启用 NUMA 拓扑，因此 Linux 会模拟一个 NUMA node 来兼容结构。**

`[ 0.000000] NUMA: Failed to initialise from firmware`

**调用过程：**
```
start_kernel()                         // init/main.c
└── setup_arch()                      // arch/arm64/kernel/setup.c
	└── bootmem_init();       //arch/arm64/mm/init.c
		└── arch_numa_init();     // drivers/base/arch_numa.c
			└──   numa_init(arch_acpi_numa_init）    // drivers/base/arch_numa.c
```

**代码片段：**
```c
#ifdef CONFIG_ACPI_NUMA

static int __init arch_acpi_numa_init(void)

{

    int ret;

    ret = acpi_numa_init();

    if (ret) {

        pr_info("Failed to initialise from firmware\n");

        return ret;

    }

    return srat_disabled() ? -EINVAL : 0;

}
```

**解释：**

- 内核启动时 尝试从 ACPI 或 设备树 中提取 NUMA 节点信息。
- 如果没有找到 ACPI NUMA table（如 SRAT），就打印这条 log，并调用 numa_add_fake_nodes() 手动伪造 node。

`[ 0.000000] NUMA: Faking a node at [mem 0x0000000080000000-0x000000017fffffff]`

**调用过程：**
```
arch_numa_init();    --->    numa_init(dummy_numa_init);   //driver/base/arch_numa.c

pr_info("NUMA: Faking a node at [mem 0x%016llx-%016llx]\n", start, end);
```

**作用：**

- 将整段物理内存地址映射为一个“伪 NUMA 节点”，让高层内存管理逻辑能正常运行。
- 把从 memblock_start_of_DRAM() 到 memblock_end_of_DRAM() 之间的区域当成一个 NUMA 节点。

`[ 0.000000] NUMA: NODE_DATA [mem 0x17f7fb5c0-0x17f800fff]`

**调用过程：**
```
arch_numa_init();    --->    numa_init();   //driver/base/arch_numa.c

numa_register_nodes       //driver/base/arch_numa.c  

setup_node_data     //driver/base/arch_numa.c  

pr_info("NODE_DATA [mem %#010Lx-%#010Lx]\n", nd_pa, nd_pa + nd_size - 1);
```

**作用：**

- 打印出 每个 NUMA node 的数据结构 struct pglist_data 的物理地址。

**结构体位置：**

`include/linux/mmzone.h` ：
```c
struct pglist_data {  
	...  
};

```

**内核用这个结构记录什么？**

- 每个 node 拥有的 zone 划分（如 DMA、Normal）
- 所有 page 的管理器 pageframe allocator

### **20-25.  Zone**

在内核中，“Zone” 是物理内存的一种逻辑划分，按访问需求或设备约束将物理地址空间切片。

|  Zone 名称 |  作用 |
|---|---|
|DMA|用于设备 DMA 访问的低端内存（32bit DMA 设备通常只支持前 16MB 或 4GB）|
|DMA32|为 64 位平台上 32 位设备提供的最大支持内存区（最多 4GB）|
|Normal|普通内核内存分配区（主要用于页缓存等）|
|Device|专用于内存映射 I/O 或特殊用途的区域（reserved, 不用于分配）|
|Movable|用于 热插拔、迁移（如内存碎片整理）|

`[ 0.000000] Zone ranges:`

- **文件：** mm/page_alloc.c
- **函数：** free_area_init()
- **调用链：  **
```
start_kernel()  
└── setup_arch()  
	└── paging_init()      //arch/arm64/mm/mmu.c    设置页表和内存映射
	└── bootmem_init();       //arch/arm64/mm/init.c
		└── zone_sizes_init() 
			└── free_area_init()   //mm/mm_init.c
				└── pr_info("Zone ranges:\n");
```

- **解释：** 开始打印各个 Zone 的物理内存区间范围。

`[ 0.000000] DMA [mem 0x0000000080000000-0x00000000ffffffff]`

- DMA 区间是 2GB 到 4GB 之间的物理内存
- 一般设置给 DMA-capable 的 legacy 设备（特别是 ARM 上）
- 这个范围是由 arch 定义的：

- `arch/arm64/include/asm/memory.h` 中：  
```c
#define DMA_ZONE_SIZE        UL(0x100000000) // 4GB
```

`[ 0.000000] DMA32 empty`

- 对于 ARM64 架构常常不使用 ZONE_DMA32，因为 ZONE_DMA 已足够。
- 所以这个分区为空。

`[ 0.000000] Normal [mem 0x0000000100000000-0x000000017fffffff]`

- 表示从 4GB 起的地址被划分为 Normal 区间（典型的内核页缓存区间）
- 用于常规的内核物理内存分配，如：

- slab
- page cache
- 文件映射页

`[ 0.000000] Device empty`

- Device 区间是给 memory-mapped I/O 使用的，不用于内核 page 分配。
- QEMU 虚拟机中一般没有设置 Device memory，所以下面为空。

`[ 0.000000] Movable zone start for each node`

- **文件：** `mm/page_alloc.c`
- **函数：** build_all_zonelists()
- **解释：**

- 内核支持“内存迁移”（memory compaction），需要知道哪个 zone 的内存页是 movable。
- 会后续继续打印每个 node 的 movable 起点，比如：  
`Node 0, zone   Normal   4096 pages -> node movable`

### **26-39. [ 0.000000] Early memory node ranges**

这一部分日志是 Linux 内核在 early memory 初始化阶段（boot memory allocator 启动前），打印的物理内存布局

这些日志明确告诉你：当前系统有哪些连续的物理内存段可用，它们都归属于“Node 0”，也就是只有一个 NUMA 节点（常见于桌面机或 QEMU 虚拟机）。

- **调用链：**

同上
```
└── free_area_init()   //mm/mm_init.c
     └── pr_info("Early memory node ranges\n");
```


遍历所有内存块（PFN区段），提取：

- `start_pfn`: 起始页帧号
- `end_pfn`: 结束页帧号（非包含）
- `nid`: 属于哪个 NUMA node（通常为 0

等价于遍历 memblock.memory 中注册的物理内存段。
```c
for_each_mem_pfn_range(i, MAX_NUMNODES, &start_pfn, &end_pfn, &nid) {
```

然后把页帧号转化成物理地址范围打印出来：

- PAGE_SHIFT 一般是 12，表示页大小 4KB
- 左移 12 位得到字节级地址
- 减 1 表示“包含”的范围：[start, end-1]
```c
pr_info("  node %3d: [mem %#018Lx-%#018Lx]\n", nid,
(u64)start_pfn << PAGE_SHIFT,
((u64)end_pfn << PAGE_SHIFT) - 1);
// 初始化 mem_section 的子节（subsection）结构
// 用于后续的内存页映射、稀疏页表、memory hotplug 等内核子系统追踪每一段内存的状态
subsection_map_init(start_pfn, end_pfn - start_pfn);
}
```

`[    0.000000]   node   0: [mem 0x000000017f5d0000-0x000000017fffffff]`

- Node 0 是 NUMA 节点 0（大多数系统只有一个）
- 每一段 [mem xxx - yyy] 是可用的物理内存地址范围
- 这些地址范围是从 bootloader 或 UEFI ACPI 表中解析出来的
- 在 setup_arch() 阶段通过 memblock_add() 注册给 memblock 子系统管理

|日志地址区间 | 说明 |
| - | - |
|[0x80000000 - 0xffd8cfff] |主内存块，大部分可用 |
|[0xffd8d000 - 0xfffb5fff] |可能是 UEFI/E820 中保留但仍可用的空隙区域 |
| [0xfffb6000 - 0x17beffff] |大段的正常 DRAM 区域 |
| [0x17bf0000 - 0x17bfbfff] |通常是 ACPI 表、RSDP、SMBIOS 等 metadata 所在位置 |
| [0x17bfc000 - 0x17c01fff] |可能为 FDT 或 ACPI GTDT 区 |
| [0x17c020000 - 0x17c12fff] |SMBIOS 等其它 EFI tables 映射区域 |
| [0x17c130000 - 0x17f40fff] |INITRD（根文件系统）的展开区域 |
| [0x17f410000 - 0x17f5cfff] |EFI runtime reserved 区域（例如 MOKvar） |
| [0x17f5d0000 - 0x17fffffff] |剩余 DRAM |

### **40. [    0.000000] Initmem setup node 0 [mem 0x0000000080000000-0x000000017fffffff]**
**调用链：**
同上
```
└── free_area_init()   //mm/mm_init.c
	└── free_area_init_node       //mm/mm_init.c    初始化指定节点的内存区域。
```


- **解释：**
	- 初始化 NODE_DATA 区域，也就是每个 NUMA 节点的物理内存布局。
	- 这是在没有 NUMA 的情况下，伪造 node 0 的内存范围；
	- 分配 NODE_DATA(nid) 结构体，给页分配器准备使用；
	- 打印出当前物理内存 node 的地址范围。

### **41. [ 0.000000] cma: Reserved 32 MiB at 0x00000000fdc00000 on node -1**

**调用链：**
```
start_kernel()
└── setup_arch()
	└── bootmem_init();       //arch/arm64/mm/init.c
		└── dma_contiguous_reserve(arm64_dma_phys_limit);//kernel/dma/contiguous.c 
			└── cma_declare_contiguous()
```


**功能：**

CMA（Contiguous Memory Allocator）为连续内存保留一块区域，供设备驱动 DMA 使用。

**说明：**

- node = -1 表示不特定某个 NUMA 节点；
- CMA区域通过 device tree 或参数 cma=32M 自动保留。

### **42.[ 0.000000] psci: probing for conduit method from ACPI**
### **43.[ 0.000000] psci: PSCIv0.2 detected in firmware.**
### **44.[ 0.000000] psci: Using standard PSCI v0.2 function IDs**
### **45.[ 0.000000] psci: Trusted OS migration not required**

**调用链：**
```
start_kernel()  
└── setup_arch()
	└── psci_acpi_init();       //drivers/firmware/pcsi/pcsi.c 
		└──  set_conduit()    //drivers/firmware/pcsi/pcsi.c
			└──   psci_probe()     //drivers/firmware/pcsi/pcsi.c
```


**功能：**

PSCI（Power State Coordination Interface）用于 ARM64 CPU 的电源管理。

**说明：**

- 内核识别 PSCI 的版本（v0.2）
- 如果支持 ACPI，优先用 ACPI 方式识别 conduit
- Trusted OS migration not required 是 SMC 接口的返回值。

### **46.[ 0.000000] percpu: Embedded 54 pages/cpu s95784 r8192 d117208 u221184**

**调用链：**
```
start_kernel()  
  → setup_per_cpu_areas()    //mm/percpu.c
```

**功能：**

内核为每个 CPU 分配了“percpu” 区域（每核私有变量空间）。

**说明：**

- s95784：总空间大小；
- r d u 表示 reserved、dynamic、used 的大小；
- 用于如 this_cpu_ptr() 等宏来访问每个 CPU 的本地变量。

### **47. [ 0.000000] pcpu-alloc: s95784 r8192 d117208 u221184 alloc=54\*4096**
### **48. [ 0.000000] pcpu-alloc: [0] 0 [0] 1**

**源码位置：**
```
start_kernel()
└── setup_per_cpu_areas()    //mm/percpu.c
	└── pcpu_embed_first_chunk()      // mm/percpu.c
		└── pcpu_setup_first_chunk()        // mm/percpu.c
			└── PCPU_SETUP_BUG_ON  --- pcpu_dump_alloc_info()
```


**功能：**

详细展示每个 CPU percpu 内存页的分配详情。

为所有 CPU 分配并映射 __percpu 变量区域

### **49. [ 0.000000] Detected PIPT I-cache on CPU0**

**源码路径：**
```
start_kernel()
└──smp_prepare_boot_cpu()     // arch/arm64/kernel/smp.c
	└──__cpuinfo_store_cpu()      // arch/arm64/kernel/cpuinfo.c
		└──cpuinfo_detect_icache_policy()             
```


**功能：**

- 内核检测到 CPU0 的 I-Cache（指令缓存）使用的是 PIPT（Physically Indexed, Physically Tagged）策略，

即：按物理地址索引和标记缓存行。

### **50. [ 0.000000] CPU features: detected: Address authentication (IMP DEF algorithm)**
### **51. [ 0.000000] CPU features: detected: GIC system register CPU interface**
### **52. [ 0.000000] CPU features: detected: Spectre-v4**

**调用链：**
```
start_kernel()
└── smp_prepare_boot_cpu     // arch/arm64/kernel/smp.c
	└── cpuinfo_store_boot_cpu    // arch/arm64/kernel/cpuinfo.c
		└── init_cpu_features    //arch/arm64/kernel/cpufeature.c
			└── setup_boot_cpu_capabilities//arch/arm64/kernel/cpufeature.c
				└── update_cpu_capabilities(SCOPE_BOOT_CPU | SCOPE_LOCAL_CPU)
```


**功能：**

**50** 内核在启动时检测到了 CPU 支持 "Address authentication"（地址认证）IMP DEF 表示使用了实现自定义算法（而非标准 ARM 算法）。

**51** 说明 CPU 支持通过 系统寄存器访问 GIC（Generic Interrupt Controller），而不是通过 MMIO 访问。

**52** CPU 被内核检测出 存在 Spectre-v4（预测执行旁路）漏洞。

### **53. [ 0.000000] alternatives: applying boot alternatives**

**调用链：**
```
start_kernel()
└── smp_prepare_boot_cpu   // arch/arm64/kernel/smp.c
	└── apply_boot_alternatives   // arch/arm64/kernel/alternative.c
```

**功能：**

替换某些关键路径的指令实现（比如优化内存拷贝），根据 CPU 特性动态打 patch。

### **54.[ 0.000000] Kernel command line: BOOT_IMAGE=/vmlinuz-6.8.0-55-generic root=/dev/mapper/ubuntu--vg-ubuntu--lv ro**

**调用链：**
```
start_kernel()
└── pr_notice("Kernel command line: %s\n", saved_command_line);
```

**功能：**

显示内核启动参数，来自 GRUB 或 bootloader。

### **55. [ 0.000000] Unknown kernel command line parameters "BOOT_IMAGE=...", will be passed to user space.**

**源码路径：**
```
start_kernel()
└── print_unknown_bootoptions()
```

**功能：**

内核解析不了 BOOT_IMAGE，标记为“用户空间处理”。