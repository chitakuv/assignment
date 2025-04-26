本阶段核心作用：

为正式启动用户空间做准备，完成内核子系统与 CPU 的初始化

## ① RCU 子系统初始化（第 70~76 条）


```log
70.[    0.000000] rcu: Preemptible hierarchical RCU implementation.
71.[    0.000000] rcu:         RCU restricting CPUs from NR_CPUS=512 to nr_cpu_ids=2.
72.[    0.000000]         Trampoline variant of Tasks RCU enabled.
73.[    0.000000]         Rude variant of Tasks RCU enabled.
74.[    0.000000]         Tracing variant of Tasks RCU enabled.
75.[    0.000000] rcu: RCU calculated value of scheduler-enlistment delay is 100 jiffies.
76.[    0.000000] rcu: Adjusting geometry for rcu_fanout_leaf=16, nr_cpu_ids=2
```

70. rcu: Preemptible hierarchical RCU implementation.  
71. rcu: Restricting CPUs from NR_CPUS=512 to nr_cpu_ids=2.  
72~74. 启用 Tasks RCU 各个变体：Trampoline, Rude, Tracing  
72. scheduler-enlistment delay = 100 jiffies  
73. 设置 RCU fanout geometry（树结构）

**核心文件：**

- kernel/rcu/tree.c
- kernel/rcu/tasks*.c

**调用链：**

```
start_kernel()  
└── rcu_init()                          // kernel/rcu/tree.c      
	├── rcu_init_geometry()      
	├── rcu_init_one()      
	├── rcu_spawn_gp_kthread()  
	└── rcu_init_tasks_generic()           // kernel/rcu/tasks.c      
		├── rcu_tasks_boot_tasks_rcu()      
		├── rcu_tasks_rude_init()  
		└── rcu_tasks_trace_init()
```
**作用：**

- 初始化主 RCU 树结构
- 注册 CPU 节点、任务跟踪器
- 启用 Tasks-RCU 系列变体用于用户态任务生命周期追踪

**说明：**

RCU 是内核中用于高效读写同步的机制，尤其在多核系统中很关键。

## ② 中断控制器 GIC 初始化（第 77~83 条）

```log
77.[    0.000000] NR_IRQS: 64, nr_irqs: 64, preallocated irqs: 0
78.[    0.000000] GICv3: 960 SPIs implemented
79.[    0.000000] GICv3: 0 Extended SPIs implemented
80.[    0.000000] Root IRQ handler: gic_handle_irq
81.[    0.000000] GICv3: GICv3 features: 16 PPIs
82.[    0.000000] GICv3: CPU0: found redistributor 0 region 0:0x000000002c100000
83.[    0.000000] GICv2m: range[mem 0x2c080000-0x2c080fff], SPI[64:990]
```

- 77. NR_IRQS: 硬中断总数（逻辑定义）
- 77. GICv3: 960 个 SPI  
- 77. GICv3: 16 PPIs  
- 82-83. Redistributor 初始化和 SPI 范围配置

**作用：**

- 初始化 GICv3 中断控制器，建立中断描述符表
- 配置 SPI（共享中断）和 PPI（私有中断）

**核心文件：**

- drivers/irqchip/irq-gic-v3.c
- arch/arm64/kernel/irq.c

**调用链：**

```
start_kernel()  
└── init_IRQ()                         // arch/arm64/kernel/irq.c  
	└── irqchip_init()               // irqchip/core.c  
	└── gic_of_init()           // irq-gic-v3.c                
		├── gic_smp_init()  
		└── gic_init_bases()
```

## ③ 定时器与调度器时钟初始化（第 85~87 条）

```log
84.[    0.000000] rcu: srcu_init: Setting srcu_struct sizes based on contention.
85.[    0.000000] arch_timer: cp15 timer(s) running at 24.00MHz (virt).
86.[    0.000000] clocksource: arch_sys_counter: mask: 0xffffffffffffff max_cycles: 0x588fe9dc0, max_idle_ns: 440795202592 ns
87.[    0.000000] sched_clock: 56 bits at 24MHz, resolution 41ns, wraps every 4398046511097ns
```

**作用：**

- 初始化高精度时钟源
- 设置 sched_clock，后续进程调度/定时器依赖它

**核心文件：**

- arch/arm64/kernel/time.c
- kernel/time/clocksource.c

**调用链：**

```
start_kernel()  
 └── time_init()                        // arch/arm64/kernel/time.c  
      └── arch_timer_of_init()  
      └── register_current_timer_delay()  
 └── sched_clock_postinit()            // kernel/time/sched_clock.c
```
④ 控制台 & BogoMIPS & pid 管理（第 88~92 条）

```
88.[    0.000120] Console: colour dummy device 80x25
89.[    0.000121] printk: legacy console [tty0] enabled
90.[    0.001281] ACPI: Core revision 20230628
91.[    0.001602] Calibrating delay loop (skipped), value calculated using timer frequency.. 48.00 BogoMIPS (lpj=24000)
92.[    0.001604] pid_max: default: 32768 minimum: 301
88~89. Console/printk 初始化91. BogoMIPS 计算：衡量 CPU 处理指令速率92. pid_max 设置：最大进程数
88~89. Console/printk 初始化91. BogoMIPS 计算：衡量 CPU 处理指令速率92. pid_max 设置：最大进程数
```

**作用：**

