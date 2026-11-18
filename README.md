# MaixCAM Pro MaixCDK Hello World 模板

这是面向校内赛新生的最小 Linux/WSL2 + MaixCDK C++ 工程。它对应教程第 6、7、8 章：在 WSL2 Ubuntu 22.04 中安装 MaixCDK，交叉编译出 MaixCAM Pro 可执行文件，上传到设备运行，再打包成可安装应用。

代码只依赖 MaixCDK 的 `basic` 组件，第一次编译更容易成功。程序启动后会打印两行 Hello World 信息、应用 id 和每秒运行时间，10 秒后自动退出；这可以同时验证 RISC-V 交叉编译、动态库查找、SSH/SCP 上传和正常退出。

## 1. 你将得到什么

```text
rm_hello_world/
├── app.yaml                  # 应用 id、名称、版本和打包信息
├── main/
│   ├── CMakeLists.txt        # 源文件、头文件和组件依赖
│   ├── Kconfig               # 预留配置入口
│   ├── include/main.h        # 共享头文件入口
│   └── src/main.cpp          # Hello World 主程序
├── .gitignore
├── LICENSE
└── README.md
```

`build/`、`dist/` 和根目录 `CMakeLists.txt` 都是 MaixCDK 生成物，已加入 `.gitignore`，不要提交到 Git。

## 2. 先确认设备和 WSL

### 2.1 Windows 安装 WSL2 Ubuntu 22.04

以管理员身份打开 PowerShell：

```powershell
wsl --install -d Ubuntu-22.04
wsl -l -v
```

确认 `Ubuntu-22.04` 的 `VERSION` 是 `2`。如果已经安装，可以直接进入：

```powershell
wsl -d Ubuntu-22.04
```

MaixCAM Pro 与 MaixCAM 使用同一颗 SG2002，MaixCDK 构建平台统一写成 `maixcam`。

### 2.2 推荐的网络和 PATH 设置

如果使用 Windows 11 22H2 或更新版本，可以在 Windows 文件 `%UserProfile%\\.wslconfig` 中写入：

```ini
[wsl2]
networkingMode=mirrored
memory=8GB
```

在 Ubuntu 中执行：

```bash
sudo tee -a /etc/wsl.conf >/dev/null <<'EOF'
[interop]
appendWindowsPath = false
EOF
```

然后在 PowerShell 执行一次：

```powershell
wsl --shutdown
```

重新打开 Ubuntu。镜像网络让 WSL 更容易访问 MaixCAM Pro 的 USB 网卡 IP；`appendWindowsPath=false` 可以避免 Windows 路径中的括号干扰部分第三方构建脚本。内存值按自己的电脑调整，编译 OpenCV 示例时再适当增加。

### 2.3 把工程放在 WSL Linux 文件系统

推荐放在 `~/rm_projects`，不要放在 `/mnt/c` 下编译：

```bash
mkdir -p ~/rm_projects
cd ~/rm_projects
# 将本仓库复制或克隆到这里，例如：
# git clone <本仓库地址> rm_hello_world
cd rm_hello_world
```

如果要在 VS Code 中编辑：

```bash
code .
```

请安装 VS Code 的 **WSL** 扩展，并确认左下角显示已连接到 Ubuntu。

## 3. 安装 MaixCDK

在 Ubuntu 终端执行：

```bash
sudo apt update
sudo apt install -y git cmake build-essential python3 python3-pip python-is-python3 autoconf automake libtool
cmake --version                 # 教程要求 >= 3.13

cd ~
git clone https://github.com/sipeed/MaixCDK
# GitHub 下载慢时可改用：
# git clone https://gitee.com/Sipeed/MaixCDK

cd ~/MaixCDK
python3 -m pip install -U pip
python3 -m pip install -U -r requirements.txt -i https://pypi.tuna.tsinghua.edu.cn/simple

echo 'export PATH="$HOME/.local/bin:$PATH"' >> ~/.bashrc
echo 'export MAIXCDK_PATH="$HOME/MaixCDK"' >> ~/.bashrc
source ~/.bashrc
maixcdk -h
```

最后一条命令能显示帮助就说明命令已安装。`MAIXCDK_PATH` 允许工程放在 `~/MaixCDK` 之外；本模板就是独立工程。

## 4. 第一次编译（生成 MaixCAM Pro 的 RISC-V 程序）

在本仓库根目录执行：

```bash
maixcdk build -p maixcam
```

第一次编译会下载 MaixCDK 工具链和平台依赖，可能需要较长时间。不要省略 `-p maixcam`：不加平台时默认选项可能是给电脑运行的 `linux`，而不是给 MaixCAM Pro 的 RISC-V 目标。

下载慢时可以在 WSL 中设置代理（端口按本机软件修改）：

```bash
export http_proxy=http://127.0.0.1:7890
export https_proxy=http://127.0.0.1:7890
maixcdk build -p maixcam
```

编译成功后检查产物：

```bash
ls -la build/
file build/rm_hello_world
ls -la build/dl_lib/
```

`file` 的结果应显示 RISC-V 目标（通常包含 `UCB RISC-V` 或 `riscv64`）。如果显示 `x86-64`，说明平台选错了，请删除 `build/` 后重新执行 `maixcdk build -p maixcam`。

后续只改了源代码时可使用增量构建：

```bash
maixcdk build2 -p maixcam
```

## 5. 连接 MaixCAM Pro 并查看 IP

设备与电脑连接有两种方式：

