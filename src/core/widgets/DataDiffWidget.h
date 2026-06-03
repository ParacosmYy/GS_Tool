/**
 * @file DataDiffWidget.h
 * @brief 数据对比视图 — 双列差异高亮显示
 *
 * 左右两列分别显示两份数据，通过颜色高亮标记:
 *   - 新增行(绿色背景)
 *   - 删除行(红色背景)
 *   - 修改行(黄色背景)
 *   - 未变化行(默认)
 *
 * 设计模式: 策略(Strategy) — 可替换diff算法
 * 协作: TerminalModel(数据源) / ProtocolView(协议对比)
 */
#ifndef DATA_DIFF_WIDGET_H
#define DATA_DIFF_WIDGET_H

#include <QWidget>
#include <QVector>

class QTextEdit;
class QLabel;
class QSplitter;

/// @brief 差异行类型
enum class DiffLineType {
    Unchanged,  ///< 未变化
    Added,      ///< 新增(左侧无，右侧有)
    Removed,    ///< 删除(左侧有，右侧无)
    Modified    ///< 修改(两侧都有但内容不同)
};

/// @brief 差异行数据
struct DiffLine {
    DiffLineType type;
    QString leftText;   ///< 左侧文本
    QString rightText;  ///< 右侧文本
    int lineNumber = 0; ///< 行号
};

/**
 * @brief 数据对比视图 — 双列LCS差异高亮
 */
class DataDiffWidget : public QWidget {
    Q_OBJECT

public:
    explicit DataDiffWidget(QWidget* parent = nullptr);

    /// @brief 设置对比数据并执行diff
    void setData(const QString& leftData, const QString& rightData);

    /// @brief 设置标签文本
    void setLeftLabel(const QString& label);
    void setRightLabel(const QString& label);

    /// @brief 清空对比数据
    void clear();

    /// @brief 差异统计
    int addedCount() const { return m_added; }
    int removedCount() const { return m_removed; }
    int modifiedCount() const { return m_modified; }

private:
    QVector<DiffLine> computeDiff(const QStringList& left, const QStringList& right) const;
    void refreshDisplay();
    QColor diffColor(DiffLineType type) const;

    QLabel* m_leftLabel = nullptr;      ///< objectName="diffLeftLabel"
    QLabel* m_rightLabel = nullptr;     ///< objectName="diffRightLabel"
    QTextEdit* m_leftEdit = nullptr;    ///< objectName="diffLeftEdit"
    QTextEdit* m_rightEdit = nullptr;   ///< objectName="diffRightEdit"
    QLabel* m_statsLabel = nullptr;     ///< objectName="diffStatsLabel"

    QVector<DiffLine> m_diffResult;     ///< diff结果缓存
    int m_added = 0;
    int m_removed = 0;
    int m_modified = 0;

    // ---- 统计计数器 ----
    quint64 m_totalComparisons = 0;     ///< 总对比次数
    quint64 m_totalDiffs = 0;           ///< 总差异数(新增+删除+修改累计)
    quint64 m_totalBytesCompared = 0;   ///< 总对比字节数

public:
    /** @brief 获取总对比次数 @return 对比计数 */
    quint64 totalComparisons() const { return m_totalComparisons; }
    /** @brief 获取总差异数 @return 差异计数 */
    quint64 totalDiffs() const { return m_totalDiffs; }
    /** @brief 获取总对比字节数 @return 字节计数 */
    quint64 totalBytesCompared() const { return m_totalBytesCompared; }
    /** @brief 重置对比统计计数器 */
    void resetDiffStatistics();
};

#endif // DATA_DIFF_WIDGET_H
