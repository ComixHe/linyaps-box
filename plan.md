# linyaps-box `create` + `start` 子命令实现计划

## 参考方案

参考 youki 的 notify.sock 模式（而非 runc/crun 的 exec.fifo）：

```
create → clone/fork → init 进程做完全部设置后 accept(notify.sock) 阻塞
create 命令写 CREATED 状态后退出
start 命令 → connect(notify.sock) → 写 START_CONTAINER → 跑 poststart hooks → 退出
start 不阻塞、不监控、不做 I/O 转发
```

## 涉及文件

| 操作 | 文件 |
|------|------|
| 修改 | `src/linyaps_box/container.h` |
| 修改 | `src/linyaps_box/container.cpp` |
| 修改 | `src/linyaps_box/command/options.h` |
| 修改 | `src/linyaps_box/command/app.cpp` |
| 修改 | `CMakeLists.txt` |
| 新增 | `src/linyaps_box/command/create.h` |
| 新增 | `src/linyaps_box/command/create.cpp` |
| 新增 | `src/linyaps_box/command/start.h` |
| 新增 | `src/linyaps_box/command/start.cpp` |

## 实现步骤

### Step 1 — 数据结构层

#### 1.1 `run_container_options_t` 新增 notify_listener

文件：`src/linyaps_box/container.h`

```cpp
struct run_container_options_t
{
    int preserve_fds;
    std::optional<unix_socket> console_socket;
    std::optional<socket> notify_listener;  // 新增
};
```

说明：`notify_listener` 是已 bind+listen 的 Unix domain socket (SOCK_STREAM)，由 create 命令创建、传给 container::create、再透传到 clone_fn_args。子进程 clone 后继承 fd，在其上 accept 阻塞等 start。

#### 1.2 `clone_fn_args` 新增 notify_listener

文件：`src/linyaps_box/container.cpp`（匿名 namespace 内）

```cpp
struct clone_fn_args {
    int preserve_fds;
    container *container;
    unix_socket socket;
    std::optional<unix_socket> console_socket;
    std::optional<socket> notify_listener;  // 新增
};
```

#### 1.3 `container` 新增 `child_pid()` 访问器

文件：`src/linyaps_box/container.h`

```cpp
// public:
[[nodiscard]] auto child_pid() const noexcept { return child_pid_; }
```

供 create 命令查看子进程 PID（写状态用，尽管 PID 已在 create 内部写入）。

#### 1.4 `options.h` 新增 create/start 选项结构体

文件：`src/linyaps_box/command/options.h`

```cpp
struct create_options
{
    std::string ID;
    std::filesystem::path bundle;
    std::filesystem::path config;
    int preserve_fds{ 0 };
    std::optional<std::string> console_socket;
};

struct start_options
{
    std::string ID;
    std::optional<std::string> console_socket;
};
```

字段含义：
- `ID`：容器标识
- `bundle`：OCI bundle 目录路径
- `config`：config.json 路径（可选，默认 bundle/config.json）
- `preserve_fds`：保留的 fd 数量
- `console_socket`：外部 Unix socket 路径，用于接收 PTY master FD

---

### Step 2 — 内部管道层

#### 2.1 `start_container_process()` 透传 notify_listener

文件：`src/linyaps_box/container.cpp`

将 `options.notify_listener` 从 `run_container_options_t` 传递到 `clone_fn_args`：

```cpp
clone_fn_args args = {
    options.preserve_fds,
    &container,
    std::move(sockets.second),
    std::move(options.console_socket),
    std::move(options.notify_listener)  // 新增
};
```

注意：`options.notify_listener` 被 move 后变为 nullopt。create 命令不再持有该 socket。

#### 2.2 `clone_fn` 分岔等待路径

文件：`src/linyaps_box/container.cpp`

现有代码在发送 `CONTAINER_CREATED` 后从 sync_socket 读 `START_CONTAINER`。修改为：

```cpp
socket << std::byte(sync_message::CONTAINER_CREATED);

if (args.notify_listener) {
    // ---------- create/start 模式 ----------
    // sync_socket 不再需要（parent 已读完 CONTAINER_CREATED 后会退出）
    socket.close();

    // accept 等待外部 start 命令连接
    auto conn = args.notify_listener->accept();
    args.notify_listener->close(); // 关闭 listen socket

    std::byte start_byte;
    conn >> start_byte;
    conn.close();
    if (start_byte != std::byte(sync_message::START_CONTAINER)) {
        throw unexpected_sync_message(sync_message::START_CONTAINER, sync_message(start_byte));
    }
} else {
    // ---------- run 模式（现有路径不变） ----------
    std::byte start_byte;
    socket >> start_byte;
    if (start_byte != std::byte(sync_message::START_CONTAINER)) {
        throw unexpected_sync_message(sync_message::START_CONTAINER, sync_message(start_byte));
    }
}

// 以下代码两模式共享：
start_container_hooks(container, status, socket);
execute_process(oci_config);
```

