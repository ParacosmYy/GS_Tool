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

/** @brief 差异行类型枚举 */
enum class DiffLineType {
    Unchanged,  ///< 未变化
    Added,      ///< 新增(左侧无，右侧有)
    Removed,    ///< 删除(左侧有，右侧无)
    Modified    ///< 修改(两侧都有但内容不同)
};

/** @brief 差异行数据结构 */
struct DiffLine {
    DiffLineType type;   ///< 行差异类型
    QString leftText;    ///< 左侧文本
    QString rightText;   ///< 右侧文本
    int lineNumber = 0;  ///< 行号
};

/**
 * @brief 数据对比视图 — 双列LCS差异高亮
 */
class DataDiffWidget : public QWidget {
    Q_OBJECT

public:
    /** @brief 构造数据对比视图 @param parent 父控件 */
    explicit DataDiffWidget(QWidget* parent = nullptr);

    /** @brief 设置对比数据并执行diff @param leftData 左侧原始数据 @param rightData 右侧修改数据 */
    void setData(const QString& leftData, const QString& rightData);

    /** @brief 设置左侧标签文本 @param label 标签字符串 */
    void setLeftLabel(const QString& label);
    /** @brief 设置右侧标签文本 @param label 标签字符串 */
    void setRightLabel(const QString& label);

    /** @brief 清空对比数据 */
    void clear();

    /** @brief 获取新增行数 @return 新增行计数 */
    int addedCount() const { return m_added; }
    /** @brief 获取删除行数 @return 删除行计数 */
    int removedCount() const { return m_removed; }
    /** @brief 获取修改行数 @return 修改行计数 */
    int modifiedCount() const { return m_modified; }

private:
    /** @brief 使用LCS算法计算差异 @param left 左侧行列表 @param right 右侧行列表 @return 差异行列表 */
    QVector<DiffLine> computeDiff(const QStringList& left, const QStringList& right) const;
    /** @brief 刷新左右编辑器显示 */
    void refreshDisplay();
    /** @brief 获取差异类型对应的语义颜色 @param type 差异类型 @return 颜色值 */
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
