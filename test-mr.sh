#!/bin/bash
# 说明：
# 1. 先进入 build 目录
# 2. 编译项目
# 3. 运行 coordinator
# 4. 运行多个 worker（后台运行）

set -e  # 遇到错误就退出

# Step 1: 创建并进入 build 目录
mkdir -p build
cd build

# Step 2: 运行 cmake 和 make
cmake ..
make -j$(nproc)

# Step 3: 启动 coordinator（前台运行）
echo "Starting coordinator..."
#& means run in background,avoiding blocked by accept() in coordinator
./coordinator &

# Step 4: 启动几个 worker（后台运行）
echo "Starting workers..."
for i in {1..3}; do
    ./worker &
done

# Step 5: 等待所有后台任务结束
wait