1. **USB 网卡**：用支持数据的 Type-C 线连接，Windows 网络适配器里通常会出现一个 USB 网卡。教程示例 IP 是 `10.131.167.1`，实际地址以设备信息页为准。
2. **Wi-Fi**：让设备和电脑加入同一个网络，在设备的网络信息里查看 IP。

先测试 SSH：

```bash
ssh root@<设备IP>
```

默认用户名和密码都是 `root`（以当前系统版本为准）。第一次连接输入 `yes` 接受主机密钥。若刷机后出现 `REMOTE HOST IDENTIFICATION HAS CHANGED`：

```bash
ssh-keygen -R <设备IP>
```

设备端可以用下面命令确认架构和空间：

```bash
uname -m
free -m
df -h
```

`uname -m` 应为 RISC-V 相关架构；WSL 中执行 `uname -m` 则通常是 `x86_64`，两者不同正是交叉编译存在的原因。

## 6. 上传并运行 Hello World

把 `<设备IP>` 替换成实际地址。在 WSL 工程根目录执行：

```bash
ssh root@<设备IP> "mkdir -p /root/rm_hello_world"
scp build/rm_hello_world build/dl_lib root@<设备IP>:/root/rm_hello_world/
ssh root@<设备IP>
```

进入设备后：

```bash
cd /root/rm_hello_world
LD_LIBRARY_PATH=./dl_lib ./rm_hello_world
```

你应看到类似输出：

```text
Hello World!
Hello from MaixCAM Pro via MaixCDK.
app_id: rm_hello_world
running for 0 s
running for 1 s
...
Program exit.
```

`LD_LIBRARY_PATH=./dl_lib` 不能省略。MaixCDK 默认动态链接，`dl_lib` 中放着设备端需要的 `.so` 文件。程序运行时按 `Ctrl+C` 也可以提前退出；不要直接拔电或关闭终端，以便 MaixCDK 正常释放资源。

也可以不进入交互式 SSH，直接远程运行：

```bash
ssh root@<设备IP> "cd /root/rm_hello_world && LD_LIBRARY_PATH=./dl_lib ./rm_hello_world"
```

## 7. 修改代码并重复验证

打开 `main/src/main.cpp`，例如把：

```cpp
log::info("Hello World!");
```

改成：

```cpp
log::info("Hello RM algorithm contest!");
```

然后循环执行：

```bash
maixcdk build2 -p maixcam
scp build/rm_hello_world build/dl_lib root@<设备IP>:/root/rm_hello_world/
ssh root@<设备IP> "cd /root/rm_hello_world && LD_LIBRARY_PATH=./dl_lib ./rm_hello_world"
```

这就是后续视觉、串口、IMU 练习的基本开发循环：改代码 → 交叉编译 → 上传 → 在板端验证。

## 8. 打包成设备应用（对应教程第 8 章）

临时调试使用上面的目录运行即可；需要在设备应用列表中安装时执行：

```bash
maixcdk release -p maixcam
ls -lh dist/
scp dist/*.zip root@<设备IP>:/root/
ssh root@<设备IP> "app_store_cli install /root/rm_hello_world_v1.0.0.zip"
```

安装后回到设备应用列表应能看到“校内赛 Hello World 模板”。如果系统支持应用商店扫码，也可以在工程目录执行：

```bash
maixcdk deploy
```

设备端应用名称和可执行文件名来自 `app.yaml` 的 `id`。`id` 只能使用小写字母、数字和下划线；同一设备不能同时安装两个相同 id 的应用。

## 9. 常见问题

### `maixcdk: command not found`

检查当前 shell 是否加载了 `~/.local/bin` 和 `MAIXCDK_PATH`：

```bash
source ~/.bashrc
echo "$PATH"
echo "$MAIXCDK_PATH"
python3 -m pip show maixcdk
```

### 编译时选成了 `linux` 或产物是 x86-64

删除生成物并明确指定平台：

```bash
rm -rf build dist CMakeLists.txt
maixcdk build -p maixcam
```

### `cannot open shared object file`

确认上传了整个 `build/dl_lib` 目录，并在设备端使用：

```bash
LD_LIBRARY_PATH=./dl_lib ./rm_hello_world
```

### SSH 能连上但 `scp` 失败

确认目标目录存在、设备有剩余空间：

```bash
ssh root@<设备IP> "mkdir -p /root/rm_hello_world; df -h"
```

### 设备提示主机密钥变化

刷写系统或更换设备后，删除旧密钥再连接：

```bash
ssh-keygen -R <设备IP>
```

### camera 示例提示资源被占用

本模板不访问摄像头，因此不需要处理 launcher 抢占问题。进入教程第 7 章的 `camera_display` 或第 8 章视觉模板时，先用 MaixVision 连接一次让 launcher 退出，或在设备端查找并结束 launcher 进程；退出程序要使用 `Ctrl+C`。

## 10. 从这里继续扩展

- 加摄像头和屏幕：在 `main/CMakeLists.txt` 增加 `vision` 等依赖，参考 MaixCDK `examples/camera_display`。
- 加串口：增加 `peripheral`，参考教程第 8 章 UART1 的 `A19/A18` 引脚配置。
- 加 IMU/AHRS：增加 `ext_dev`、`ahrs`，第一次校准时保持设备静止。
- 加 OpenCV：增加 `opencv`，并继续关注 MaixCAM Pro 只有 256 MB 内存这一约束。

每次增加组件后，都先让最小改动成功编译，再一次只加入一个硬件模块，便于定位问题。

## 许可证

本项目采用 MIT License，见 `LICENSE`。