完整时序（create/start 模式）：

```
parent (create 命令)                  child (init 进程)                 start 命令
│                                     │                                │
├─ clone() ──────────────────────────→│                                │
│  (options.notify_listener 传下去)   │                                │
│                                     ├─ initialize_container          │
│                                     │  (sync socket 协调)            │
├─ configure_ns/hooks ─── sync ──────→│                                │
│                                     ├─ configure_mounts              │
│                                     ├─ do_pivot_root                 │
│                                     ├─ set_capabilities              │
│                                     ├─ unblock signals               │
│                                     ├─ send CONTAINER_CREATED (sync) │
│  ←── 收到 CONTAINER_CREATED ────────│                                │
│                                     ├─ close(sync_socket)            │
│                                     ├─ accept(notify.sock) ←── 阻塞 ─│
│ 写 CREATED 状态，退出                │                                │
│                                     │                                │
│                                     │                        connect(notify.sock)
│                                     │                        write START_CONTAINER
│                                     │                                │
│                                     ├── accept 返回                  │
│                                     ├── read START_CONTAINER         │
│                                     ├── close(conn)                  │
│                                     ├── start_container_hooks        │
│                                     ├── execvpe                      │
│                                     │                        poststart hooks
│                                     │                        退出
```

---

### Step 3 — `container::create()` 调整

文件：`src/linyaps_box/container.cpp`

当前 `create()` 在收到子进程 `CONTAINER_CREATED` 后直接返回。这个行为在 notify 模式下无需改变——因为子进程已经在 accept(notify.sock) 上阻塞了，parent 不需要发 START_CONTAINER。

关键点：`notify_listener` 的创建在 create 命令层（command/create.cpp），不侵入 container 类。`create()` 方法只需要透传 options.notify_listener 到 start_container_process。

create() 的改动量：
- 调用 `start_container_process` 时 options.notify_listener 已被设置，透传即可
- 子进程在 CONTAINER_CREATED 后会走新的 accept 路径

---

### Step 4 — create 命令

文件：`src/linyaps_box/command/create.cpp`

```cpp
auto linyaps_box::command::create(
    const struct create_options &options,
    const global_options &global) -> int
{
    // 1. 创建 runtime + container 对象（同 run 命令）
    status_directory_manager mgr(global.root);
    runtime_t runtime(std::move(mgr));
    create_container_options_t create_opts{
        global.manager, options.ID, options.bundle, options.config
    };
    auto container = runtime.create_container(create_opts);

    // 2. 验证 config
    const auto &cfg = container.get_config();
    if (!cfg.process || !cfg.root) {
        throw std::runtime_error("'process' and 'root' are required");
    }

    // 3. 创建 notify.sock
    //    路径: <status_dir>/<ID>/notify.sock
    auto notify_sock_path = /* status_dir path */ / "notify.sock";
    ::unlink(notify_sock_path.c_str());  // 清除残留

    socket notify_sock(socket::Domain::Unix, socket::Type::Stream);
    auto addr = socketAddress::unix(notify_sock_path.native());
    notify_sock.bind(addr);
    notify_sock.listen();

    // 4. 构建 run_options
    run_container_options_t run_options;
    run_options.preserve_fds = options.preserve_fds;
    run_options.notify_listener = std::move(notify_sock);

    if (cfg.process->terminal && options.console_socket) {
        run_options.console_socket = unix_socket::connect(*options.console_socket);
    }
    // terminal 模式未传 --console-socket：disabled（头尾模式）
    // 不创建内部 console_recv_socket_，子进程不会做 PTY 设置

    // 5. create 容器（clone + 设置，子进程阻塞在 notify.sock）
    container.create(std::move(run_options));

    // 6. 状态已在 create() 内写为 CREATED，直接退出
    return 0;
}
```

获取 status dir 路径的方式：

```cpp
// container 有 status_dir() 但返回 const ref，且是 private
// 需要通过容器 ID 从 status_directory_manager 获取
```

问题：`container` 的 `status_dir()` 是 `protected`（在 `container_ref` 中），对外不可见。

解决方案：create 命令直接构造 `notify.sock` 路径：

```cpp
auto root_path = global.root / options.ID / "notify.sock";
```

`global.root` 是状态目录根路径（默认 `/run/user/1000/linglong/box`），`<root>/<ID>/` 是容器状态目录。

---

### Step 5 — start 命令

文件：`src/linyaps_box/command/start.cpp`

