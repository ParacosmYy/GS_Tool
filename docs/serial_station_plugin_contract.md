# Serial Station 插件契约

> 适用范围：用户自定义协议与控件插件，通过 `python/embeddebug/serial_station/plugins/` 加载。
> 对齐 VOFA+ 开放可扩展插件体系：不改主线代码即可扩展能力。

---

## 一、插件目录结构

插件放在用户插件根目录下，每个插件一个子目录，子目录内必须有 `plugin.py`：

```text
<plugin_root>/
  my_protocol/
    plugin.py          # 必须，插件入口
  my_control/
    plugin.py
```

插件根目录默认为 `plugins_user/`（可配置）。`PluginManager.discover()` 扫描
该目录下所有含 `plugin.py` 的子目录。

---

## 二、plugin.py 契约

每个 `plugin.py` 必须定义以下模块级变量与函数：

| 符号 | 类型 | 必填 | 说明 |
|---|---|---|---|
| `PLUGIN_NAME` | str | 是 | 插件显示名（唯一标识） |
| `PLUGIN_VERSION` | str | 是 | 版本号 |
| `PLUGIN_TYPE` | str | 是 | `"protocol"` 或 `"control"` |
| `register(registry)` | callable | 是 | 注册函数，把插件能力注册到 registry |

### 2.1 协议插件（PLUGIN_TYPE = "protocol"）

`register(registry)` 接收一个 `SerialProtocolRegistry`，调用 `registry.register(name, factory)`。
`factory` 是无参可调用对象，返回一个 `SerialProtocol` 实例。

```python
PLUGIN_NAME = "my_proto"
PLUGIN_VERSION = "1.0.0"
PLUGIN_TYPE = "protocol"

from embeddebug.serial_station.protocols.base import SerialProtocol

class MyProtocol(SerialProtocol):
    name = "my_proto"
    # 实现 feed / build_command / reset ...

def register(registry):
    registry.register("my_proto", MyProtocol)
```

### 2.2 控件插件（PLUGIN_TYPE = "control"）

`register(registry)` 接收一个控件 registry（dict 或类似对象），注册控件工厂。

```python
PLUGIN_NAME = "my_led"
PLUGIN_VERSION = "0.1.0"
PLUGIN_TYPE = "control"

from PyQt6.QtWidgets import QWidget

class MyWidget(QWidget):
    ...

def register(registry):
    registry["my_led"] = MyWidget
```

---

## 三、插件生命周期

| 阶段 | 方法 | 说明 |
|---|---|---|
| 发现 | `PluginManager.discover()` | 扫描目录，提取元数据，不执行 register |
| 加载 | `PluginManager.load_all(...)` | 加载所有非禁用插件，执行 register |
| 启用 | `PluginManager.enable(name)` | 移除禁用标记 |
| 禁用 | `PluginManager.disable(name)` | 标记禁用，下次 load_all 跳过 |
| 重载 | `PluginManager.reload(name)` | 清除已加载标记后重新加载 |

无效插件（语法错误、缺 register、未知 PLUGIN_TYPE）在 discover 阶段静默跳过，
不抛异常，保证主程序启动稳定。

---

## 四、边界约束

- 插件代码在主进程内执行（非沙箱），用户需自行确保插件可信。
- 插件不得 import 主线的 controller 内部状态，只能通过 register 注入 registry。
- 插件失败不影响其它插件加载（异常被捕获，该插件标记为未加载）。
- 协议插件必须实现 `SerialProtocol` 接口（feed/build_command/reset）。

---

## 五、示例

见 `tests/python/unit/test_plugins.py` 中的 `_write_protocol_plugin` 与
`_write_control_plugin` fixture，展示最小可用插件。
