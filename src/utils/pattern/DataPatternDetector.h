/**
 * @file DataPatternDetector.h
 * @brief 数据模式检测器 — 自动发现数据流中的重复模式
 *
 * 功能: 分析数据流，自动检测重复出现的字节序列模式，
 *       报告模式频率/长度/位置，支持最小长度过滤和模式锁定。
 *
 * 协作: TerminalWidget(实时数据流) / TriggerEngine(模式触发)
 */
#ifndef DATAPATTERNDETECTOR_H
#define DATAPATTERNDETECTOR_H

#include <QObject>
#include <QMap>
#include <QList>
#include <QByteArray>

/**
 * @brief 数据模式检测器 — 自动发现数据流中的重复字节序列
 */
class DataPatternDetector : public QObject {
    Q_OBJECT

public:
    /** @brief 检测到的模式 */
    struct DetectedPattern {
        QByteArray pattern;         ///< 模式字节序列
        int occurrences = 0;        ///< 出现次数
        int firstOffset = -1;       ///< 首次出现偏移
        int lastOffset = -1;        ///< 最后出现偏移
        double frequency = 0.0;     ///< 出现频率(次/KB)
    };

    /** @brief 统计 */
    struct Stats {
        quint64 totalBytesAnalyzed = 0; ///< 累计分析字节数
        quint64 totalPatternsFound = 0; ///< 累计发现模式数
        quint64 totalScans = 0;         ///< 累计扫描次数
        int     peakPatterns = 0;       ///< 峰值模式数
        int     longestPattern = 0;     ///< 最长模式长度
        quint64 totalDetections = 0;    ///< 累计检测事件数
    };

    explicit DataPatternDetector(QObject* parent = nullptr);

    /** @brief 设置最小模式长度 @param minLen 最小字节长度(默认2) */
    void setMinPatternLength(int minLen);
    /** @brief 设置最大模式长度 @param maxLen 最大字节长度(默认16) */
    void setMaxPatternLength(int maxLen);
    /** @brief 设置最小出现次数过滤 @param minCount 最小次数(默认2) */
    void setMinOccurrences(int minCount);

    /** @brief 扫描数据中的重复模式 @param data 待扫描数据 */
    void scan(const QByteArray& data);

    /** @brief 获取检测到的模式(按频率降序) @return 模式列表 */
    QList<DetectedPattern> detectedPatterns() const;

    /** @brief 清除所有检测数据 */
    void clear();

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    /** @brief 模式被检测到 @param pattern 模式数据 @param count 出现次数 */
    void patternDetected(const QByteArray& pattern, int count);
    /** @brief 扫描完成 @param patternCount 发现的模式数 */
    void scanCompleted(int patternCount);

private:
    void buildSuffixArray(const QByteArray& data);
    void findPatterns(const QByteArray& data);

    int m_minPatternLen;    ///< 最小模式长度
    int m_maxPatternLen;    ///< 最大模式长度
    int m_minOccurrences;   ///< 最小出现次数

    QMap<QByteArray, DetectedPattern> m_patterns; ///< 检测到的模式

    Stats m_stats;
};

#endif // DATAPATTERNDETECTOR_H
