**【此阶段全部进入 C 语言主体：start_kernel()】**

### **56. [    0.000000] Dentry cache hash table entries: 524288 (order: 10, 4194304 bytes, linear)**

**函数：**

```
start_kernel()                    // init/main.c
└── vfs_caches_init_early()    //  fs/dcache.c
	└──dcache_init_early()      // 分配 dentry_hashtable

```

**功能：**

- 初始化目录项缓存，初始化了 2^19 个哈希桶（524288）用于存放 dentry，总共约 4MB，用于加速目录查找

**解释：**

- dentry 全称是 directory entry，即「目录项」。
- 它是 VFS（虚拟文件系统）中用于缓存路径解析结果 的结构体，避免频繁访问磁盘。
- 每访问一次 /home/laozhu/file.txt，内核都会查 dentry 缓存看有没有缓存 /home、/home/laozhu、file.txt，加快访问速度。

### **57. [ 0.000000] Inode-cache hash table entries: 32768 (order: 6, 262144 bytes, linear)**

**函数：**

```
start_kernel()                    // init/main.c
└── vfs_caches_init_early()    //  fs/dcache.c
	└──inode_init_early()      // fs/inode.c    inode 缓存
		└── alloc_large_system_hash()
			└──inode_hashtable 初始化

```

**说明：**

- 初始化 inode 缓存，保存文件系统中每个文件的元数据

### **58. [ 0.000000] Fallback order for Node 0: 0**

**函数：**

```
start_kernel()                         // init/main.c
└──  mm_core_init()                   // init/main.c
	└── build_all_zonelists()        // mm/page_alloc.c
		└── build_zonelists(pgdat)      // mm/page_alloc.c
			└── build_zonelists_in_node_order()
				└── build_thisnode_zonelists()
					└── pr_info("Fallback order for Node %d: ", local_node);

```

***说明：***

系统在构建内存管理的 zone 列表（zonelists）时，需要指定在某个内存节点（NUMA node）分配失败时，从哪些节点 fallback（回退）分配。

- 在启用了 NUMA 的系统中，可能有多个节点（Node 0, Node 1, ...）。
- 当前系统只有 Node 0，因此 fallback 策略就是自己。

### **59. [ 0.000000] Built 1 zonelists, mobility grouping on. Total pages: 129024**

**函数：**

```
start_kernel()
└── mm_core_init()
	└──   build_all_zonelists()
		└──  build_zonelists()
			└──   pr_info("Built %i zonelists, mobility grouping on. Total pages: %ld\n",nr_online_nodes, num_physpages);

```


**说明：**

- 系统完成了 zone 列表的初始化工作。
- 其中包含一个 zonelist，代表只有一个 memory node（Node 0）。
- mobility grouping：用于页面分配的分组（可移动、不可移动、回收页），提高内存回收效率。
- Total pages = 物理内存总页数（1 page = 4KB），也就是 129024 * 4KB ≈ 504MB


## **60. [    0.000000] Policy zone: Normal**

**函数：**
```
start_kernel()
	└── mm_core_init()
		└── build_all_zonelists()
			└── calculate_zone_totalpages()
			└── set_pageblock_migratetype()
			└── page_alloc_init_late()
				└── setup_per_zone_wmarks()
					└── printk("Policy zone: %s", zone_names[policy_zone]);
```


**说明：**

系统最终将所有默认内存分配操作的 首选 zone 类型 设置为 ZONE_NORMAL

在 Linux 中，常见的 zone 类型包括：

- ZONE_DMA：供 DMA（32-bit）设备使用（低端内存）
- ZONE_DMA32：供某些 64-bit DMA 设备使用
- ZONE_NORMAL：大多数内存分配都会走这里
- ZONE_HIGHMEM（x86）：高端内存，ARM64 不使用
- ZONE_MOVABLE：可被回收/迁移的内存页


