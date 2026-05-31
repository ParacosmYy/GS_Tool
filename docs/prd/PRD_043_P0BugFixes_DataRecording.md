# PRD-043: P0 Bug Fixes + Data Recording Enhancement Phase 1

## 背景
BUG_AUDIT_042 发现 5 个 P0 级缺陷，其中 3 个直接影响用户核心操作（复制、搜索、OTA 传输），
2 个涉及运行时崩溃风险。按 CLAUDE.md 3.4.5 铁律，P0 未清零前禁止新特性开发。
FEATURE_REVIEW_042 已批准 Data Recording Enhancement（Proposal C），本迭代在修复 P0 后
启动其 Phase 1（DataBookmark 基础设施），为后续 Phase 2/3 的多流回放与时间范围导出奠定数据模型基础。

## 需求列表
| ID | 需求描述 | 优先级 | 涉及模块 |
|----|---------|--------|---------|
| R1 | TerminalWidget Ctrl+C 改用 `modifiers() == Qt::ControlModifier` 精确匹配，避免 Ctrl+Shift+C 误触 | P0 | terminal |
| R2 | TerminalWidget 搜索高亮在 timestamps 可见时 xOffset 已含时间戳宽度，需从 match.startCol 偏移中扣除时间戳字符数 | P0 | terminal |
| R3 | AnimatedProgressBar stopShimmer() 改为 disconnect + deleteLater，消除 stop 后立即 nullptr 与 DeleteWhenStopped 的竞态 | P0 | ota |
| R4 | XModemTransfer 新增 `m_userSelectedMode` 标志，仅当用户未显式选择 CRC 时才允许 NAK 切换为 Checksum | P0 | ota/protocols |
| R5 | ConnectionController connectNetwork() 成功后保存 host/port 到 m_lastConnectParams；onAutoReconnect 从已保存参数重建连接 | P0 | core |
| R6 | TerminalModel setMaxLines() 添加 QAtomicInt m_reentrancyGuard，emit dataCleared 前检测重入并提前返回 | P0 | terminal |
| R7 | 新增 DataBookmark 结构体：{qint64 timestampMs, QString label, quint8 streamId}，定义于 DataLogger.h | P1 | utils |
| R8 | DataLogger 持有 QVector<DataBookmark> m_bookmarks，提供 addBookmark/removeBookmark/bookmarks/clearBookmarks 四个 API | P1 | utils |

## 接口设计

### DataBookmark 结构体 (DataLogger.h 内定义)
```cpp
/** @brief 数据书签 - 在录制流中标记关键时间点 */
struct DataBookmark {
    qint64 timestampMs;   ///< 距录制开始的毫秒偏移
    QString label;        ///< 用户自定义标签
    quint8 streamId;      ///< 数据流标识 (0=serial, 1=RTT, 2=TCP, 3=UDP)
};
```

### DataLogger 新增公开方法
```cpp
void addBookmark(qint64 timestampMs, const QString& label, quint8 streamId = 0);
QVector<DataBookmark> bookmarks() const;
bool removeBookmark(int index);
void clearBookmarks();
```

### XModemTransfer 修改
```cpp
// 新增成员: bool m_userSelectedMode = false;
// setUserMode(Mode mode) 中设置 m_userSelectedMode = true
// WaitingForStart 状态中: if (!m_userSelectedMode && ch == NAK) 才切换
```

### ConnectionController 修改
```cpp
// 新增成员: QVariantMap m_lastConnectParams; // {"host","port","timeout"}
// connectNetwork() 成功后: m_lastConnectParams = params;
// onAutoReconnect(): 从 m_lastConnectParams 读取参数
```

### TerminalModel 修改
```cpp
// 新增成员: std::atomic<bool> m_inSetMaxLines{false};
// setMaxLines() 入口: if (m_inSetMaxLines.exchange(true)) return;
// 方法末尾: m_inSetMaxLines = false;
```

## 依赖的公共组件
| 组件 | 复用方式 |
|------|---------|
| DataLogger | 扩展（新增 bookmark 存储与 API） |
| TerminalModel | 修改（重入保护） |
| TerminalWidget | 修改（Ctrl+C 修正 + 搜索高亮修正） |
| AnimatedProgressBar | 修改（shimmer 生命周期修正） |
| XModemTransfer | 修改（模式保护） |
| ConnectionController | 修改（网络参数持久化） |

## 设计模式
- **观察者模式**: DataLogger::bookmarksChanged() 信号通知 UI 层书签变更（为 Phase 2 预留）
- **策略模式**: XModemTransfer 的 m_userSelectedMode 保护策略选择不被对端覆盖

## 影响范围
| 文件 | 变更类型 | 影响 |
|------|---------|------|
| src/terminal/TerminalWidget.cpp:365 | 一行修改 | Ctrl+C 精确匹配 |
| src/terminal/TerminalWidget.cpp:224 | 像素计算修正 | 搜索高亮去除时间戳双重偏移 |
| src/ota/AnimatedProgressBar.h:97-103 | 重构 stopShimmer | 消除 nullptr 竞态 |
| src/ota/protocols/XModemTransfer.cpp:174-178 | 条件分支增加 | 保护用户 CRC 选择 |
| src/core/ConnectionController.cpp:231,366 | 参数保存/恢复 | 网络自动重连使用正确参数 |
| src/terminal/TerminalModel.cpp:140-156 | 原子重入守卫 | 防止 dataCleared 信号重入 |
| src/utils/DataLogger.h | 新增 struct + 4 个方法 | Phase 1 bookmark 基础设施 |
| src/utils/DataLogger.cpp | 新增 4 个方法实现 | bookmark CRUD |

## 验收标准
1. Ctrl+C 复制正常；Ctrl+Shift+C 不触发复制，不被拦截
2. 搜索高亮在 timestamps 开启时位置正确，无水平偏移
3. AnimatedProgressBar 连续 start/stop 无崩溃，无 ASan 报告
4. XModem CRC 模式下接收方发 NAK 不降级为 Checksum
5. 网络连接断开后自动重连使用用户原始参数（非硬编码 127.0.0.1:8080）
6. TerminalModel::setMaxLines() 在 dataCleared 槽函数递归调用时安全返回
7. DataBookmark 结构体编译通过，DataLogger 四个 bookmark API 功能测试通过
8. 零编译错误，三主题 QSS 无变化（本次无 UI 新增）
9. 所有修改文件行数未突破 CLAUDE.md 4.6 上限
