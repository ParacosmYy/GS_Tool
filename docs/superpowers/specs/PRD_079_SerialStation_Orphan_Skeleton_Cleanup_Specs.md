# Specs - PRD-079 Serial Station Orphan Skeleton Cleanup

## 1. 范围

删除以下未引用、未进 CMake 的重复骨架：

- `src/apps/serial_station/app.cpp`
- `src/apps/serial_station/app.h`
- `src/apps/serial_station/config.h`
- `src/apps/serial_station/constants.h`
- `src/apps/serial_station/models.h`
- `src/apps/serial_station/window.h`

## 2. 依据

- 外部 include 搜索未发现业务引用。
- CMake 未引用这些小写文件。
- canonical 文件已存在并加入 CMake。

## 3. 验收

- [ ] `ctest --test-dir build -R "AsciiTextProtocol|SerialProtocolRegistry|SerialSession" --output-on-failure` 通过。
- [ ] `cmake --build build --target EmbedDebug --parallel 4` 通过或无工作。
- [ ] `EmbedDebug.bat` 可启动。
- [ ] `tools/source-tree-audit.ps1` 报告不再列出已删除小写文件。

## 4. BATCH 判定

- 是否需要 BATCH：否。
- 并行度上限：1。
- 原因：删除同一目录下的重复骨架，范围小，串行更清晰。

