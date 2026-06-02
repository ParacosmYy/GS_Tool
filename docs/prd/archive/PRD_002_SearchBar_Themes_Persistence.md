# PRD-002: 终端搜索栏 + 多主题切换 + 配置持久化 + 导航树完善

## 背景
当前项目存在以下缺陷：
1. 终端缺少搜索功能，大量数据中找不到关键内容
2. 主题只有一套暗色，无法切换
3. 应用重启后所有配置丢失，体验差
4. 导航树只有 Serial Port 节点，没有 Tools/Help 等分类
5. DataStatistics 面板没有从 MainWindow 的状态栏逻辑中独立出来

## 需求列表

| ID | 需求描述 | 优先级 | 涉及模块 |
|----|---------|--------|---------|
| R1 | 实现 TerminalSearchBar 搜索栏组件 | P0 | terminal/ |
| R2 | 新增 modern_dark.qss 和 light.qss 主题 | P1 | resources/ |
| R3 | ThemeManager 支持外部 QSS 文件热加载 | P1 | core/ |
| R4 | SettingsManager 保存/恢复串口配置、窗口位置、主题选择 | P0 | utils/ |
| R5 | 导航树增加 Tools(导出/统计) 和 Help 分组 | P1 | core/ |
| R6 | MainWindow 启动时自动恢复上次会话配置 | P0 | core/ |

## 接口设计

### TerminalSearchBar
```cpp
class TerminalSearchBar : public QWidget {
    Q_OBJECT
public:
    explicit TerminalSearchBar(QWidget* parent = nullptr);
    QString searchPattern() const;
    bool isRegexMode() const;
    bool isHexMode() const;
signals:
    void searchRequested(const QString& pattern, bool regex, bool hex);
    void searchCleared();
    void closed();
};
```

### SettingsManager 增强
```cpp
// 新增方法
void saveSerialConfig(const QVariantMap& config);
QVariantMap loadSerialConfig() const;
void saveWindowGeometry(const QByteArray& geometry);
QByteArray loadWindowGeometry() const;
void saveTheme(const QString& themeName);
QString loadTheme() const;
```

## 设计模式
- **观察者模式**: TerminalSearchBar 发出搜索信号，TerminalWidget 响应高亮
- **单例模式**: SettingsManager 持久化全局配置

## 依赖的公共组件
- `SettingsManager` — 配置持久化
- `ThemeManager` — 主题切换
- `HexConverter` — HEX搜索验证

## 影响范围
| 文件 | 操作 |
|------|------|
| `src/terminal/TerminalSearchBar.h/cpp` | 新增 |
| `src/core/MainWindow.h/cpp` | 修改（搜索栏、会话恢复、导航树） |
| `src/utils/SettingsManager.h/cpp` | 修改（新增便捷方法） |
| `src/core/ThemeManager.h/cpp` | 修改（支持外部文件） |
| `resources/themes/modern_dark.qss` | 新增 |
| `resources/themes/light.qss` | 新增 |

## 验收标准
1. Ctrl+F 弹出搜索栏，输入关键词后终端高亮匹配
2. 支持 HEX 模式搜索（如 "AA 55"）
3. 三套主题可切换，重启后保持
4. 串口配置和窗口位置重启后恢复
