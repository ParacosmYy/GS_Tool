/**
 * @file ByteFrequencyAnalyzer.h
 * @brief 字节频率分析器 -- 分析串口数据流中各字节值的分布、香农熵和重复模式
 * @author EmbedDebug Team
 * @date 2026-06-05
 *
 * 对输入的二进制数据进行 256 级字节值计数，计算频率、累积分布、
 * 香农信息熵，并检测超过最短长度的重复字节序列(主导模式)。
 * 支持 CSV 直方图导出和累计统计查询。
 */

#ifndef BYTEFREQUENCYANALYZER_H
#define BYTEFREQUENCYANALYZER_H

#include <QByteArray>
#include <QList>
#include <QObject>
#include <QString>
#include <QVector>
#include <QtGlobal>

/**
 * @class ByteFrequencyAnalyzer
 * @brief 字节值分布分析引擎
 *
 * 典型用法:
 * @code
 *   ByteFrequencyAnalyzer analyzer;
 *   auto result = analyzer.analyze(data);
 *   double entropy = result.shannonEntropy;
 *   auto top5 = analyzer.topFrequencies(5);
 *   auto csv  = analyzer.exportHistogram();
 * @endcode
 */
class ByteFrequencyAnalyzer : public QObject
{
    Q_OBJECT

public:
    /** @brief 单个字节值的统计信息 */
    struct ByteStats {
        quint64 count        = 0;  ///< 出现次数
        double  frequency    = 0;  ///< 频率(0.0~1.0)
        double  cumulativeFreq = 0; ///< 累积频率
        double  entropy      = 0;  ///< 该字节的局部信息量 -log2(p)，p=0时为0
    };

    /** @brief 一次分析的综合结果 */
    struct AnalysisResult {
        int       uniqueBytes        = 0;   ///< 数据中出现的不同字节值数量(0~256)
        double    shannonEntropy     = 0.0; ///< 香农信息熵(bit)
        double    maxEntropy         = 0.0; ///< 最大可能熵(bit)，即 log2(256)=8
        double    normalizedEntropy  = 0.0; ///< 归一化熵(0.0~1.0)
        quint64   totalBytes         = 0;   ///< 本次分析的总字节数
        quint64   mostFrequentByte   = 0;   ///< 出现次数最多的字节值
        quint64   leastFrequentByte  = 0;   ///< 出现次数最少的字节值(仅统计出现过的)
        QByteArray dominantPattern;         ///< 检测到的主导重复字节模式
    };

    /** @brief 累计统计数据 */
    struct Stats {
        quint64 totalAnalyses         = 0; ///< 累计分析次数
        quint64 totalBytesAnalyzed     = 0; ///< 累计分析字节数
        quint64 totalPatternsFound     = 0; ///< 累计检测到的重复模式数
        double  avgEntropy             = 0; ///< 历史平均香农熵
        quint64 maxBytesInSingleAnalysis = 0; ///< 单次分析最大字节数
    };

    /** @brief 构造字节频率分析器 @param parent 父对象 */
    explicit ByteFrequencyAnalyzer(QObject *parent = nullptr);

    // ---- 核心分析 ----

    /**
     * @brief 对数据进行完整的字节频率分析
     *
     * 统计 256 个字节值的出现次数，计算频率、累积频率、香农熵，
     * 并更新最近一次分析结果。
     *
     * @param data 待分析的数据(可为空，此时返回零值结果)
     * @return 分析结果快照
     */
    AnalysisResult analyze(const QByteArray &data);

    /**
     * @brief 查询指定字节值的频率统计
     * @param byteValue 字节值(0~255)
     * @return 该字节的统计信息，未分析过或字节值越界时返回零值
     */
    ByteStats byteFrequency(int byteValue) const;

    /**
     * @brief 获取出现频率最高的 N 个字节值(按计数降序)
     * @param count 返回数量，超过 256 时截断为 256
     * @return ByteStats 列表，附带字节值存于用户数据中；此处返回 (byteValue, ByteStats) 对
     */
    QList<QPair<int, ByteStats>> topFrequencies(int count) const;

    /** @brief 获取最近一次分析结果 @return AnalysisResult 快照 */
    AnalysisResult analysisResult() const;

    // ---- 导出 ----

    /**
     * @brief 将字节频率直方图导出为 CSV 格式
     *
     * 列: ByteValue(0~255), Count, Frequency, CumulativeFrequency
     *
     * @return CSV 格式字符串(含表头)
     */
    QString exportHistogram() const;

    // ---- 模式检测 ----

    /**
     * @brief 检测数据中重复出现的字节序列(主导模式)
     *
     * 扫描数据，查找长度 >= minLength 的连续重复子序列，
     * 返回重复次数最多的子序列。
     *
     * @param minLength 最短模式长度(字节数)，小于 2 时强制为 2
     * @return 检测到的重复模式，未找到返回空 QByteArray
     */
    QByteArray detectPattern(int minLength) const;

    // ---- 统计 ----

    /** @brief 获取累计统计数据快照 @return Stats 结构体副本 */
    Stats stats() const;

    /** @brief 重置所有累计统计计数器和分析状态 */
    void resetStatistics();

signals:
    /** @brief 分析完成时发射 @param result 本次分析结果 */
    void analysisComplete(const ByteFrequencyAnalyzer::AnalysisResult &result);

    /** @brief 检测到重复模式时发射 @param pattern 检测到的重复字节序列 */
    void patternDetected(const QByteArray &pattern);

private:
    /**
     * @brief 计算当前字节计数的香农熵
     * @return 香农熵(bit)，数据为空时返回 0
     */
    double computeShannonEntropy() const;

    /**
     * @brief 在最近分析的数据中查找长度 >= minLength 的重复子序列
     * @param minLength 最短模式长度
     * @return 重复次数最多的子序列，未找到返回空
     */
    QByteArray findDominantPattern(int minLength) const;

    QVector<quint64> m_byteCounts;       ///< 256 个字节值的出现计数
    QByteArray       m_lastData;          ///< 最近一次分析的原始数据(用于模式检测)
    AnalysisResult   m_lastResult;        ///< 最近一次分析结果
    Stats            m_stats;             ///< 累计统计
};

#endif // BYTEFREQUENCYANALYZER_H
