# TODO - OCI Runtime Spec v1.3.0 合规性

本文档跟踪 OCI Runtime Specification v1.3.0 合规性所需的功能。
基于代码库分析和 OCI 规范对比。

**图例：**
- **难度**：低 (1-2 天) / 中 (3-5 天) / 高 (1-2 周) / 很高 (2-4 周)
- **工时**：预估开发人天

---

## 优先级 1：核心 OCI 合规性（基础合规必需）

### 生命周期操作

| 功能 | 状态 | 难度 | 工时 | 说明 |
|------|------|------|------|------|
| `create` 命令 | 缺失 | 中 | 3天 | 创建容器但不启动；需要状态持久化 |
| `start` 命令 | 缺失 | 低 | 2天 | 启动已创建的容器 |
| `delete` 命令 | 缺失 | 低 | 1天 | 删除已停止的容器 |
| `state` 命令 | 缺失 | 低 | 1天 | 查询容器状态 |

**当前状态**：仅实现了 `run`、`exec`、`kill`、`list` 命令。
**总工时**：约 7 天
**依赖**：需要状态文件/目录管理来支持容器生命周期

### 状态管理

| 功能 | 状态 | 难度 | 工时 | 说明 |
|------|------|------|------|------|
| 容器状态持久化 | 缺失 | 中 | 3天 | 每个容器存储 state.json |
| 状态目录管理 | 缺失 | 中 | 2天 | 创建/管理 /run/ll-box/<container-id>/ |

**总工时**：约 5 天

### Cgroup 资源限制

| 功能 | 状态 | 难度 | 工时 | 说明 |
|------|------|------|------|------|
| Cgroup v2 管理器 | 缺失 | 高 | 10天 | 现代系统的主要目标 |
| Cgroup v1 管理器 | 缺失 | 高 | 8天 | 旧系统支持 |

**内存资源** (`linux.resources.memory`)：
| 字段 | 状态 | 难度 | 工时 | 说明 |
|------|------|------|------|------|
| `limit` | 缺失 | 低 | 0.5天 | 内存限制 |
| `reservation` | 缺失 | 低 | 0.5天 | 软限制 |
| `swap` | 缺失 | 低 | 0.5天 | 交换分区限制 |
| `kernel` | 缺失 | 低 | 0.5天 | 仅 cgroup v1，规范中标记为不推荐 |
| `kernelTCP` | 缺失 | 低 | 0.5天 | 仅 cgroup v1，规范中标记为不推荐 |
| `swappiness` | 缺失 | 低 | 0.5天 | 交换倾向参数 |
| `disableOOMKiller` | 缺失 | 低 | 0.5天 | OOM 杀手控制 |
| `useHierarchy` | 缺失 | 低 | 0.5天 | cgroup v2 层次结构 |
| `checkBeforeUpdate` | 缺失 | 低 | 0.5天 | cgroup v2 内存限制检查 |

**CPU 资源** (`linux.resources.cpu`)：
| 字段 | 状态 | 难度 | 工时 | 说明 |
|------|------|------|------|------|
| `shares` | 缺失 | 低 | 0.5天 | CPU 权重 |
| `quota` | 缺失 | 低 | 0.5天 | CPU 配额 |
| `burst` | 缺失 | 低 | 0.5天 | cgroup v2 CPU 突发 |
| `period` | 缺失 | 低 | 0.5天 | CPU 周期 |
| `realtimeRuntime` | 缺失 | 中 | 1天 | 实时调度运行时间 |
| `realtimePeriod` | 缺失 | 中 | 1天 | 实时调度周期 |
| `cpus` (cpuset.cpus) | 缺失 | 低 | 0.5天 | CPU 亲和性 |
| `mems` (cpuset.mems) | 缺失 | 低 | 0.5天 | 内存节点 |
| `idle` | 缺失 | 低 | 0.5天 | cgroup v2 CPU 空闲 |

