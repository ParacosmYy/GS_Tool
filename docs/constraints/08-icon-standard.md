# 08 - 图标体系规范

> 本文档是 EmbedDebug 约束体系的第8模块。涉及图标使用时必须加载。

---

## 一、图标库选型

| 属性 | 值 |
|------|-----|
| 库名 | Lucide Icons |
| 地址 | https://lucide.dev/ |
| 许可 | MIT License |
| 风格 | 线条风格(Line icons), 1.5px 描边 |
| 格式 | SVG (矢量, 支持任意缩放和着色) |

选型理由:
- MIT许可, 无法律风险
- 线条风格与 EmbedDebug 极简暗色UI一致
- 图标数量 1000+, 覆盖常见场景
- SVG 格式支持 `currentColor` 运行时着色
- 社区活跃, 持续更新

---

## 二、资源路径

- 图标文件存放: `resources/icons/lucide/<name>.svg`
- 示例:
  - `resources/icons/lucide/cable.svg` (串口连接)
  - `resources/icons/lucide/bluetooth.svg` (BLE连接)
  - `resources/icons/lucide/terminal.svg` (终端)
- 资源注册: `resources/app.qrc` 中统一注册
- 命名: kebab-case, 与 Lucide 原始文件名一致

---

## 三、图标尺寸规范

| 用途 | 尺寸(px) | 颜色 | 说明 |
|------|----------|------|------|
| 导航树节点图标 | 16 | TextSecondary | 树节点前的连接类型图标 |
| 面板标题图标 | 16 | TextSecondary | BasePanel 标题栏左侧 |
| 工具栏按钮 | 20 | TextSecondary | 工具栏各功能按钮 |
| 空状态图标 | 48 | TextMuted (60%) | EmptyStateWidget 中心图标 |
| 命令面板图标 | 16 | TextSecondary | CommandPalette 列表项 |
| Tab标签图标 | 16 | TextSecondary | 标签页图标 |

---

## 四、IconManager API

```cpp
/**
 * @brief 图标管理器, 负责图标加载、缓存和着色
 *
 * 单例, 通过 ThemeManager 感知主题变化。
 * 主题切换时自动清空缓存并重新着色。
 */
class IconManager : public QObject {
    Q_OBJECT
public:
    static IconManager& instance();

    /**
     * @brief 获取图标(自动缓存+着色)
     * @param name Lucide图标名(不含路径和后缀), 如 "cable"
     * @return 着色后的QIcon
     */
    QIcon icon(const QString& name) const;

    /**
     * @brief 获取指定尺寸的Pixmap
     * @param name 图标名
     * @param size 目标尺寸, 默认16
     * @return 着色后的QPixmap
     */
    QPixmap pixmap(const QString& name, int size = 16) const;

    /**
     * @brief 清空缓存(主题切换时调用)
     */
    void clearCache();

private:
    IconManager(QObject* parent = nullptr);
    mutable QMap<QString, QIcon> m_cache;  // 图标缓存
    QString tintSvg(const QString& svgPath, const QColor& color) const;
};
```

---

## 五、着色规则

### 默认着色流程

1. SVG 文件中所有 `fill` 和 `stroke` 属性使用 `currentColor`
2. IconManager 读取当前主题的 `TextSecondary` 色值
3. 运行时将 SVG 中的 `currentColor` 替换为实际色值
4. 缓存着色结果, 同一主题下不重复处理

### 状态着色

| 状态 | 颜色 | 说明 |
|------|------|------|
| 默认 | TextSecondary | IconManager 默认着色 |
| 选中/激活 | Accent | 导航树选中项、激活Tab |
| 悬停 | TextPrimary | 工具栏按钮hover |
| 禁用 | TextMuted (opacity 40%) | 按钮不可用时 |
| 错误 | Error | 连接断开指示 |
| 成功 | Success | 连接成功指示 |
| 警告 | Warning | 警告状态指示 |

---

## 六、图标子集注册表

EmbedDebug 使用的 Lucide 图标子集(~50个):

### 导航 (10个)

| 图标名 | 用途 |
|--------|------|
| `cable` | 串口连接 |
| `globe` | TCP/UDP连接 |
| `bluetooth` | BLE连接 |
| `radio` | 无线连接(CAN) |
| `wifi` | MQTT连接 |
| `cpu` | USB连接 |
| `circuit-board` | SPI/I2C连接 |
| `terminal` | 终端 |
| `layout-dashboard` | 仪表盘 |
| `plug` | 无连接占位 |

