/**
 * @file NotificationHistory.h
 * @brief 通知历史记录控件 -- 应用通知日志查看/过滤/导出
 *
 * 功能: 记录应用所有通知(Debug/Info/Warning/Error/Critical)，表格显示
 *       (时间/级别/来源/标题/消息)，按级别过滤和文本搜索，颜色编码级别，
 *       自动滚动到最新，支持JSON和CSV格式导出。
 *
 * 协作: NotificationManager(通知来源) / ToastWidget(实时通知) / EventBus(事件总线)
 */
#ifndef NOTIFICATIONHISTORY_H
#define NOTIFICATIONHISTORY_H

#include <QWidget>
#include <QList>
#include <QString>
#include <QColor>

class QTableWidget;
class QComboBox;
class QLineEdit;
class QPushButton;
class QLabel;

/**
 * @class NotificationHistory
 * @brief 通知历史记录控件 -- 滚动查看所有通知历史，支持5级过滤/搜索/导出
 */
class NotificationHistory : public QWidget {
    Q_OBJECT

public:
    /** @brief 通知级别枚举，从低到高 */
    enum class Level { Debug = 0, Info = 1, Warning = 2, Error = 3, Critical = 4 };
    Q_ENUM(Level)

    /** @brief 单条通知记录 */
    struct NotificationEntry {
        qint64 timestamp = 0;   ///< 创建时间戳(ms)
        Level level = Level::Info; ///< 通知级别
        QString title;          ///< 通知标题
        QString message;        ///< 通知消息内容
        QString source;         ///< 来源组件标识
        int id = 0;             ///< 唯一自增ID
    };

    /** @brief 运行统计数据 */
    struct Stats {
        quint64 totalNotifications = 0;         ///< 累计通知总数
        quint64 totalByLevel[5] = {0,0,0,0,0};  ///< 各级别通知计数
        quint64 totalSearches = 0;               ///< 累计搜索次数
        quint64 totalExports = 0;                ///< 累计导出次数
        quint64 totalClears = 0;                 ///< 累计清除次数
        int peakHistorySize = 0;                 ///< 历史记录峰值大小
    };

    /**
     * @brief 构造通知历史记录控件
     * @param maxHistory 最大历史条目数，默认500
     * @param parent 父控件
     */
    explicit NotificationHistory(int maxHistory = 500, QWidget* parent = nullptr);

    /** @brief 添加通知条目 @param level 级别 @param title 标题 @param message 消息 @param source 来源 */
    void addEntry(Level level, const QString& title, const QString& message,
                  const QString& source = {});

    /** @brief 添加通知条目 @param entry 完整条目 */
    void addEntry(const NotificationEntry& entry);

    /** @brief 获取全部条目 @return 通知条目列表 */
    QList<NotificationEntry> entries() const { return m_entries; }

    /** @brief 获取过滤后的条目 @return 过滤结果列表 */
    QList<NotificationEntry> filteredEntries() const;

    /** @brief 设置级别过滤器 -- 只显示>=minLevel的通知 @param minLevel 最低级别 */
    void setFilterLevel(Level minLevel);

    /** @brief 设置搜索文本 @param text 搜索关键字(标题+消息+来源) */
    void setSearchText(const QString& text);

    /** @brief 清除所有历史记录 */
    void clearHistory();

    /** @brief 获取总条目数 @return 总数 */
    int count() const { return m_entries.size(); }

    /** @brief 获取过滤后条目数 @return 过滤后数量 */
    int filteredCount() const;

    /** @brief 导出为JSON文件 @param filePath 文件路径 @return 是否成功 */
    bool exportToJson(const QString& filePath);

    /** @brief 导出为CSV文件 @param filePath 文件路径 @return 是否成功 */
    bool exportToCsv(const QString& filePath);

    /** @brief 获取统计引用 @return Stats常量引用 */
    const Stats& stats() const { return m_stats; }

    /** @brief 重置统计计数器 */
    void resetStatistics();

signals:
    /** @brief 新条目添加时发射 @param id 条目ID */
    void entryAdded(int id);
    /** @brief 历史记录清除时发射 */
    void historyCleared();
    /** @brief 过滤器变更时发射 @param minLevel 当前最低级别 */
    void filterChanged(Level minLevel);

private:
    /** @brief 构建UI布局 -- 工具栏+表格+状态栏 */
    void setupUI();
    /** @brief 刷新表格视图 -- 根据当前过滤器重绘所有行 */
    void refreshView();
    /** @brief 级别枚举转字符串 @param level 级别 @return 中文名称 */
    QString levelToString(Level level) const;
    /** @brief 获取级别对应的颜色 @param level 级别 @return QColor */
    QColor levelColor(Level level) const;
    /** @brief 格式化时间戳 @param ts 毫秒时间戳 @return 格式化字符串 HH:mm:ss.zzz */
    QString formatTimestamp(qint64 ts) const;
    /** @brief 判断条目是否通过当前过滤条件 @param entry 条目 @return true通过 */
    bool matchesFilter(const NotificationEntry& entry) const;

    QList<NotificationEntry> m_entries;     ///< 全部通知条目(最新在前)
    Level m_filterLevel = Level::Debug;     ///< 最低显示级别
    QString m_searchText;                   ///< 搜索关键字
    int m_maxHistory;                       ///< 最大历史条目数
    int m_nextId = 0;                       ///< 自增ID计数器
    Stats m_stats;                          ///< 运行统计

    // ---- UI控件 ----
    QTableWidget* m_table = nullptr;        ///< 通知表格(时间/级别/来源/标题/消息)
    QComboBox* m_levelCombo = nullptr;      ///< 级别过滤下拉框
    QLineEdit* m_searchEdit = nullptr;      ///< 搜索输入框
    QPushButton* m_clearBtn = nullptr;      ///< 清除按钮
    QPushButton* m_exportJsonBtn = nullptr; ///< JSON导出按钮
    QPushButton* m_exportCsvBtn = nullptr;  ///< CSV导出按钮
    QLabel* m_statusLabel = nullptr;        ///< 状态栏标签
};

#endif // NOTIFICATIONHISTORY_H