**PID 资源** (`linux.resources.pids`)：
| 字段 | 状态 | 难度 | 工时 | 说明 |
|------|------|------|------|------|
| `limit` | 缺失 | 低 | 0.5天 | 最大进程数限制 |

**块 I/O 资源** (`linux.resources.blockIO`)：
| 字段 | 状态 | 难度 | 工时 | 说明 |
|------|------|------|------|------|
| `weight` | 缺失 | 低 | 0.5天 | 块 IO 权重 |
| `leafWeight` | 缺失 | 低 | 0.5天 | 仅 cgroup v1 CFQ |
| `weightDevice` | 缺失 | 中 | 1天 | 设备级权重 |
| `throttleReadBpsDevice` | 缺失 | 中 | 1天 | 读 BPS 限制 |
| `throttleWriteBpsDevice` | 缺失 | 中 | 1天 | 写 BPS 限制 |
| `throttleReadIOPSDevice` | 缺失 | 中 | 1天 | 读 IOPS 限制 |
| `throttleWriteIOPSDevice` | 缺失 | 中 | 1天 | 写 IOPS 限制 |

**网络资源** (`linux.resources.network`)：
| 字段 | 状态 | 难度 | 工时 | 说明 |
|------|------|------|------|------|
| `classID` | 缺失 | 低 | 0.5天 | cgroup v1 net_cls |
| `priorities` | 缺失 | 中 | 1天 | cgroup v1 net_prio |

**大页内存限制** (`linux.resources.hugepageLimits`)：
| 字段 | 状态 | 难度 | 工时 | 说明 |
|------|------|------|------|------|
| 按页大小限制 | 缺失 | 中 | 2天 | hugetlb 控制器 |

**设备 cgroup 规则** (`linux.resources.devices`)：
| 字段 | 状态 | 难度 | 工时 | 说明 |
|------|------|------|------|------|
| 设备允许/拒绝规则 | 缺失 | 中 | 2天 | |
| allow/type/major/minor/access 支持 | 缺失 | 中 | 1天 | |

**RDMA 资源** (`linux.resources.rdma`)：
| 字段 | 状态 | 难度 | 工时 | 说明 |
|------|------|------|------|------|
| `hcaHandles` | 缺失 | 中 | 1天 | |
| `hcaObjects` | 缺失 | 中 | 1天 | |

**统一 cgroup v2** (`linux.resources.unified`)：
| 字段 | 状态 | 难度 | 工时 | 说明 |
|------|------|------|------|------|
| 任意 cgroup v2 参数 | 缺失 | 中 | 2天 | |

**Cgroup 总工时**：约 40 天（包含 v1 + v2 管理器 + 所有资源）

### 自定义设备

| 功能 | 状态 | 难度 | 工时 | 说明 |
|------|------|------|------|------|
| 解析 `linux.devices` | 缺失 | 低 | 1天 | |
| 创建设备节点 (mknod/bind mount) | 缺失 | 中 | 2天 | |
| 设备属性 (type/path/major/minor/fileMode/uid/gid) | 缺失 | 中 | 2天 | |

**当前状态**：仅创建默认设备 (`/dev/null`、`/dev/zero`、`/dev/full`、`/dev/random`、`/dev/urandom`、`/dev/tty`、`/dev/ptmx`)。
**总工时**：约 5 天

---

## 优先级 2：安全与隔离

### Seccomp 支持

| 功能 | 状态 | 难度 | 工时 | 说明 |
|------|------|------|------|------|
| 解析 seccomp 配置 | 缺失 | 中 | 2天 | |
| 应用 seccomp 过滤器 (libseccomp) | 缺失 | 高 | 5天 | |
| `defaultAction` | 缺失 | 中 | 1天 | |
| `defaultErrnoRet` | 缺失 | 低 | 0.5天 | |
| `architectures` | 缺失 | 中 | 1天 | |
| `syscalls` 数组 | 缺失 | 中 | 2天 | |
| `args` 参数匹配 | 缺失 | 高 | 3天 | 复杂的参数匹配逻辑 |
| `flags` (TSYNC/LOG/SPEC_ALLOW/WAIT_KILLABLE_RECV) | 缺失 | 中 | 2天 | |
| `listenerPath` (SCMP_ACT_NOTIFY) | 缺失 | 高 | 4天 | 需要 socket 通信 |
| `listenerMetadata` | 缺失 | 低 | 0.5天 | |
| 容器进程状态（用于 seccomp notify） | 缺失 | 高 | 3天 | |

