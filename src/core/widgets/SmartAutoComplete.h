/**
 * @file SmartAutoComplete.h
 * @brief 智能自动补全 — 基于发送历史的实时前缀匹配
 *
 * 浮动在发送栏下方的补全列表，用户每输入一个字符即刷新匹配结果。
 * 匹配策略: 前缀匹配 + 发送频率排序。
 *
 * 设计模式: 策略(Strategy) — 可替换匹配策略
 * 协作: SendHistory(数据源) / SendBar(触发输入)
 */
#ifndef SMART_AUTO_COMPLETE_H
#define SMART_AUTO_COMPLETE_H

#include <QWidget>
#include <QVector>

class QListWidget;
class QLineEdit;

/**
 * @brief 自动补全条目
 */
struct AutoCompleteEntry {
    QString text;       ///< 完整文本
    int frequency = 0;  ///< 使用频率(次数)
    qint64 lastUsed = 0;///< 最后使用时间戳(ms)
};

/**
 * @brief 智能自动补全弹出列表
 */
class SmartAutoComplete : public QWidget {
    Q_OBJECT

public:
    explicit SmartAutoComplete(QWidget* parent = nullptr);

    /** @brief 设置数据源(发送历史条目) */
    void setEntries(const QVector<AutoCompleteEntry>& entries);

    /** @brief 根据当前输入前缀过滤并显示 */
    void showForPrefix(const QString& prefix, const QPoint& position);

    /** @brief 隐藏补全列表 */
    void hideComplete();

    /** @brief 当前是否有选中项 */
    bool hasSelection() const;

    /** @brief 获取当前选中项文本 */
    QString selectedText() const;

    /** @brief 处理键盘导航(上/下/Enter/Escape) */
    bool handleKeyEvent(QKeyEvent* event);

    // ── 统计计数器 ──

    /** @brief 获取建议列表显示总次数 */
    quint64 totalSuggestions() const { return m_totalSuggestions; }
    /** @brief 获取用户选择补全项总次数 */
    quint64 totalSelections() const { return m_totalSelections; }
    /** @brief 重置所有统计计数器 */
    void resetAutoCompleteStatistics();

signals:
    /** @brief 用户选择了某个补全项 */
    void entrySelected(const QString& text);

private:
    /** @brief 过滤匹配条目，返回匹配索引 */
    QVector<int> filterEntries(const QString& prefix) const;

    /** @brief 排序: 频率降序 → 时间降序 */
    void sortEntries(QVector<int>& indices) const;

    QListWidget* m_listWidget = nullptr;  ///< objectName="autoCompleteList"
    QVector<AutoCompleteEntry> m_entries; ///< 全部历史条目
    QString m_currentPrefix;              ///< 当前过滤前缀

    // ── 统计计数器 ──
    quint64 m_totalSuggestions = 0;       ///< 建议列表显示次数
    quint64 m_totalSelections = 0;        ///< 用户选择补全项次数
};

#endif // SMART_AUTO_COMPLETE_H
