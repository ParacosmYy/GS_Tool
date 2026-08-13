# ADR-0082：工作区 Tab 淡入 effect 生命周期

日期：2026-08-10  
状态：Accepted  
范围：`presentation/controllers/workspace_runtime.py`

## 背景

工作区 Tab 切换使用短时 `QGraphicsOpacityEffect` 和 `QPropertyAnimation` 提供页面淡入。
此前 animation 完成或被 stop 后只把 opacity 恢复为 `1.0`，没有把 effect 从 page 解绑。
这样会让页面长期携带一次性装饰对象，并在快速切换与生命周期边界增加 Qt 对象 ownership 风险。

## 决策

- 用 `_release_workspace_effect()` 作为 workspace transition 的唯一 effect 释放路径。
- 释放时先用 `shiboken6.isValid()` 判断 effect，再恢复 opacity，并在 effect 仍挂在目标 page 时调用 `setGraphicsEffect(None)`。
- `stop_workspace_transition()` 负责中断动画、清空当前引用并释放 effect；自然完成路径执行同一释放逻辑。
- 快速切换继续先 stop 上一个 transition，再创建新的一次性 effect；duration、easing、Tab 路由和业务状态不变。

## 边界

该模块只拥有 presentation transition 的临时资源，不拥有 ViewModel、transport、协议、记录器或业务计时器。
低动效、暂停、隐藏、最小化和关闭仍由现有 motion/lifecycle policy 决定并回到静态页面。

## 验证

```text
scripts/check.ps1                                      PASS
.venv\Scripts\python.exe -m compileall -q src scripts  PASS
.venv\Scripts\python.exe -m ruff check src scripts    PASS
UI192_WORKSPACE_EFFECT_VECTOR_PASS natural=1 stop=1 rapid_switch=1 reduced_motion=1 detached=4
```

Qt offscreen 向量未显示主窗口，覆盖自然完成、主动停止、快速切换和低动效回退；未启动可见 GUI/EXE、
未运行 HIDPI、读屏、真实设备、OTA/AES/RTT/J-Link、硬件、签名和正式发行验收。未创建、修改或运行
unit test、mock、fixture、harness 或 test-only 资产。嵌入式 C/C++ 适用性：N/A。