## **61. [ 0.000000] mem auto-init: stack:off, heap alloc:off, heap free:off**

**函数：**

```
start_kernel()                          // init/main.c
└── mm_core_init()                 // mm/mm_init.c
	└── report_meminit()            // mm/page_alloc.c
		└── pr_info(...)           // printk 输出 log
```

**说明：**

设置内核内存初始化策略，是否在堆栏/heap 中做先期清空（进一步优化启动时间）

这条 log 表示当前内核启动时关闭了“自动内存清零功能（mem auto-initialization）”：

- stack:off —— 栈内存初始化（清零）未开启。
- heap alloc:off —— 分配时堆内存不清零。
- heap free:off —— 释放时堆内存不清零。

这是为了 提升启动性能，在嵌入式等场景下尤其重要。


### **62. [ 0.000000] software IO TLB: area num 2.**

**函数：**

```
start_kernel()                        // init/main.c
└── parse_early_param()                 // init/main.c
	└── early_param("swiotlb", setup_io_tlb_npages)   注册
		└── setup_io_tlb_npages()        // kernel/dma/swiotlb.c
			└── swiotlb_adjust_nareas()    //kernel/dma/swiotlb.c
				└── pr_info("area num %d.\n", nareas)
```

**说明：**

这是内核启动时初始化 Software IO TLB（软 I/O 快取缓冲） 的日志。

**用于：**

- DMA 数据交换时，CPU 与硬件设备之间的 非连续物理地址 映射。
- 尤其用于不支持“直接地址访问”（Direct Mapping）的设备。

这里的 area num 2 表示初始化了 2 个 IO TLB 区块区域。

### **63.[    0.000000] software IO TLB: mapped [mem 0x00000000f9c00000-0x00000000fdc00000] (64MB)**

**函数：**

```
start_kernel()                      // init/main.c
└── mm_core_init()                  // core_initcall 宏注册
	└── mem_init()                 // arch/arm64/mm/init.c
		└── swiotlb_init()         // lib/swiotlb.c
			└──swiotlb_init_with_tbl()
				└── swiotlb_init_remap()  <-- 打印 mapped 的 log
```

**说明：**

软件 I/O TLB（Bounce Buffer）区域已经映射成功，从物理地址 0xf9c00000 到 0xfdc00000，总计 64MB。

这块区域用于：

DMA 设备不支持访问高地址内存

或 IOMMU 不可用的情况下，通过中转 bounce buffer 进行 DMA 映射。

### **64.[    0.000000] Memory: 3883088K/4194304K available (23104K kernel code, 5190K rwdata, 16008K rodata, 12224K init, 1233K bss, 278448K reserved, 32768K cma-reserved)**

**函数：**

```
start_kernel()                        // init/main.c
└── mm_core_init()                  // mm/mm_init.c，core_initcall
	└── mem_init()                // arch/arm64/mm/init.c
		└── mem_init_print_info()      // mm/mm_init.c
```

**说明：**

- 这条日志展示的是内核启动时内存布局与使用情况的统计汇总，字段含义如下：

|字段|说明|
|---|---|
|4194304K|总物理内存：4GB（= 1024 * 1024 * 4）|
|3883088K|可用内存：减去了内核和预留区域|
|23104K kernel code|内核的代码段 .text|
|5190K rwdata|可读写的静态数据段 .data|
|16008K rodata|只读数据段 .rodata|
|12224K init|内核初始化代码段（__init），启动后会释放|
|1233K bss|未初始化数据段 .bss|
|278448K reserved|内核保留区，可能用于设备、BIOS、页表等|
|32768K cma-reserved|Contiguous Memory Allocator（CMA）保留区，供大型设备 DMA 使用|


### **65. [ 0.000000] SLUB: HWalign=64, Order=0-3, MinObjects=0, CPUs=8, Nodes=1**

**函数：**

