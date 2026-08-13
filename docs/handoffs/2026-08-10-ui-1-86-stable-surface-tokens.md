# UI-1.86 Stable Control Surface Tokens

日期：2026-08-10

## 交付摘要

`theme_stylesheet_controls.py` 已把 QTableWidget、表头、QStatusBar、双向 QScrollBar、corner 和 QToolTip 的稳定颜色统一接入已有语义
token。表头与 Tooltip 继续保留渐变层级；三主题 variant override、原生控件交互、业务状态和 accessibility 契约没有改变。

## 评审记录

产品 `019fec1d-7654-72a3-ad58-53ccb04107bd`、架构 `019fec1d-76a5-78f0-a504-44f2197f861f`、UI `019fec1d-76f5-7b13-afeb-2f4264e81790`、
开发 `019fec1d-7744-7551-a4c6-ce6909e91af5`、验证 `019fec1d-778e-7393-a12e-642ee0897013`、打包 `019fec1d-77ed-77b2-bc2c-5ad51a323d73`
角色均已调用但在等待窗口内超时并关闭；独立复核 `019fec1f-6f85-77c0-a620-560e824c0ae5` 同样超时并关闭，均不视为通过。

父代理五轴审查：正确性确认所有颜色引用来自已有 token；可读性/简化确认没有新增抽象层；架构确认 stable/variant owner 边界不变；安全确认无输入、网络、
存储、密钥和依赖变化；性能确认仅改变 QSS 字符串，不增加 timer、线程、事件总线或绘制路径。嵌入式 C/C++ 适用性：N/A。

## 验证

```text
scripts/check.ps1                                      PASS
.venv\\Scripts\\python.exe -m compileall -q src          PASS
.venv\\Scripts\\ruff.exe check src/serialforge/        PASS
UI186_STABLE_QSS_NEAR_WHITE_PASS count=0
UI186_THEME_SURFACE_VECTOR_PASS themes=3 selectors=table,statusbar,scrollbar,tooltip
```

向量为 Qt offscreen 内存 widget，未显示主窗口；未运行可见 GUI、HIDPI、读屏、EXE 启动、真实设备/网络、OTA、签名和正式发行验收，也未创建或运行
unit test、mock、fixture、harness 或 test-only 资产。当前环境的 PySide6 字体目录警告不影响本轮 QSS 颜色验证。

## 打包

```text
UI186_PACKAGE_FINAL: PASS
artifact: dist/release/0.1.0/core/onefile/app/SerialForge.exe
root: SerialForge.exe (byte-identical)
source revision: local-ui-1.86
size: 47,892,639 bytes
SHA-256: 18C089F9C700E230F676F1AB1EEDF5F777ED3584D0352C789800CC8AC0BA1683
archive listing SHA-256: 29EC8DE9EE334CB8ADAE1802391D18BA980954D8901F7C500841CD70109A720D
python / PyInstaller: 3.12.13 / 6.22.0
signature / release_eligible: NotSigned / false
hardware_acceptance: not_run
```

canonical artifact 已覆盖根目录 `SerialForge.exe`，两者 size/SHA-256 一致；包为未签名 `engineering_build`，不能等同正式发行或硬件验收通过。

EMBEDDED_WORKFLOW_COMPLETE: review + simplification + verification recorded
