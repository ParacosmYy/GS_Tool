# PRD-001: 连接工厂模式 + 数据导出 + 发送历史 + 连接状态管理重构

## 背景
当前项目存在以下架构缺陷：
1. `MainWindow` 直接 `new SerialConnection()` 违反工厂模式，新增连接类型时需要改 MainWindow
2. 缺少 `SendHistory` 发送历史组件，用户无法复用历史命令
3. 缺少 `DataExporter` 数据导出组件，用户无法保存串口数据
4. `DataStatistics` 统计面板未实现
5. 连接工厂 `ConnectionFactory` 缺失，创建连接的逻辑散落在 MainWindow 中
6. 连接状态管理不够完善，缺少统一的连接生命周期管理

## 需求列表

| ID | 需求描述 | 优先级 | 涉及模块 |
|----|---------|--------|---------|
| R1 | 实现 ConnectionFactory 工厂模式，统一创建连接对象 | P0 | core/ |
| R2 | 实现 SendHistory 发送历史，支持最近50条记录、搜索、复用 | P0 | serial/ |
| R3 | 实现 DataExporter 数据导出，支持 TXT/CSV/BIN 格式 | P0 | utils/ |
| R4 | 实现 DataStatistics 数据统计面板 | P1 | serial/ |
| R5 | 重构 MainWindow 使用工厂创建连接 | P0 | core/ |
| R6 | 完善 ConnectionManager 的连接生命周期管理 | P0 | core/ |

## 接口设计

### ConnectionFactory (工厂模式)
```cpp
class ConnectionFactory {
public:
    static IConnection* create(ConnectionType type, QObject* parent = nullptr);
};
```

### SendHistory
```cpp
class SendHistory : public QObject {
    Q_OBJECT
public:
    void addEntry(const QString& text, bool isHex);
    QStringList recentEntries(int count = 50) const;
    void clear();
signals:
    void historyChanged();
private:
    struct Entry { QString text; bool isHex; QDateTime time; };
    QList<Entry> m_entries;
    int m_maxEntries = 50;
};
```

### DataExporter
```cpp
class DataExporter : public QObject {
    Q_OBJECT
public:
    enum Format { Txt, Csv, Bin };
    bool exportToFile(const QString& filePath, Format format,
                      const QVector<TerminalLine>& lines,
                      const QDateTime& from = QDateTime(),
                      const QDateTime& to = QDateTime());
};
```

### DataStatistics
```cpp
class DataStatistics : public QWidget {
    Q_OBJECT
public:
    void update(quint64 rxBytes, quint64 txBytes, qint64 elapsed);
    void reset();
};
```

## 依赖的公共组件
- `Constants.h` — 枚举定义
- `TerminalModel` — 数据模型（导出数据源）
- `HexConverter` — HEX格式化
- `IConnection` — 连接接口

## 设计模式
- **工厂模式**: `ConnectionFactory` 封装连接创建逻辑，MainWindow 不需要知道具体类
- **单例模式**: `SendHistory` 可作为连接级别的单例

## 影响范围
| 文件 | 操作 |
|------|------|
| `src/core/ConnectionFactory.h/cpp` | 新增 |
| `src/serial/SendHistory.h/cpp` | 新增 |
| `src/utils/DataExporter.h/cpp` | 新增 |
| `src/serial/DataStatistics.h/cpp` | 新增 |
| `src/core/MainWindow.h/cpp` | 修改（使用工厂） |
| `src/core/ConnectionManager.h/cpp` | 修改（完善生命周期） |
| `src/CMakeLists.txt` | 修改（新增源文件） |

## 验收标准
1. `ConnectionFactory::create(Serial)` 返回 `SerialConnection*`
2. SendHistory 记录发送历史，下拉列表可复用
3. DataExporter 导出 TXT/CSV/BIN 格式正确
4. DataStatistics 实时显示收发速率
5. 编译零错误，commit ≥ 200行变更
