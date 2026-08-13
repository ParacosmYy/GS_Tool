# UI-1.88 Stable Shell Semantic Tokens

日期：2026-08-10

## 交付摘要

`theme_stylesheet_base.py` 的 app shell、section/error/status、连接/观测/发送 band、pipeline、协议/组件/数据集/回放状态和 preset/context badge 已全部接入已有
semantic token；base 与 controls 的稳定 hex literal audit 为 `0`。selector、property state、焦点/选择/禁用、布局、动效和业务 projection 保持不变。

## 评审记录

第一轮产品 `019fec2b-b9dc-7ba0-9d6c-b6137eb8ba46`、架构 `019fec2b-ba28-7a62-b0d4-104b671e2a90`、UI `019fec2b-ba74-7140-8f4c-4027b048592d`、
开发 `019fec2b-bac2-7421-9243-3ee890413c91`、验证 `019fec2b-bb0e-7d72-82ee-1ae86c5cec59`、打包 `019fec2b-bb60-7a81-bf27-82e1b26f7c5d` 均已调用但超时关闭。

第二轮产品 `019fec2c-df18-70f3-93b7-66d0a490f8e4`、架构 `019fec2c-df67-7f62-9c37-1f75e37865cb`、UI `019fec2c-dfb1-7a93-a6d9-7d784753f40d`、
开发 `019fec2c-dfff-7e22-9e16-b27906688184`、验证 `019fec2c-e04d-7af2-889c-d09ad4ca8625`、打包 `019fec2c-e098-7f71-b7d0-b60f63ee51c9` 为 import-fix review，均超时关闭。

独立复核 `019fec2e-afc4-7c83-aa13-be5a25b15c61` 在实现后调用，超时关闭；超时不视为通过。父代理五轴审查：正确性确认 token 与 stylesheet 解析；可读性/简化确认不新增抽象；
架构确认 base/variant owner 边界不变；安全确认无输入、网络、存储、密钥和依赖变化；性能确认只改变 QSS 字符串。嵌入式 C/C++ 适用性：N/A。

## 验证

```text
scripts/check.ps1                                      PASS
.venv\\Scripts\\python.exe -m compileall -q src          PASS
.venv\\Scripts\\ruff.exe check src scripts              PASS
UI188_SHELL_STATE_VECTOR_PASS themes=3 states=connection,observation,send,pipeline,status,replay
theme token audit                                      PASS (legacy_qss_literals=0)
```

向量为 Qt offscreen 内存 widget，未显示主窗口；第一次断言误报合法选中文字，收紧为 white/#ffffff 背景 fallback 后通过。当前环境的 PySide6 字体目录警告不影响验证。未运行可见 GUI、HIDPI、读屏、EXE 启动、真实设备/网络、OTA、签名和正式发行验收，也未创建、修改或运行 unit test、mock、fixture、harness 或 test-only 资产。

## 打包

```text
UI188_PACKAGE_FINAL: PASS
artifact: dist/release/0.1.0/core/onefile/app/SerialForge.exe
root: SerialForge.exe (byte-identical)
source revision: local-ui-1.88
size: 47,895,484 bytes
SHA-256: 78DB186E8BFD92E48F09A1E8A9830DA16B45F2995912D8A63682C689944A87D4
archive listing SHA-256: 29EC8DE9EE334CB8ADAE1802391D18BA980954D8901F7C500841CD70109A720D
python / PyInstaller: 3.12.13 / 6.22.0
signature / release_eligible: NotSigned / false
hardware_acceptance: not_run
```

canonical artifact 已覆盖根目录 `SerialForge.exe`，两者 size/SHA-256 一致；包为未签名 `engineering_build`，不能等同正式发行或硬件验收通过。

EMBEDDED_WORKFLOW_COMPLETE: review + simplification + verification recorded
