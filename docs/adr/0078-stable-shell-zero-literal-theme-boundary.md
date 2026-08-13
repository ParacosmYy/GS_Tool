# ADR 0078：稳定 Shell 零字面量主题边界

日期：2026-08-10

状态：已接受

## 背景

UI-1.87 已清理 controls stable template 的局部颜色，但 `theme_stylesheet_base.py` 仍带有连接状态带、观测/发送表面、pipeline/status badge 等大量默认 hex。variant
通常会覆盖它们，但 fallback 与主题切换的颜色来源仍不透明，也不利于新增 OTA/debug 页面复用。

## 决策

- base stable stylesheet 的颜色全部引用 `theme_tokens.py` 已存在的 semantic roles；稳定文件不保留 hex literal。
- 状态 selector、property projection、几何、焦点/禁用/选择和动效结构保持原样；`theme_variant_shell.py` 继续拥有三主题具体覆盖。
- 不新增 palette registry、状态源、动态 timer、事件总线、controller、设备库或密钥依赖。
- 用静态 token audit、compileall、ruff 和三主题 shell state offscreen vector 作为证据；可见 GUI、硬件与正式发布另行授权。

## 结果

base/controls 两个稳定模板共享同一语义颜色边界，新增嵌入式调试站页面只需复用 `ThemeSpec` 角色，不需要复制默认色板；主题切换和安全/业务层依赖方向不变。

