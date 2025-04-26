### systemd 核心服务与模块加载（1209 – 1233 行） ​

- 挂载 dev-hugepages、dev-mqueue、debugfs、tracefs（1209–1212）
    
- 启动 journald、keyboard-setup、kmod-static-nodes、lvm2-monitor（1213–1216）
    
- modprobe 加载 configfs、dm_mod、drm、efi_pstore、fuse、loop（1218–1225）
    
- systemd-modules-load、remount-fs、udev-trigger（1228–1233）
    

##### 挂载内核虚拟文件系统（1209–1212 行） ​

1. **dev-hugepages.mount**
    
    - **日志：**
        
        ```
        systemd[1]: Mounting dev-hugepages.mount - Huge Pages File System...
        ```
        
    - **做了什么：**  
        在 `mount_unit_start()` 中执行 `mount(..., "hugetlbfs", ...)`，将 `hugetlbfs` 挂载到 `/dev/hugepages`，用于支持用户配置的大页分配。
        
    - **术语：**  
        Huge Pages —— 预留的大内存页，减少 TLB 缺失开销。
        
2. **dev-mqueue.mount**
    
    - **日志：**
        
        ```
        systemd[1]: Mounting dev-mqueue.mount - POSIX Message Queue File System...
        ```
        
    - **功能：**  
        挂载 `mqueue` 文件系统到 `/dev/mqueue`，由 `mount("mqueue","/dev/mqueue","mqueue",0,…)` 完成。供进程间基于 POSIX mq 的消息排队。
        
3. **sys-kernel-debug.mount**
    
    - **日志：**
        
        ```
        systemd[1]: Mounting sys-kernel-debug.mount - Kernel Debug File System...
        ```
        
    - **功能：**  
        挂载 `debugfs` 到 `/sys/kernel/debug`，提供内核调试接口（如 perf、ftrace）。
        
4. **sys-kernel-tracing.mount**
    
    - **日志：**
        
        ```
        systemd[1]: Mounting sys-kernel-tracing.mount - Kernel Trace File System...
        ```
        
    - **功能：**  
        挂载 `tracefs` 到 `/sys/kernel/tracing`，用于动态跟踪系统调用、IRQ 等事件。
        

##### 启动关键 systemd 服务（1213–1216 行） ​

1. **systemd-journald.service**
    
    - **日志：**
        
        ```
        systemd[1]: Starting systemd-journald.service - Journal Service...
        ```
        
    - **做了什么：**  
        `service_unit_start()` → `service_exec_start()` 启动 `journald` 守护进程，收集内核及用户态日志。
        
2. **keyboard-setup.service**
    
    - **日志：**
        
        ```
        systemd[1]: Starting keyboard-setup.service - Set the console keyboard layout...
        ```
        
    - **功能：**  
        加载键盘布局（`loadkeys`、`kbd`），确保控制台输入时正确映射物理键。
        
3. **kmod-static-nodes.service**
    
    - **日志：**
        
        ```
        systemd[1]: Starting kmod-static-nodes.service - Create List of Static Device Nodes...
        ```
        
    - **功能：**  
        扫描 `/lib/modules/.../modules.builtin` 中内置模块，为内核编译进来的设备节点在 `/dev` 下创建静态节点。
        
4. **lvm2-monitor.service**
    
    - **日志：**
        
        ```
        systemd[1]: Starting lvm2-monitor.service - Monitoring of LVM2 mirrors, snapshots etc. using dmeventd or progress polling...
        ```
        
    - **功能：**  
        启动 `dmeventd`（或轮询模式）监控 LVM2 设备的镜像和快照状态。
        

##### 按需加载内核模块（1218–1225 行） ​

1. **configfs**
    
    - **日志：**
        
        ```
        systemd[1]: Starting modprobe@configfs.service - Load Kernel Module configfs...
        ```
        
    - **调用：**  
        执行 `ExecStart=+/usr/bin/modprobe configfs`，将 `configfs` 模块加载到内核中，支持用户空间配置文件系统。
        
2. **dm_mod**
    
    - **日志：**
        
        ```
        systemd[1]: Starting modprobe@dm_mod.service - Load Kernel Module dm_mod...
        ```
        
    - **功能：**  
        加载 Device-Mapper 核心模块，为 LVM、加密、RAID 等提供底层支持。
        
3. **drm**
    
    - **日志：**
        
        ```
        systemd[1]: Starting modprobe@drm.service - Load Kernel Module drm...
        ```
        
    - **功能：**  
        加载通用 Direct Rendering Manager 模块，为 GPU 和显示子系统打好基础。
        