**当前状态**：存在构建选项 `linyaps-box_ENABLE_SECCOMP` 但无实现。
**总工时**：约 24 天
**依赖**：libseccomp >= 2.3.3

### LSM 支持

| 功能 | 状态 | 难度 | 工时 | 说明 |
|------|------|------|------|------|
| AppArmor (`process.apparmorProfile`) | 缺失 | 中 | 3天 | 需要 aa-exec 或 setprocattr |
| SELinux (`process.selinuxLabel`) | 缺失 | 中 | 3天 | 需要 setexeccon |
| SELinux 挂载标签 (`linux.mountLabel`) | 缺失 | 中 | 2天 | 应用到挂载点 |

**当前状态**：字段已解析但未应用。
**总工时**：约 8 天

### 时间命名空间

| 功能 | 状态 | 难度 | 工时 | 说明 |
|------|------|------|------|------|
| `time` 命名空间类型 | 缺失 | 中 | 3天 | |
| `linux.timeOffsets` (monotonic/boottime) | 缺失 | 中 | 2天 | |

**内核要求**：Linux 5.6+
**总工时**：约 5 天

### I/O 优先级

| 功能 | 状态 | 难度 | 工时 | 说明 |
|------|------|------|------|------|
| `process.ioPriority` (class/priority) | 缺失 | 低 | 1天 | ioprio_set 系统调用 |

**总工时**：约 1 天

### 进程调度器

| 功能 | 状态 | 难度 | 工时 | 说明 |
|------|------|------|------|------|
| `process.scheduler.policy` | 缺失 | 中 | 2天 | sched_setscheduler |
| `process.scheduler.nice` | 缺失 | 低 | 0.5天 | |
| `process.scheduler.priority` | 缺失 | 低 | 0.5天 | |
| `process.scheduler.flags` | 缺失 | 低 | 0.5天 | |
| `process.scheduler.runtime/deadline/period` | 缺失 | 中 | 1天 | SCHED_DEADLINE |

**总工时**：约 4.5 天

### execCPUAffinity

| 功能 | 状态 | 难度 | 工时 | 说明 |
|------|------|------|------|------|
| `process.execCPUAffinity.initial` | 缺失 | 中 | 1天 | cgroup 转换前的 CPU |
| `process.execCPUAffinity.final` | 缺失 | 中 | 1天 | cgroup 转换后的 CPU |

**总工时**：约 2 天

---

## 优先级 3：高级特性

### Intel RDT

| 功能 | 状态 | 难度 | 工时 | 说明 |
|------|------|------|------|------|
| `intelRdt.closID` | 缺失 | 高 | 3天 | resctrl 文件系统 |
| `intelRdt.l3CacheSchema` | 缺失 | 高 | 2天 | L3 缓存分配 |
| `intelRdt.memBwSchema` | 缺失 | 高 | 2天 | 内存带宽分配 |
| `intelRdt.schemata` (v1.3.0) | 缺失 | 高 | 2天 | 新增字段 |
| `intelRdt.enableMonitoring` | 缺失 | 高 | 3天 | 监控支持 |

**要求**：Intel RDT 硬件支持和 resctrl 文件系统。
**总工时**：约 12 天

### 内存策略

| 功能 | 状态 | 难度 | 工时 | 说明 |
|------|------|------|------|------|
| `linux.memoryPolicy.mode` | 缺失 | 中 | 2天 | set_mempolicy |
| `linux.memoryPolicy.nodes` | 缺失 | 中 | 1天 | |
| `linux.memoryPolicy.flags` | 缺失 | 中 | 1天 | |

