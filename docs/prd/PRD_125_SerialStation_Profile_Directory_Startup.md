# PRD-125 Serial Station 档案目录快捷启动

## 背景

Serial Station 已支持按具体 `.edserialprofile` 启动、恢复上次档案和维护最近档案。企业工位通常会把生产线配置统一放在固定目录中，例如 `profiles/line-a/` 或共享盘目录。当前保存/加载档案对话框仍默认打开系统文档目录，用户每天需要手动定位目录，不符合“打开方便快捷、工站配置齐全”的目标。

## 目标

1. 新增启动参数 `--profile-dir` 和 `--serial-profile-dir`。
2. 参数出现时自动路由到 Serial Station。
3. Serial Station 保存/加载档案对话框默认使用该目录。
4. 若同时提供 `--profile` 或 `--last-profile`，目录参数只影响后续对话框默认位置，不改变显式档案加载优先级。
5. 档案目录路径需要标准化，空值不生效。

## 非目标

1. 不自动扫描目录内所有档案。
2. 不改变最近档案索引格式。
3. 不创建真实生产线档案模板。
4. 不提升真实串口设备验证等级，本轮仍为 D1 自动化测试。

## 用户故事

作为产线调试工程师，我希望通过一个固定脚本进入 Serial Station，并让保存/加载档案对话框直接打开工位配置目录，从而减少重复定位文件的时间。

## 验收标准

1. `EmbedDebug.bat --station serial --profile-dir .\profiles` 能打开 Serial Station。
2. `EmbedDebug.bat --profile-dir .\profiles` 未指定 station 时也能自动路由到 Serial Station。
3. `StartupOptions` 能解析 flag 与 equals 两种目录写法。
4. `SerialStationWindow` 能设置、读取并标准化默认档案目录。
5. 显式 `--profile` 优先加载具体档案，目录参数不覆盖档案路径。

## 三轴目标

```text
Engineering: E5，启动解析、窗口目录状态和路由均有 QTest
User path:   U4，用户可用脚本固定工位配置目录
Device:      D1，不触碰真实串口连接验证
```
