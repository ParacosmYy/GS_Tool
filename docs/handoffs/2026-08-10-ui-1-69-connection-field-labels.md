# SerialForge UI-1.69 交接：连接配置字段标签

日期：2026-08-10  
项目：`D:\Workplace\Agent_Workplace\SerialForge`  
父代理：Codex；本轮唯一写入者  
范围：统一连接配置 UART/网络/BLE 普通字段标签的主题视觉层级，保持原生配置和交互契约。

## 交付结果

- 新增局部 `_field_label(text)` helper，普通字段统一设置 `QLabel[role="muted"]`。
- section 标题继续使用 `role="section"`；hint、状态、快捷提示和 BLE 属性摘要保留自身 presentation 语义。
- 未改变 preset/transport/BLE/network signal wiring、配置值、Tab order、focus/accessibility 或业务状态；未新增依赖、timer、
  QWidget、全局 label 工厂或跨 controller 访问。

## 角色与质量门

```text
产品/架构/UI/开发/验证/打包六角色均在源码修改前调用，等待超时后关闭：
019feb7d-a982-7e33-902f-53adb9b292c4
019feb7d-a9cc-7221-8e01-5e1834a522b6
019feb7d-aa17-7883-930c-5fc3a0a2f7ea
019feb7d-aa64-7481-a2e8-e1ca2e15bde9
019feb7d-aab3-7bd1-bfc1-13acfc656225
019feb7d-aafe-7bb2-9401-d297c8d52d07
独立质量复核：019feb7f-8abd-7b22-a841-3734a85fbab0，完成后调用，等待超时后关闭
```

父代理五轴审查：GO。独立复核未返回意见，未被计为通过。简化评估：局部 helper 消除重复 property 设置，不新增共享工厂或
生命周期接线。

## 验证

```text
check.ps1                         PASS 147 files <=1000; 3 themes; 22 tokens; 19 selectors
compileall -q src                 PASS
UI169_CONNECTION_LABELS_PASS      PASS × 3 themes; muted=37; sections=3
provenance.py verify              PASS
root/canonical hash               PASS
```

未启动完整 GUI、SerialForge.exe 或后台服务；未执行 HIDPI/读屏/真实视觉/真实串口、网络、BLE、RTT/J-Link、OTA、硬件、签名和
正式发行验收。未创建、修改或运行 unit test、mock、fixture、harness 或 test-only 资产。Qt offscreen 仅做短时内存布局向量，
字体目录警告不代表 Windows 字体结论。嵌入式 C/C++ 适用性：N/A。

## 最新 EXE

- 根目录：[SerialForge.exe](../../SerialForge.exe)
- canonical：[dist/release/0.1.0/core/onefile/app/SerialForge.exe](../../dist/release/0.1.0/core/onefile/app/SerialForge.exe)
- source revision：`local-ui-1.69`
- size：`47,886,264` bytes
- SHA-256：`63CD7F3E0C219B7034C5F18F96ADB04B4A5EAE490831495C3AC6A128AAE2A7E9`
- archive listing SHA-256：`C90B0C21DBC68E1B814A2DE8560D675CDFC90B5881A06AC1C1BDFC727338C600`
- `signature=NotSigned`；`release_eligible=false`；`hardware_acceptance=not_run`

EMBEDDED_WORKFLOW_COMPLETE: review + simplification + verification recorded
