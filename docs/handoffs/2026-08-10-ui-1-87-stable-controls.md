# UI-1.87 Stable Controls Semantic Tokens

日期：2026-08-10

## 交付摘要

`theme_stylesheet_controls.py` 已将状态面、输入/禁用态、SpinBox/ComboBox popup、Menu、按钮、checkbox、workspace Tab、terminal、终端空态和批量空态
接入已有 semantic token。没有改变 selector、property state、焦点/选择/禁用行为、布局尺寸、业务 projection 或三主题 variant owner。

## 评审记录

第一轮产品 `019fec25-5f57-7320-b0ed-e793815f07cd`、架构 `019fec25-5fad-76a3-8009-b46bc5a632cd`、UI `019fec25-5ffc-7761-8de9-214a15e5b3b3`、
开发 `019fec25-604d-7600-bc43-bce63ee61a12`、验证 `019fec25-609b-7c42-8ea4-4b85068222a2`、打包 `019fec25-60e3-7af2-83a8-e84b0a3f6d41` 均已调用但超时关闭。

第二轮产品 `019fec26-db34-7912-b595-2d36852e5b72`、架构 `019fec26-db82-7082-942d-0f2a14acafbb`、UI `019fec26-dbcf-7a13-9378-7caf6a50896a`、
开发 `019fec26-dc1d-7ac0-9182-b753dfb5eede`、验证 `019fec26-dc6b-7880-b60c-9e5db022e8fa`、打包 `019fec26-dcb9-7573-8dba-d746f988e907` 为 import-fix review，均超时关闭。

独立复核 `019fec27-bc61-7ba1-bdd5-cb94e2e73ffd` 在实现后调用，超时关闭；超时不视为通过。父代理完成五轴审查：正确性确认已有 token 与 QSS 解析；可读性/简化确认不新增抽象；
架构确认 stable/variant 边界不变；安全确认无输入、网络、存储、密钥和依赖变化；性能确认只改变样式字符串。嵌入式 C/C++ 适用性：N/A。

## 验证

```text
scripts/check.ps1                                      PASS
.venv\\Scripts\\python.exe -m compileall -q src          PASS
.venv\\Scripts\\ruff.exe check src/serialforge/        PASS
UI187_CONTROL_SURFACE_VECTOR_PASS themes=3 controls=line,combo,button,checkbox,tab,terminal,table
theme token audit                                      PASS (legacy_qss_literals=208)
```

向量为 Qt offscreen 内存 widget，未显示主窗口；当前环境的 PySide6 字体目录警告不影响 QSS/token 验证。未运行可见 GUI、HIDPI、读屏、EXE 启动、真实设备/网络、OTA、签名和正式发行验收，
也未创建、修改或运行 unit test、mock、fixture、harness 或 test-only 资产。

## 打包

```text
UI187_PACKAGE_FINAL: PASS
artifact: dist/release/0.1.0/core/onefile/app/SerialForge.exe
root: SerialForge.exe (byte-identical)
source revision: local-ui-1.87
size: 47,894,441 bytes
SHA-256: 632E63F3D038A15571194AD89567D66B40C4D0C4452C4C54AD5804AEAF2E4638
archive listing SHA-256: 29EC8DE9EE334CB8ADAE1802391D18BA980954D8901F7C500841CD70109A720D
python / PyInstaller: 3.12.13 / 6.22.0
signature / release_eligible: NotSigned / false
hardware_acceptance: not_run
```

canonical artifact 已覆盖根目录 `SerialForge.exe`，两者 size/SHA-256 一致；包为未签名 `engineering_build`，不能等同正式发行或硬件验收通过。

EMBEDDED_WORKFLOW_COMPLETE: review + simplification + verification recorded