4. **efi_pstore**
    
    - **日志：**
        
        ```
        systemd[1]: Starting modprobe@efi_pstore.service - Load Kernel Module efi_pstore...
        ```
        
    - **功能：**  
        加载 EFI 持久存储后端，用于保存崩溃转储到 NVRAM 或固件保留区。
        
    - **内核输出：**
        
        ```
        pstore: Using crash dump compression: deflate
        pstore: Registered efi_pstore as persistent store backend
        ```
        
5. **fuse**
    
    - **日志：**
        
        ```
        systemd[1]: Starting modprobe@fuse.service - Load Kernel Module fuse...
        ```
        
    - **功能：**  
        挂载用户空间文件系统支持，由 `fuse.ko` 提供。
        
6. **loop**
    
    - **日志：**
        
        ```
        systemd[1]: Starting modprobe@loop.service - Load Kernel Module loop...
        ```
        
    - **功能：**  
        加载 `loop.ko`，支持将文件作为块设备进行挂载。
        
7. **Condition 跳过**
    
    - **说明：**  
        `netplan-ovs-cleanup.service` 与 `systemd-fsck-root.service` 因不满足 `ConditionFileIsExecutable` 或 `ConditionPathExists` 而跳过启动。
        

##### 最后的核心模块与文件系统调整（1228–1233 行） ​

1. **systemd-modules-load.service**
    
    - **日志：**
        
        ```
        systemd[1]: Starting systemd-modules-load.service - Load Kernel Modules...
        ```
        
    - **功能：**  
        在 `/etc/modules-load.d/` 和 `/usr/lib/modules-load.d/` 中读取配置，一次性加载其列出的模块。
        
2. **systemd-remount-fs.service**
    
    - **日志：**
        
        ```
        systemd[1]: Starting systemd-remount-fs.service - Remount Root and Kernel File Systems...
        ```
        
    - **做了什么：**  
        在早期挂载阶段后，可能以只读方式挂载根分区；此服务根据 `/etc/fstab` 重新以读写或其它选项挂载所有核心文件系统。
        
3. **systemd-udev-trigger.service**
    
    - **日志：**
        
        ```
        systemd[1]: Starting systemd-udev-trigger.service - Coldplug All udev Devices...
        ```
        
    - **功能：**  
        触发 `udev` 处理所有已存在设备（coldplug），确保在 initramfs 卸载后，用户空间 `udev` 正确创建 `/dev` 中的节点。
        
4. **Journal 完成**
    
    - **日志：**
        
        ```
        systemd[1]: Started systemd-journald.service - Journal Service.
        ```
        
    - **含义：**  
        `systemd` 确认 `journald` 服务已成功启动，可以开始持久化日志。
        

### 根文件系统重新挂载与交换分区配置（1234–1241 行） ​

1. **重挂载根文件系统为读写并启用写回式配额**
    
    - **日志：**
        
        ```
        EXT4-fs (dm-0): re-mounted 825fc773-fad5-40ab-abed-b3deb608beca r/w. Quota mode: writeback.
        ```
        
    - **做了什么：**  
        `systemd-remount-fs.service` 调用：
        
        ```
        mount -o remount,rw,quota /
        ```
        
        对根文件系统执行重新挂载，并激活配额支持。
        
    - **调用链：**  
        用户态 `mount()` → `sys_mount()` → `do_mount()` → `vfs_remount()` → `ext4_remount()`（`fs/ext4/super.c`）。
        
    - **术语：**  
        写回式配额（writeback quota）：对用户/组磁盘使用量的统计异步写入，性能较高但可能有少量不一致。
        
2. **添加交换文件并分配 swap 空间**
    
    - **日志：**
        
        ```
        Adding 2000892k swap on /swap.img.  Priority:-2 extents:16 across:4325376k SS
        ```
        
    - **做了什么：**  
        用户态 `swapon /swap.img` → 内核 `sys_swapon()` → `add_swap_area()` → `swapfile_set_up()`。
        
    - **术语：**
        
        - `2000892k`：交换空间大小约 1.9 GiB
            
        - `Priority -2`：swap 文件优先级
            
        - `extents:16`：文件被拆分为 16 段
            
        - `across:4325376k SS`：跨越约 4.1 GiB 分散区块
            
3. **Loopback 设备容量检测**
    
    - **日志：**
        
        ```
        loop0: detected capacity change from 0 to 121944
        loop1: detected capacity change from 0 to 79224
        loop2: detected capacity change from 0 to 1192
        ```
        
    - **做了什么：**  
        `loop_set_fd()` 绑定文件到 `/dev/loopX` → `loop_set_status()` → `loop_set_capacity()` → `set_capacity()` → `blk_set_capacity()`。
        
    - **术语：**  
        扇区数（512 B）：121 944≈60 MiB，79 224≈38.7 MiB，1 192≈0.58 MiB。
        

