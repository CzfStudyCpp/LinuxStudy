
# 要编译的文件列表
tests=(
    "block_remove_test.cpp"
    "block_read_test.cpp"
    "block_write_test.cpp"
    "block_load_stat.cpp"
    "block_init_test.cpp"
)

# 共享的类文件
shared_files="file_op.cpp index_handle.cpp mmap_file.cpp mmap_file_op.cpp"

# 循环遍历测试文件数组并编译每个测试文件
for test in "${tests[@]}"
do
    # 提取文件名（不含扩展名）
    exe="${test%.cpp}"

    # 编译源文件生成可执行文件
    g++ -g "$test" $shared_files -o "$exe"

    # 检查编译是否成功
    if [ $? -eq 0 ]; then
        echo "Successfully compiled $test to $exe"
    else
        echo "Failed to compile $test"
    fi
done