**总工时**：约 4 天

### 网络设备迁移

| 功能 | 状态 | 难度 | 工时 | 说明 |
|------|------|------|------|------|
| 将接口移动到容器命名空间 | 缺失 | 高 | 3天 | |
| 接口重命名 (`name` 字段) | 缺失 | 中 | 1天 | |
| 保留全局作用域的 IP 地址 | 缺失 | 中 | 1天 | |
| 设置设备状态为 "up" | 缺失 | 低 | 0.5天 | |

**总工时**：约 5.5 天

### 挂载 ID 映射

| 功能 | 状态 | 难度 | 工时 | 说明 |
|------|------|------|------|------|
| 挂载配置中的 `uidMappings`/`gidMappings` | 缺失 | 高 | 4天 | |
| `idmap` 挂载选项 | 缺失 | 高 | 3天 | mount_setattr(2) |
| `ridmap` 递归 ID 映射 | 缺失 | 高 | 2天 | |

**内核要求**：Linux 5.12+
**总工时**：约 9 天

### 域名

| 功能 | 状态 | 难度 | 工时 | 说明 |
|------|------|------|------|------|
| `domainname` 字段 | 缺失 | 低 | 0.5天 | setdomainname |

**总工时**：约 0.5 天

### Sysctl

| 功能 | 状态 | 难度 | 工时 | 说明 |
|------|------|------|------|------|
| `linux.sysctl` 内核参数 | 缺失 | 中 | 2天 | 写入 /proc/sys |

**总工时**：约 2 天

### Personality

| 功能 | 状态 | 难度 | 工时 | 说明 |
|------|------|------|------|------|
| `linux.personality.domain` | 缺失 | 低 | 1天 | personality 系统调用 |
| `linux.personality.flags` | 缺失 | 低 | 0.5天 | |

**总工时**：约 1.5 天

---

## 优先级 4：特性结构（可选）

### 运行时特性查询

| 功能 | 状态 | 难度 | 工时 | 说明 |
|------|------|------|------|------|
| `features` 命令 | 缺失 | 中 | 3天 | 返回支持的特性 |
| `ociVersionMin`/`ociVersionMax` | 缺失 | 低 | 0.5天 | |
| `hooks` 数组 | 缺失 | 低 | 0.5天 | |
| `mountOptions` 数组 | 缺失 | 中 | 1天 | |
| `linux` 特定特性 | 缺失 | 中 | 2天 | |
| `potentiallyUnsafeConfigAnnotations` | 缺失 | 低 | 0.5天 | |

**总工时**：约 7.5 天

---

## 已实现功能修正

基于代码库分析，以下修正适用：

### 已验证实现

- [x] 命名空间：pid、network、mount、ipc、uts、user、cgroup（在 `src/linyaps_box/utils/namespaces.cpp` 中验证）
- [x] 用户命名空间 UID/GID 映射（在 `src/linyaps_box/container.cpp` 中验证）
- [x] 能力集：所有 5 个集合（effective、bounding、inheritable、permitted、ambient）（在 `src/linyaps_box/utils/capabilities.cpp` 中验证）
- [x] 钩子：所有 6 种类型（prestart、createRuntime、createContainer、startContainer、poststart、poststop）（在 `src/linyaps_box/hooks.cpp` 中验证）
- [x] 挂载：bind、tmpfs、proc、sysfs、devpts、mqueue（在 `src/linyaps_box/utils/mount.cpp` 中验证）
- [x] 挂载传播选项：private/shared/slave/unbindable、递归（已验证）
- [x] 掩码路径和只读路径（在 `src/linyaps_box/container.cpp` 中验证）
- [x] 根文件系统（支持只读）（已验证）
- [x] Pivot root（已验证）
- [x] 进程配置：args、env、cwd、user、terminal、consoleSize（在 `src/linyaps_box/config.cpp` 中验证）
- [x] 资源限制 (rlimits)（已验证）
- [x] noNewPrivileges（已验证）
- [x] oomScoreAdj（已验证）
- [x] 主机名 (UTS 命名空间)（已验证）
- [x] 注解 (Annotations)（已验证）
- [x] 默认设备（在 `src/linyaps_box/container.cpp` 中验证）
- [x] /dev/fd、/dev/stdin、/dev/stdout、/dev/stderr 符号链接（已验证）
- [x] rootfsPropagation（已验证）