### 连接操作 (12个)

| 图标名 | 用途 |
|--------|------|
| `plug-zap` | 连接中 |
| `link` | 已连接 |
| `unlink` | 已断开 |
| `refresh-cw` | 重连 |
| `settings-2` | 连接设置 |
| `scan` | 扫描设备 |
| `search` | 搜索 |
| `x` | 关闭/取消 |
| `check` | 确认/成功 |
| `alert-circle` | 错误 |
| `alert-triangle` | 警告 |
| `info` | 信息提示 |

### 终端 (8个)

| 图标名 | 用途 |
|--------|------|
| `terminal-square` | 终端面板 |
| `trash-2` | 清除终端 |
| `copy` | 复制 |
| `clipboard` | 粘贴 |
| `download` | 导出 |
| `upload` | 导入 |
| `filter` | 过滤 |
| `scroll-text` | 自动滚动 |

### 数据/图表 (8个)

| 图标名 | 用途 |
|--------|------|
| `bar-chart-2` | 波形图 |
| `trending-up` | FFT频谱 |
| `git-commit` | 数据点 |
| `book-marked` | 书签 |
| `hash` | 数据统计 |
| `table-2` | 数据表格 |
| `activity` | 实时数据 |
| `layers` | 多通道 |

### 文件/操作 (8个)

| 图标名 | 用途 |
|--------|------|
| `folder-open` | 打开文件 |
| `save` | 保存 |
| `file-text` | 文件 |
| `hard-drive` | 固件文件 |
| `play` | 开始录制 |
| `square` | 停止录制 |
| `circle` | 录制中指示 |
| `clock` | 时间戳 |

### 动作 (12个)

| 图标名 | 用途 |
|--------|------|
| `send` | 发送数据 |
| `corner-down-left` | 回车发送 |
| `zap` | 快捷发送 |
| `repeat` | 定时发送 |
| `command` | 命令面板 |
| `maximize-2` | 全屏 |
| `minimize-2` | 退出全屏 |
| `chevron-down` | 折叠/展开 |
| `chevron-right` | 折叠指示 |
| `palette` | 主题切换 |
| `moon` | 暗色主题 |
| `sun` | 亮色主题 |

### 状态 (5个)

| 图标名 | 用途 |
|--------|------|
| `loader` | 加载中 |
| `circle-check` | 成功 |
| `circle-x` | 失败 |
| `shield-check` | 安全/校验通过 |
| `signal` | 信号强度 |

### 布局 (6个)

| 图标名 | 用途 |
|--------|------|
| `panel-left` | 左侧面板 |
| `panel-right` | 右侧面板 |
| `panel-bottom` | 底部面板 |
| `columns-2` | 双栏布局 |
| `rows-2` | 双行布局 |
| `sidebar` | 侧边栏 |

### 特殊 (9个)

| 图标名 | 用途 |
|--------|------|
| `bluetooth-searching` | BLE扫描中 |
| `radio-tower` | 无线信号 |
| `puzzle` | 插件 |
| `code-2` | 协议编辑 |
| `binary` | 二进制数据 |
| `arrow-up-down` | 上下行指示 |
| `package` | OTA固件包 |
| `key` | 加密/密钥 |
| `inbox` | 空数据状态 |

---

## 七、新增图标规则

添加新图标到项目中时, 必须遵循以下步骤:

1. **从 Lucide 图标库选取**, 不自行绘制
   - 访问 https://lucide.dev/icons/ 搜索图标
   - 确认图标名和风格符合项目需求

2. **在本文档子集注册表中登记**
   - 按分类添加到§六的对应表格中
   - 注明用途说明

3. **放置 SVG 文件**
   - 下载原始SVG放入 `resources/icons/lucide/<name>.svg`
   - 确认 SVG 内容使用 `currentColor` 作为填充色

4. **注册到 Qt 资源文件**
   - 在 `resources/app.qrc` 中添加 `<file>icons/lucide/<name>.svg</file>`

5. **代码中使用**
   ```cpp
   // 通过 IconManager 获取图标
   QIcon cableIcon = IconManager::instance().icon("cable");
   button->setIcon(cableIcon);
   ```

6. **三主题验证**
   - 确认图标在暗色/浅色/终端风三套主题下均正确着色
   - 主题切换后图标颜色自动更新
