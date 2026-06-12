# PRD-078 - CMake Generated Utils Slimming

## 背景

PRD-076 的审计显示：

- 当前工作区 `src/` 下源码约 82026 个。
- `src/utils/` 占 81117 个。
- CMake 文本引用约 6078 个，其中 `utils` 约 5237 个。
- 主目标全量构建在当前机器上多次超时。

`CMakeLists.txt` 已经有一条过滤规则，注释说明“批量算法草稿目录存在大量重复 QObject 类名，不能直接进入主 GUI 目标”。但现有规则只排除了：

```cmake
^src/utils/(cluster|code|dsp|fft|graph|matrix|signal|tree)[0-9]+/
```

仍有大量 `src/utils/<目录名含数字>/` 的算法草稿目录进入主 GUI 目标，例如 `aes2`、`automata2`、`btree5`、`cache2`、`conv3`、`heap8`、`linalg6`、`segment_tree2` 等。

## 目标

1. 扩展 CMake 过滤规则，把所有 `src/utils/<目录名含数字>/` 从主 GUI 目标排除。
2. 不删除源码，只让批量算法草稿不参与主 GUI 构建。
3. 保留 canonical 工具目录，例如 `settings`、`log`、`export`、`data`、`crypto`、`converter`、`checksum`、`packet`、`perf`、`pipeline`、`timestamp` 等。
4. 保持 Serial Station 新增测试和 bat 启动链路可用。

## 非目标

1. 不删除 `src/utils/` 下任何文件。
2. 不移动目录。
3. 不重写 CMake 结构。
4. 不改变 UI 行为。
5. 不处理非数字目录的技术债。

## 安全依据

1. 这些目录已被现有注释归类为“批量算法草稿”。
2. 现有主代码未发现 include `utils/<目录名含数字>/...` 的外部依赖。
3. 本轮只改 CMake 过滤规则，失败可单点回退。
4. 新增 Serial Station QTest 不依赖这些目录。

## 验收标准

1. CMake 配置通过。
2. `ninja -C build -t targets all` 中不再出现被排除的数字 utils 目录对象。
3. Serial Station 3 个 QTest 继续通过。
4. `EmbedDebug.bat` 仍可启动已有 `build/EmbedDebug.exe`。
5. 不创建第二构建目录。

## 后续

如果本轮过滤后主目标仍然构建慢，下一轮继续拆分：

1. 把非必要 feature widget 从主目标拆成可选 target。
2. 把 `utils/` 算法草稿迁移到独立静态库或实验目录。
3. 单独处理旧小写 `src/apps/serial_station/` 残留骨架。