### 未实现（原列表错误）

以下功能原列表标记为已实现，但实际**未实现**：

- [ ] **时间命名空间**：未实现 - 代码库中无时间命名空间支持
- [ ] **idmap/ridmap 挂载选项**：未实现 - 未发现 mount_setattr 使用
- [ ] **mountLabel (SELinux)**：已解析但未应用到挂载点

### 部分实现

- [ ] **process.apparmorProfile**：已解析但未应用
- [ ] **process.selinuxLabel**：已解析但未应用
- [ ] **linux.sysctl**：可能已解析但未应用

---

## 按优先级汇总

| 优先级 | 类别 | 总工时 | 难度 |
|--------|------|--------|------|
| P1 | 生命周期操作 | 7 天 | 中 |
| P1 | 状态管理 | 5 天 | 中 |
| P1 | Cgroup 资源 | 40 天 | 高 |
| P1 | 自定义设备 | 5 天 | 中 |
| P2 | Seccomp | 24 天 | 高 |
| P2 | LSM 支持 | 8 天 | 中 |
| P2 | 时间命名空间 | 5 天 | 中 |
| P2 | I/O 优先级 | 1 天 | 低 |
| P2 | 进程调度器 | 4.5 天 | 中 |
| P2 | execCPUAffinity | 2 天 | 中 |
| P3 | Intel RDT | 12 天 | 高 |
| P3 | 内存策略 | 4 天 | 中 |
| P3 | 网络设备迁移 | 5.5 天 | 高 |
| P3 | 挂载 ID 映射 | 9 天 | 高 |
| P3 | 域名 | 0.5 天 | 低 |
| P3 | Sysctl | 2 天 | 中 |
| P3 | Personality | 1.5 天 | 低 |
| P4 | 特性结构 | 7.5 天 | 中 |

**预估总工时**：约 144 开发人天（单人约 7 个月）

---

## 建议实现顺序

### 第一阶段：核心 OCI 合规性（必需）

1. 状态管理（create/start/delete/state 命令）
2. 基础 cgroup v2 管理器（内存/CPU 限制）
3. 自定义设备

**预估**：25-30 天

### 第二阶段：安全特性

1. Seccomp 支持（基础）
2. LSM 支持（AppArmor/SELinux）
3. Sysctl

**预估**：35 天

### 第三阶段：资源管理

1. 完整 cgroup 资源支持
2. 进程调度器
3. I/O 优先级

**预估**：20 天

### 第四阶段：高级特性

1. 时间命名空间
2. 挂载 ID 映射
3. 网络设备迁移
4. Intel RDT

**预估**：30 天

---

## 测试

- [ ] OCI 运行时一致性测试套件（runc tests / oci-runtime-tool）
- [ ] Cgroup 限制集成测试
- [ ] Seccomp 集成测试
- [ ] 生命周期操作集成测试

---

## 参考资料

- [OCI Runtime Specification v1.3.0](https://github.com/opencontainers/runtime-spec/blob/v1.3.0/spec.md)
- [Linux cgroups v2 文档](https://www.kernel.org/doc/Documentation/cgroup-v2.txt)
- [seccomp(2) 手册页](https://man7.org/linux/man-pages/man2/seccomp.2.html)
- [capabilities(7) 手册页](https://man7.org/linux/man-pages/man7/capabilities.7.html)
- [namespaces(7) 手册页](https://man7.org/linux/man-pages/man7/namespaces.7.html)
- [mount_setattr(2) 手册页](https://man7.org/linux/man-pages/man2/mount_setattr.2.html)
