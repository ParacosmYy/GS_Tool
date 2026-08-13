# ARCH-6f：动态属性刷新 owner 收窄

日期：2026-08-10  
范围：删除 MainWindow 中仅用于 QSS dynamic property 刷新的静态 facade；不改变状态投影、主题 selector、Qt signal 或生命周期语义。

## 交付

- 新增 `src/serialforge/presentation/property_refresh.py`，集中 `refresh_dynamic_property()`；值未变化时跳过刷新，值变化时执行
  `setProperty`、`unpolish/polish` 和 `update`。
- `bootstrap.py` 直接把刷新原语注入 `StatusSurfaceController`；`lifecycle.py`、`connection.py`、`commands.py`、
  `terminal_runtime.py` 显式导入，不再通过 MainWindow 间接调用。
- 删除 `MainWindow._set_dynamic_property` 与 lifecycle 中的同名函数；MainWindow 从 789 行降至 782 行，新文件 20 行。

## 架构审查

- 架构师 Luna max `019fe9d8-ffe3-7443-8ef9-75369bcfccb1` 在等待窗口内未返回结论，已关闭；未把超时解释为 GO。
- 父代理按已核对的最小依赖图实现：property refresh 只依赖 Qt widget；status surface 通过显式 callback 接收；其它
  presentation controller 直接调用同一原语，没有引入 controller 循环依赖或新的生命周期分支。

## 验证

```text
python -m compileall -q src                  pass
scripts/check.ps1                            pass (119 files <= 1000)
ARCH6F_PROPERTY_REFRESH                      pass
  facade=removed; property-module=explicit; qss-refresh=pass; status-callback=pass
  themes=3; sizes=2; hscroll=0; motion-stop=pass; near-white=0; screenshot=pass
  screenshot: build/ui_review_arch6f_property_refresh.png
PACKAGE_ARCH6F_FINAL                         pass
  artifact: `dist/release/0.1.0/core/onefile/app/SerialForge.exe`
  root: `SerialForge.exe` (byte-identical)
  size: 47,788,203 bytes
  SHA256: `C69A8343055B383B83954C65F852F3FA3B22A4428E591C95FEE7B48899065847`
  provenance/archive/hash: pass; archive listing SHA256 `9836C898423B2A3E131EC2C30EBC55D40B70ED775FDCE3FE442ED8DFF05B2B52`
  signature `NotSigned`; release eligible `false`; hardware acceptance `not_run`
```

离屏运行出现 PySide6 fonts 目录缺失提示，中文显示为方框；这不代表 Windows 打包环境字体缺失。未启动持续 GUI/EXE、真实
UART/TCP/UDP/BLE/RTT/J-Link/OTA 或硬件；没有创建、修改或运行 unit test、mock、fixture、harness。项目为 Python/PySide6
桌面应用，不含嵌入式 C/C++/固件；embedded-enterprise-workflow 与 embedded-code-review-simplifier 的厂商源适用性为 N/A。
