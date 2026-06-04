/**
 * @file DataCompareWidget.h
 * @brief 数据字节级对比控件
 * @author Serial Tool Team
 * @date 2026-06-05
 *
 * 提供两个数据缓冲区的逐字节对比视图，高亮差异字节，
 * 支持差异导航、结果复制和统计分析。
 */

#ifndef DATACOMPAREWIDGET_H
#define DATACOMPAREWIDGET_H

#include <QByteArray>
#include <QLabel>
#include <QList>
#include <QObject>
#include <QPushButton>
#include <QSplitter>
#include <QTableWidget>
#include <QTextEdit>
#include <QWidget>

/**
 * @class DataCompareWidget
 * @brief 字节级数据对比控件
 *
 * 左侧为两个 QTextEdit 输入区域（Hex 格式），
 * 右侧为结果表格，逐字节显示 Offset/Hex A/Hex B/Type，
 * 并提供差异导航（上一个/下一个）和结果复制功能。
 */
class DataCompareWidget : public QWidget
{
    Q_OBJECT

public:
    /** @brief 差异类型枚举 */
    enum DiffType {
        Match,     ///< 字节匹配
        Different, ///< 字节不同
        OnlyInA,   ///< 仅存在于数据A
        OnlyInB    ///< 仅存在于数据B
    };
    Q_ENUM(DiffType)

    /** @brief 单字节差异条目 */
    struct DiffEntry {
        int offset = 0;        ///< 偏移量
        DiffType type = Match; ///< 差异类型
        quint8 byteA = 0;      ///< 数据A中的字节值
        quint8 byteB = 0;      ///< 数据B中的字节值
    };

    /** @brief 对比结果汇总 */
    struct CompareResult {
        int totalBytes = 0;       ///< 总字节数（取较长者）
        int matchingBytes = 0;    ///< 匹配字节数
        int differentBytes = 0;   ///< 不同字节数
        int onlyInA = 0;          ///< 仅在A中的字节数
        int onlyInB = 0;          ///< 仅在B中的字节数
        double similarity = 0.0;  ///< 相似度百分比 (0~100)
        QList<DiffEntry> diffs;   ///< 所有差异条目
    };

    /** @brief 累计统计数据 */
    struct Stats {
        quint64 totalCompares = 0;       ///< 累计对比次数
        quint64 totalBytesCompared = 0;  ///< 累计对比字节数
        quint64 totalDiffsFound = 0;     ///< 累计发现差异数
        quint64 totalNavigations = 0;    ///< 累计差异导航次数
        quint64 totalCopies = 0;         ///< 累计复制结果次数
        double peakSimilarity = 0.0;     ///< 历史最高相似度
        double lowestSimilarity = 100.0; ///< 历史最低相似度
    };

    /** @brief 构造数据对比控件 @param parent 父控件 */
    explicit DataCompareWidget(QWidget *parent = nullptr);

    /** @brief 设置待对比的数据 @param dataA 数据A @param dataB 数据B */
    void setCompareData(const QByteArray &dataA, const QByteArray &dataB);

    /** @brief 执行字节级对比并填充结果表格 @return 对比结果 */
    CompareResult compare();

    /** @brief 获取最近一次对比结果 @return 对比结果 */
    CompareResult result() const;

    /** @brief 导航到下一个差异位置 @return 差异偏移量，无差异返回-1 */
    int nextDiff();

    /** @brief 导航到上一个差异位置 @return 差异偏移量，无差异返回-1 */
    int prevDiff();

    /** @brief 获取累计统计数据 @return 统计快照 */
    Stats stats() const;

    /** @brief 重置所有累计统计计数器 */
    void resetStatistics();

signals:
    /** @brief 对比完成时发射 @param result 对比结果 */
    void compareCompleted(CompareResult result);

    /** @brief 差异导航时发射 @param offset 当前差异偏移量 */
    void diffNavigated(int offset);

private:
    /** @brief 初始化界面布局和控件 */
    void setupUI();

    /** @brief 根据对比结果填充结果表格 */
    void updateResultTable();

    /** @brief 更新底部摘要标签 */
    void updateSummary();

    /** @brief "对比"按钮点击槽函数：解析 Hex 文本并执行对比 */
    void onCompareClicked();

    /** @brief "复制结果"按钮点击槽函数：格式化结果并复制到剪贴板 */
    void onCopyResult();

    QTextEdit *m_dataAEdit = nullptr;   ///< 数据A的Hex输入编辑器
    QTextEdit *m_dataBEdit = nullptr;   ///< 数据B的Hex输入编辑器
    QPushButton *m_compareBtn = nullptr;   ///< 对比按钮
    QPushButton *m_nextDiffBtn = nullptr;  ///< 下一个差异按钮
    QPushButton *m_prevDiffBtn = nullptr;  ///< 上一个差异按钮
    QPushButton *m_copyBtn = nullptr;      ///< 复制结果按钮
    QTableWidget *m_resultTable = nullptr; ///< 对比结果表格
    QLabel *m_summaryLabel = nullptr;      ///< 摘要信息标签

    QByteArray m_dataA;              ///< 数据A原始字节
    QByteArray m_dataB;              ///< 数据B原始字节
    CompareResult m_result;          ///< 最近一次对比结果
    int m_currentDiffIndex = -1;     ///< 当前差异导航索引
    Stats m_stats;                   ///< 累计统计数据
};

#endif // DATACOMPAREWIDGET_H
