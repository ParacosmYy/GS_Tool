# UI-1.119 motion fanout 性能实验（撤回）

日期：2026-08-11  
父代理：Codex；父代理是本轮唯一写入者  
共享 checkout：`D:\Workplace\Agent_Workplace\SerialForge`；未检测到 `.git`，未创建/操作 worktree

## 实验与结论

当前 `_motion_surfaces()` 有 56 个 surface；四个 Tab 可见数量为 `24/37/24/17`。拟议改动是在
`lifecycle.on_motion_frame()` 每帧以 `QWidget.isVisible()` 跳过隐藏 surface，同时保留 stop 全量
fan-out。相同 1180×780 offscreen 合成根、120 帧、每帧 `processEvents()`、三轮样本对照如下：

```text
全量广播参考：603.93 / 604.31 / 565.14 ms，均值 591.13 ms
isVisible() 过滤：562.71 / 737.82 / 736.15 ms，均值 678.89 ms
```

过滤方案退化，已撤回源码改动；不打新包，UI-1.118 canonical artifact 仍是当前交付物。撤回后的
静态门禁、compileall、Ruff 和 source line limit 通过。

## 审查与后续

架构师线程超时，未计为通过；父代理按 performance skill 的同条件复测规则判定该方案为负收益，
没有把复杂度保留在生产代码中。`_stop_motion_surfaces()` 的全量 stop 向量残留动效为 0，但不
作为性能收益证据。若继续优化，应在 Tab 切换时缓存 visible surface 集合，再以相同测量方法复测，
不得在每帧增加可见性查询、timer、registry 或业务状态。

本实验不包含嵌入式 C/C++，embedded applicability=N/A。
