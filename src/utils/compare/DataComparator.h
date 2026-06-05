/**
 * @file DataComparator.h
 * @brief 数据缓冲区字节级比较引擎
 * @author Serial Tool Team
 * @date 2026-06-05
 *
 * 基于 LCS（最长公共子序列）算法对两个 QByteArray 执行字节级差异分析，
 * 输出 DiffBlock 列表、相似度百分比、Hex 并排对比和 CSV 导出。
 */

#ifndef DATACOMPARATOR_H
#define DATACOMPARATOR_H

#include <QByteArray>
#include <QList>
#include <QObject>
#include <QString>
#include <QtGlobal>

/**
 * @class DataComparator
 * @brief 字节级数据比较引擎
 *
 * 对两个数据缓冲区执行 LCS 最长公共子序列分析，生成结构化差异块列表。
 * 提供 similarity 百分比、Hex 并排对比文本和 CSV 差异导出。
 * 统计计数器跟踪累计比较次数、字节数和相似度极值。
 */
class DataComparator : public QObject
{
    Q_OBJECT

public:
    /** @brief 差异块类型 */
    enum class DiffType {
        Equal,      ///< 字节完全匹配
        Different,  ///< 同偏移字节不同
        Inserted,   ///< 仅存在于数据 B（相对 A 为插入）
        Deleted,    ///< 仅存在于数据 A（相对 B 为删除）
        Modified    ///< 区段内包含混合差异
    };
    Q_ENUM(DiffType)

    /** @brief 差异块：描述一段连续的差异区间 */
    struct DiffBlock {
        int offsetA = 0;         ///< 差异在数据 A 中的起始偏移
        int offsetB = 0;         ///< 差异在数据 B 中的起始偏移
        int length = 0;          ///< 差异区间的字节长度
        DiffType type = DiffType::Equal;   ///< 差异类型
        QByteArray dataA;        ///< 数据 A 中该区间的原始字节
        QByteArray dataB;        ///< 数据 B 中该区间的原始字节
    };

    /** @brief 累计统计计数器 */
    struct Stats {
        quint64 totalComparisons = 0;      ///< 累计比较调用次数
        quint64 totalEqualBytes = 0;       ///< 累计匹配字节数
        quint64 totalDifferentBytes = 0;   ///< 累计不同字节数
        quint64 totalInsertedBytes = 0;    ///< 累计插入字节数
        quint64 totalDeletedBytes = 0;     ///< 累计删除字节数
        quint64 avgSimilarity = 0;         ///< 平均相似度（0~100，整数百分比）
        quint64 minDataSize = 0;           ///< 历史最小输入数据长度
        quint64 maxDataSize = 0;           ///< 历史最大输入数据长度
    };

    /** @brief 构造数据比较引擎 @param parent 父对象 */
    explicit DataComparator(QObject *parent = nullptr);

    /**
     * @brief 执行字节级 LCS 比较并生成差异块列表
     * @param dataA 数据缓冲区 A
     * @param dataB 数据缓冲区 B
     * @return 差异块列表，按偏移升序排列
     *
     * 空输入返回空列表。比较完成后发射 comparisonComplete 信号，
     * 每个差异块逐个发射 diffDetected 信号。
     */
    QList<DiffBlock> compare(const QByteArray &dataA, const QByteArray &dataB);

    /**
     * @brief 计算两个数据缓冲区的相似度百分比
     * @param dataA 数据缓冲区 A
     * @param dataB 数据缓冲区 B
     * @return 相似度 (0.0 ~ 100.0)，双空输入返回 100.0
     *
     * 基于 LCS 匹配字节数 / max(lenA, lenB) 计算。
     */
    double similarity(const QByteArray &dataA, const QByteArray &dataB) const;

    /**
     * @brief 获取最近一次比较的差异块列表
     * @return 差异块列表，未执行过比较时为空
     */
    QList<DiffBlock> diffBlocks() const;

    /**
     * @brief 将差异导出为 CSV 格式文本
     * @return CSV 文本，包含表头行 (OffsetA, OffsetB, Length, Type, HexA, HexB)
     */
    QString exportDiff() const;

    /**
     * @brief 生成 Hex 并排对比文本
     * @param bytesPerLine 每行显示的字节数（默认 16）
     * @return 多行文本，左侧数据 A、右侧数据 B，差异字节以标记区分
     */
    QString exportHexDiff(int bytesPerLine = 16) const;

    /**
     * @brief 获取累计统计计数器快照
     * @return 统计结构体副本
     */
    Stats stats() const;

    /** @brief 重置所有累计统计计数器为初始值 */
    void resetStatistics();

signals:
    /**
     * @brief 比较完成时发射
     * @param similarity 相似度百分比 (0.0 ~ 100.0)
     */
    void comparisonComplete(double similarity);

    /**
     * @brief 检测到差异块时逐个发射
     * @param block 差异块详情
     */
    void diffDetected(const DiffBlock &block);

private:
    /**
     * @brief 构建 LCS 回溯表并提取匹配对
     * @param dataA 数据 A
     * @param dataB 数据 B
     * @return 匹配对的列表，每对 (idxA, idxB) 表示 A[i]==B[j]
     */
    QList<QPair<int, int>> computeLcs(const QByteArray &dataA,
                                       const QByteArray &dataB) const;

    /**
     * @brief 根据 LCS 匹配对生成差异块列表
     * @param matches LCS 匹配对列表
     * @param dataA 数据 A
     * @param dataB 数据 B
     * @return 差异块列表
     */
    QList<DiffBlock> buildDiffBlocks(const QList<QPair<int, int>> &matches,
                                      const QByteArray &dataA,
                                      const QByteArray &dataB) const;

    QList<DiffBlock> m_diffBlocks;   ///< 最近一次比较的差异块列表
    Stats m_stats;                    ///< 累计统计计数器
    double m_similaritySum = 0.0;    ///< 累计相似度总和（用于计算平均值）
};

#endif // DATACOMPARATOR_H