```cpp
auto linyaps_box::command::start(
    const struct start_options &options,
    const global_options &global) -> int
{
    // 1. 打开 runtime，查找容器
    status_directory_manager mgr(global.root);
    runtime_t runtime(std::move(mgr));
    const auto &containers = runtime.containers();
    auto it = containers.find(options.ID);
    if (it == containers.end()) {
        throw std::runtime_error("container not found: " + options.ID);
    }

    // 2. 验证状态为 CREATED
    auto current_status = it->second.status();
    if (current_status.status != container_status_t::runtime_status::CREATED) {
        throw std::runtime_error("container is not in CREATED state");
    }

    // 3. 连接 notify.sock
    auto notify_sock_path = global.root / options.ID / "notify.sock";
    socket start_sock(socket::Domain::Unix, socket::Type::Stream);
    start_sock.connect(socketAddress::unix(notify_sock_path.native()));

    // 4. 发送 START_CONTAINER
    start_sock << std::byte(static_cast<std::uint8_t>(/* START_CONTAINER */));

    // 5. 运行 poststart hooks
    //    需从 status dir 读 config.json
    //    复用 container_ref 的 config() 方法或直接解析
    auto config = /* 从状态目录读取 */;
    if (config.hooks && config.hooks->poststart) {
        for (const auto &hook : *config.hooks->poststart) {
            execute_hook(hook, current_status);
        }
    }

    // 6. 退出（不阻塞、不监控）
    return 0;
}
```

注意：`sync_message` enum 定义在 `container.cpp` 的匿名 namespace 中，外部不可见。
需要将 `START_CONTAINER` 的值暴露给命令层。

解决方案：在 `container.cpp` 末尾或一个公共头中定义一个常量：

```cpp
// 在 container.cpp 尾部或 container.h
constexpr std::uint8_t START_CONTAINER_BYTE = static_cast<std::uint8_t>(sync_message::START_CONTAINER);
```

或者在命令层直接硬编码（不推荐）。

最佳方案：在 `container.h` 中声明 `sync_message` 为公共 enum，仅暴露 `START_CONTAINER`。

实际上，鉴于 `sync_message` 是 container 的内部细节，更简单的方式是让 `start_sock << std::byte{...}` 直接使用正确的字节值。但这样脆弱。

折中方案：在 `container.h` 声明一个常量（不暴露整个 sync_message）：

```cpp
// container.h 中
inline constexpr std::uint8_t notify_start_message = 10; // sync_message::START_CONTAINER 的值
```

这样外部命令层知道用 `std::byte(notify_start_message)` 发送即可。

---

### Step 6 — CLI 注册

#### 6.1 `app.cpp` 注册子命令

文件：`src/linyaps_box/command/app.cpp`

```cpp
// 新增 include
#include "linyaps_box/command/create.h"
#include "linyaps_box/command/start.h"

// 在 parse 函数中注册
app.add_subcommand("create", "Create a container")
    ->callback([&]() {
        // parse create_options
    });

app.add_subcommand("start", "Start a container")
    ->callback([&]() {
        // parse start_options
    });
```

需要查看现有 `app.cpp` 的 CLI11 注册模式以保持一致。

#### 6.2 `CMakeLists.txt` 添加源文件

```cmake
# 找到现有源文件列表，追加
src/linyaps_box/command/create.cpp
src/linyaps_box/command/start.cpp
```

---

### Step 7 — 边界处理

| 事项 | 处理 |
|------|------|
| terminal 模式 | create+start 必须传 `--console-socket`，不传则禁用 terminal；`run` 命令不受影响 |
| cleanup / poststop | start 不清理（状态目录残留），`run` 命令照旧全管 |
| 信号转发 / I/O | start 不做，容器 init 进程成为孤儿被 PID 1 收养 |
| notify.sock 残留 | create 前 unlink 旧文件 |
| 多个 start 调用 | notify.sock accept 一次后关闭，第二个 start 报连接拒绝 |
| 子进程 accept 失败 | 已有异常处理逻辑，向 sync_socket 发 ERROR |
| 非法状态 | start 校验状态为 CREATED，否则报错 |
| 容器不存在 | start 查 ID 失败时报错 |

## 与现有 `run` 命令的关系

```
ll-box run   = container.create() + container.start()   [完整生命周期，阻塞等退出]
ll-box create = container.create() + (notify.sock accept)  [子进程阻塞等 start，不监控]
ll-box start  = connect(notify.sock) + poststart hooks     [触发子进程继续，退出]
```

`run` 命令不受影响，`container::start()` 方法（sync_socket 路径）仅为 `run` 服务。
`create` / `start` 命令共用 `container::create()` 方法，通过 `notify_listener` 参数分岔。

## 实现顺序

1. `container.h` — 数据结构修改（options + args + 常量 + 访问器）
2. `container.cpp` — clone_fn 分岔 + start_container_process 透传
3. `command/options.h` — create/start 选项结构体
4. `command/create.h` + `command/create.cpp`
5. `command/start.h` + `command/start.cpp`
6. `command/app.cpp` — CLI 注册
7. `CMakeLists.txt` — 添加源文件
8. 构建验证