### VMware 虚拟化通信接口 (VMCI) 初始化（1242–1246 行） ​

1. **启用 PCI 设备**
    
    - **日志：**
        
        ```
        vmw_vmci 0000:00:00.7: enabling device (0000 -> 0002)
        ```
        
    - **动作为何：**  
        在 `vmci_pci_probe()` 中调用 `pci_enable_device()`，将命令寄存器的 I/O/MMIO 位从 `0000` 设置到 `0002`。
        
2. **MMIO 寄存器访问就绪**
    
    - **日志：**
        
        ```
        vmw_vmci 0000:00:00.7: MMIO register access is available
        ```
        
    - **动作为何：**  
        驱动检测到 BAR 映射已完成（`pci_iomap()`），可以通过 `ioread32()`/`iowrite32()` 访问。
        
3. **读取并使用能力标志**
    
    - **日志：**
        
        ```
        vmw_vmci 0000:00:00.7: Using capabilities 0x2c
        ```
        
    - **含义：**  
        0x2c 功能位掩码，指示支持的通道与事件通知，由 `vmci_get_caps()` 解析。
        
4. **初始化“客体特性”**
    
    - **日志：**
        
        ```
        Guest personality initialized and is active
        ```
        
    - **动作为何：**  
        在共享内存区域写入客户机特性并激活，由 `vmci_init_guest_personality()` 完成。
        
5. **注册字符设备**
    
    - **日志：**
        
        ```
        VMCI host device registered (name=vmci, major=10, minor=122)
        ```
        
    - **动作为何：**  
        调用 `register_chrdev_region()` 和 `cdev_add()`，在 `/dev/vmci` 下创建字符设备，major=10，minor=122。

### VMware 图形驱动 (vmwgfx) 与媒体子系统（1247 – 1316 行） ​

##### 1. Linux 媒体接口注册 
```text
mc: Linux media interface: v0.10
```

- 动作为何：在内核启动早期，`media_device_init()`（drivers/media/media-core/media-device.c）被调用，注册媒体控制器核心框架（Media Controller API v0.10），为视频捕获、输出子设备提供统一的抽象层。
    
- 术语：Media Controller —— Linux 中对复合视频设备（摄像头 + 编解码器）的管理架构。
    

##### 3. vmwgfx DRM 驱动注册

##### 4. 映射 MMIO 寄存器

text

复制编辑

`vmwgfx 0000:00:0f.0: [drm] Register MMIO at 0x3d000000 size is 4096 kiB`

- 动作为何：PCI 探测回调 `vmwgfx_pci_probe()` 中先后调用 `pci_request_regions()` 然后 `pci_iomap()`，把 BAR0 映射到内核虚拟地址空间，用于后续命令队列和寄存器访问。
    
- 源码路径：drivers/gpu/drm/vmwgfx/vmwgfx_drv.c → `vmwgfx_pci_probe()` → `drm_pci_init()`。
    

##### 3. VRAM（显存）映射

text

复制编辑

`vmwgfx 0000:00:0f.0: [drm] VRAM at 0x70000000 size is 131072 kiB`

- 动作为何：驱动通过读取 PCI BAR2，调用 `drm_add_agp_mapping()` 或 `drm_add_memory_region()`，将 VM 的显存缓冲区映射进内核，大小约 128 MiB。
    

##### 5. 检测 SVGA 版本

text

复制编辑

`vmwgfx 0000:00:0f.0: [drm] Running on SVGA version 3.`

- 含义：查询设备寄存器中的版本号，确认支持哪版 VMware SVGA 接口，用于选择命令格式和功能集。
    

##### 7. 输出各类硬件能力

text

复制编辑

`… Capabilities: gbobject, dx, hp cmd queue, no bb restriction, cap2 register, size 4mb, dx3, frame type, …   DMA map mode: Caching DMA mappings.   Legacy memory limits: VRAM = 4096 kB, FIFO = 256 kB, surface = 524288 kB   MOB limits: max mob size = 262144 kB, max mob pages = 65536`

- 动作为何：驱动依次调用 `vmw_device_query_capset()`、`vmw_device_dma_set_mode()`、`vmw_svu_setup_legacy_caps()`、`vmw_svu_setup_mob_caps()`，将各种功能位和限额报告到 `drm_info()`。
    
- 术语
    
    - **DMA map mode**：决定是否使用可缓存 DMA 窗口，以提高读写效率。
        
    - **Legacy memory limits**：SVGA2 时代的显存划分限制（VRAM，命令 FIFO，表面缓冲）。
        
    - **MOB (Memory Object Buffer)**：SVGA3 引入，用于大块显存对象管理。