- 配置早期控制台（tty0）供 printk 输出
- 计算并设置伪指令速率（非性能指标）
- 初始化 PID 分配器，设置最大可用进程号

**核心文件：**

- kernel/printk/printk.c
- kernel/pid.c
- init/main.c

**调用链：**

```
start_kernel()  
 └── console_init()                    // kernel/printk/printk.c  
 └── calibrate_delay()                // init/calibrate.c  
 └── pid_idr_init()                   // kernel/pid.c
```

## ⑤ 安全模块（LSM）初始化（第 93~96 条）


```
93.[    0.003699] LSM: initializing lsm=lockdown,capability,landlock,yama,apparmor,integrity
94.[    0.003713] landlock: Up and running.
95.[    0.003714] Yama: becoming mindful.
96.[    0.003902] AppArmor: AppArmor initialized
```

93. LSM: lockdown, capability, landlock, yama, apparmor  
94. landlock 启动95. yama 启动96. AppArmor 初始化完成

**作用：**

- 初始化 Linux 安全模块（LSM）框架
- 加载用户配置的安全策略，如 AppArmor、Yama 等

**核心文件：**

- security/security.c
- security/apparmor/lsm.c
- security/yama/lsm.c

**调用链：**

```
start_kernel()  
 └── security_init()                  // security/security.c      
	 ├── lsm_early_cred()      
	 ├── yama_add_hooks()      
	 ├── apparmor_init()
```
注意：通过 __lsm_initcall 注册，各 LSM 被动态加载。

## ⑥ 多核 CPU 启动与特性识别（第 105~127 条）

```
97.[    0.004569] Mount-cache hash table entries: 8192 (order: 4, 65536 bytes, linear)
98.[    0.004576] Mountpoint-cache hash table entries: 8192 (order: 4, 65536 bytes, linear)
99.[    0.006461] RCU Tasks: Setting shift to 1 and lim to 1 rcu_task_cb_adjust=1 rcu_task_cpu_ids=2.
100.[    0.006489] RCU Tasks Rude: Setting shift to 1 and lim to 1 rcu_task_cb_adjust=1 rcu_task_cpu_ids=2.
101.[    0.006506] RCU Tasks Trace: Setting shift to 1 and lim to 1 rcu_task_cb_adjust=1 rcu_task_cpu_ids=2.
102.[    0.006584] rcu: Hierarchical SRCU implementation.
103.[    0.006585] rcu:         Max phase no-delay instances is 400.
104.[    0.006777] Remapping and enabling EFI services.
105.[    0.006877] smp: Bringing up secondary CPUs ...
106.[    0.007123] Detected PIPT I-cache on CPU1
107.[    0.007133] GICv3: CPU1: found redistributor 1 region 0:0x000000002c120000
108.[    0.007179] CPU1: Booted secondary processor 0x0000000001 [0x610f0000]
109.[    0.007236] smp: Brought up 1 node, 2 CPUs
110.[    0.007239] SMP: Total of 2 processors activated.
111.[    0.007240] CPU: All CPU(s) started at EL1
112.[    0.007249] CPU features: detected: ARMv8.4 Translation Table Level
113.[    0.007250] CPU features: detected: Data cache clean to the PoU not required for I/D coherence
114.[    0.007251] CPU features: detected: Common not Private translations
115.[    0.007252] CPU features: detected: CRC32 instructions
116.[    0.007253] CPU features: detected: Data cache clean to Point of Deep Persistence
117.[    0.007254] CPU features: detected: Data cache clean to Point of Persistence
118.[    0.007254] CPU features: detected: Data independent timing control (DIT)
119.[    0.007255] CPU features: detected: E0PD
120.[    0.007256] CPU features: detected: Generic authentication (IMP DEF algorithm)
121.[    0.007257] CPU features: detected: RCpc load-acquire (LDAPR)
122.[    0.007257] CPU features: detected: LSE atomic instructions
123.[    0.007258] CPU features: detected: Privileged Access Never
124.[    0.007259] CPU features: detected: RAS Extension Support
125.[    0.007259] CPU features: detected: Speculation barrier (SB)
126.[    0.007260] CPU features: detected: TLB range maintenance instructions
127.[    0.007284] alternatives: applying system-wide alternatives
```


105~109. smp: 启动 CPU1  
110~111. SMP 总 CPU 数报告112~126. 启用 CPU 架构功能特性（如 CRC32, SB, RAS 等）127. 启用 instruction patching alternatives（静态优化替换）

**作用：**

- 启动剩余 CPU 核（通过 smp bringup）
- 探测并启用当前架构支持的指令集扩展
- 应用 alternatives 机制，为不同硬件生成优化路径

**核心文件：**

- arch/arm64/kernel/smp.c
- arch/arm64/kernel/cpuinfo.c
- arch/arm64/kernel/setup.c

**调用链：**

```
start_kernel()  
 └── smp_prepare_cpus()               // arch/arm64/kernel/smp.c  
 └── smp_init()                       // kernel/smp.c  
      └── bringup_secondary_cpus()  
 └── cpuinfo_store_cpu()             // arch/arm64/kernel/cpuinfo.c  
 └── apply_alternatives_all()        // arch/arm64/kernel/alternative.c
```
最后：这一阶段结尾标志

`128. devtmpfs: initialized`

表示 /dev 设备管理虚拟文件系统已经初始化完成，这标志着系统“即将切入 do_basic_setup 阶段”，也就是设备驱动 + initcall 初始化阶段。