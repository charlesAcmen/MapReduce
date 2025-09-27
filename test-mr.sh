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
COORD_PID=$!

# Step 4: 启动几个 worker（后台运行）
echo "Starting workers..."
for i in {1..3}; do
    ./worker &
done

# -----------------------------
# Step 4: 等待后台任务结束
wait $COORD_PID   # 等待 coordinator 退出
# 这里可以选择再 wait 其他 worker，或者 detach 模式下直接等待 coordinator

# -----------------------------
# Step 5: 合并 ./mr-out/ 文件并排序（和你之前的 C++ 做法一致）
OUT_DIR=./mr-out
FINAL_OUT=$OUT_DIR/mr-wc-all.txt

# 清理旧文件
rm -f "$FINAL_OUT"

# -----------------------------
# Step 6: 比较参考答案
# 假设参考答案在 ../data/answers/mr-correct-wc.txt
REF="../data/answers/mr-correct-wc.txt"
if [ -f "$REF" ]; then
    echo "Comparing with reference answer..."
    if diff -q "$FINAL_OUT" "$REF" >/dev/null; then
        echo "✅ Output matches reference answer!"
    else
        echo "❌ Output differs from reference answer. Showing diff:"
        diff "$FINAL_OUT" "$REF" || true
    fi
else
    echo "Reference answer not found at $REF, skipping comparison."
fi

# -----------------------------
# Step 7: 脚本自动退出
echo "All done!"
exit 0