```
start_kernel()                        // init/main.c
└── mm_core_init()                  // mm/mm_init.c，core_initcall
	└── mem_init()                // arch/arm64/mm/init.c
		└── kmem_cache_init()     // mm/slub.c
			└── pr_info(...)      // ← 打印这条日志     
```

**说明：**

- 内核初始化SLUB 分配器，用于替代传统的 slab 分配器。

SLUB 是内核中的一种 slab 分配器，用于 频繁创建/销毁的小对象内存分配，例如：

- task_struct、file、inode、dentry

它比老式 SLAB 更快、更简洁，支持更好的 NUMA 和 per-cpu 缓存。

含义逐项解释：

|SLUB|代表正在启用的是 SLUB 分配器（Simple Lazy User-friendly Buddy allocator）|
|---|---|
|HWalign=64|硬件对齐要求是 64 字节（通常与 CPU cache line 大小一致）|
|Order=0-3|支持的页框 order 从 0（1页）到 3（8页），即最大一次可分配 8 页连续内存|
|MinObjects=0|每个 slab 最少对象数目（某些 slab 支持空 slab 初始化）|
|CPUs=8|当前系统检测到 8 个处理器核|
|Nodes=1|当前 NUMA 拓扑中只有 1 个节点（单节点系统）|

### **66.[    0.000000] ftrace: allocating 62652 entries in 245 pages**

**函数：**

```
start_kernel()                                // init/main.c
└── ftrace_init()                       // kernel/trace/ftrace.c
	└──pr_info("ftrace: allocating %ld entries in %ld pages\n",count,DIV_ROUND_UP(count, ENTRIES_PER_PAGE));
```

**说明：**

- 该日志表示 Ftrace 动态分配了用于记录函数调用的 tracing 表（dyn_ftrace 实体），

分布在多个页中，数量根据 CPU 核数和函数符号表决定。

- tracing 表 = 函数追踪记录表，保存了“哪些函数需要被追踪、它们的地址、追踪状态”等信息。

它是 Ftrace（Function Tracer）子系统中的关键数据结构，由内核在启动时通过分析内核符号表（symbol table）创建。

### **67.[    0.000000] ftrace: allocated 245 pages with 6 groups**

**函数：**

```
start_kernel()                                // init/main.c
└── ftrace_init()                       // kernel/trace/ftrace.c
	└──pr_info("ftrace: allocated %ld pages with %ld groups\n",ftrace_number_of_pages, ftrace_number_of_groups);
```

**说明：**

- 将 tracing 表按 CPU 和 cache locality 分组，优化并发和性能。
- 用于支持 function graph tracer、function tracer 等子系统。

### **68.[    0.000000] trace event string verifier disabled**

**函数：**

```
start_kernel()                                 // init/main.c
└── trace_init()                              // kernel/trace/trace.c
	└── trace_event_init()                   // kernel/trace/trace_event.c
		└── check_and_disable_verifier()    // kernel/trace/trace_event.c
			└── pr_info(...)               //kernel/trace/trace_event.c
```

**说明：**

- “trace event string verifier” 是一个验证 event 字符串格式的工具，在 debug 模式下启用。
- 正常内核中默认关闭，避免 runtime overhead。

69.[    0.000000] Dynamic Preempt: voluntary

**函数：**

```
└──klp_try_complete_transition()  // kernel/livepatch/transition.c
	└── klp_cond_resched_disable()           // kernel/livepatch/transition.c
	=sched_dynamic_klp_diable()      // kernel/sched/core.c
		└── __sched_dynamic_update()   // kernel/sched/core.c
			└── pr_info(...)              // kernel/sched/core.c
```

**说明：**

- 设置 内核的可抢占策略设为 voluntary，表示内核代码中仅在显式的调度点进行上下文切换，非实时场景使用。

当前启用的是 自愿抢占（voluntary preemption）

 也就是：只有内核代码中主动调用了 cond_resched()，才可能切换任务。