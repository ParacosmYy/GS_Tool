# PRD-123 Serial Station 上次档案快捷启动

## 背景

Serial Station 已支持 `--profile <file.edserialprofile>` 按文件启动，也支持在工作台里记录最近档案和重载上次。实际工位使用时，工程师更常见的诉求是“直接回到上次那套配置”，不希望每次都复制完整档案路径。

## 目标

1. 新增启动参数 `--last-profile` 和 `--serial-last-profile`。
2. 参数出现时自动打开 Serial Station，并加载最近一次成功使用的配置档案。
3. 若用户同时提供 `--profile`，显式路径优先，上次档案只作为无路径时的快捷入口。
4. 无上次档案、档案缺失或档案损坏时，仍打开工作台并给出系统日志，不阻塞应用启动。

## 非目标

1. 不自动连接真实串口。
2. 不删除、移动或重写 `.edserialprofile` 文件。
3. 不改变最近档案持久化格式。
4. 不提升设备验证等级；本轮仍为 D1 自动化测试。

## 用户故事

作为嵌入式调试工程师，我希望双击脚本或命令行能直接恢复上次工位配置，从而减少每日开机后的重复操作。

## 验收标准

1. `EmbedDebug.bat --station serial --last-profile` 能直达 Serial Station。
2. `EmbedDebug.bat --last-profile` 在没有显式面板时自动路由到 Serial Station。
3. `--profile <file>` 与 `--last-profile` 同时存在时，显式档案路径优先。
4. 工作台能通过公开方法加载上次档案，并更新端口、协议、命令和日志。
5. 无上次档案时返回 false，并写入可诊断日志。

## 三轴目标

```text
Engineering: E5，启动参数和工作台加载路径均有 QTest
User path:   U4，用户可用一个短参数恢复上次工位
Device:      D1，不触碰真实串口连接验证
```
