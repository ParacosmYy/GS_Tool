# ARCH-7b 交接：连接 preset catalog 的版本化 store 边界

日期：2026-08-10  
范围：连接快速配置的 presentation persistence boundary。  
父代理：Codex；父代理是本轮唯一写入者。  
架构师：Luna max `019fe95e-3b96…`，最小边界 GO。  
独立复核：Luna max `019fe964-eda5…`，修正后 GO，Critical=0、Required=0。  

## 变更

- 新增 `src/serialforge/presentation/connection_preset_codec.py`，定义 schema v1 的显式 JSON 白名单 codec；只保存
  `key/label/description/transport/values` 等安全显示/连接选项，不保存密钥、设备句柄、BLE 身份或自动连接动作。
- 新增 `src/serialforge/presentation/connection_preset_store.py`，暴露 `ConnectionPresetCatalogStore` port，并提供唯一
  Qt adapter `QSettingsConnectionPresetCatalogStore`。组合根加载 catalog 后注入 `MainWindow`，
  `controllers/connection_builder.py` 只接收显式 catalog，不直接访问 QSettings。
- codec 严格拒绝未知字段、未知 transport、schema 非严格整数、DTO 非严格整数、超长 payload 和坏 catalog；store 在读取
  失败时回退 immutable 的七项 builtin catalog。
- store 保存采用 begin/set/end 分阶段状态；任一阶段失败都 fail-open 且跳过 sync，避免将未完成写入当成成功。
- `MainWindow` 当前为 944 行；未引入 `__getattr__`、动态注册、monkey-patch、mixin 或第二套 preset 状态源。

## 架构边界

本切片只建立安全、可替换的读取/保存 contract，不宣称已经提供用户自定义 preset 编辑。builtin/custom merge、preset editor
和显式 save action 归入后续 ARCH-7c，必须另行评审产品语义和删除/回滚策略。runtime 继续只消费规范化控件值构造 domain
config；任何用户保存都不得把原始密钥或敏感端点写入 catalog。

## 评审与简化

- 架构评审允许的最小范围是 `port → codec → QSettings adapter → composition root 注入`；拒绝把完整 catalog facade、
  用户编辑器和 merge 策略一次性塞进 MainWindow。
- 独立复核首次指出 schema/DTO 对 bool、float 冒充整数的漏洞和 QSettings group lifecycle 的失败路径；已补充 strict-int
  校验、load 的 entered/finally 处理，以及 save 的 begin/set/end success gate。复核后 Critical=0、Required=0。
- 结构简化保持为三个窄模块：codec 负责序列化约束，store 负责持久化 fail-open，builder 负责 UI 组装；没有新增兼容 facade
  或跨层共享字典。

## 验证

```text
targeted py_compile / ruff                       pass
scripts/check.ps1                               pass (113 files <= 1000)
ARCH7B_CATALOG_BOUNDARY                          pass (roundtrip=7, invalid fallback, 980/1180, hscroll=0)
ARCH7B_STRICT_FINAL                              pass (bool/float rejection, schema=2, DTO=6, store load)
```

未创建、修改或运行 unit test、mock、fixture、harness；未启动持续 GUI/EXE、HIDPI、真实焦点/读屏、动效性能或 UART/TCP/UDP/BLE/
RTT/J-Link 硬件。该项目当前不包含嵌入式 C/C++、固件或 MCU，因此 `embedded-enterprise-workflow` 与
`embedded-code-review-simplifier` 的厂商源适用性为 N/A；记录的是静态、离屏和打包等授权的非破坏性证据，不声称硬件、
认证或正式发行合规。

## 当前 onefile 交付

canonical：`dist/release/0.1.0/core/onefile/app/SerialForge.exe`  
根目录覆盖：[SerialForge.exe](../../SerialForge.exe)  

```text
canonical/root byte match                          pass
SHA-256                                             76E98A9EAA1C607F19493369BF0AC3344E1C5B059B577CCA9255EB66DA410F5A
size                                                47,766,244 bytes
manifest hash/size                                  pass
archive required modules                            present (6/6)
signature                                           NotSigned
release_eligible                                   false
hardware_acceptance                                not_run
vendor_binary_matches                              0
```

`PROVENANCE.json` 的工程包状态为 `engineering_build`；未启动 EXE 或连接真实设备。该包不是正式签名发布物。

EMBEDDED_WORKFLOW_COMPLETE: review + simplification + verification recorded
