/**
 * @file WalshHadamard.h
 * @brief Walsh-Hadamard变换 — 快速WHT+序列序Walsh函数+信号相关
 *
 * 功能: 快速Walsh-Hadamard变换(FWHT)，支持自然序和序列序(Paley/Hz)
 *       Walsh函数，Hadamard矩阵生成，信号自相关/互相关分析。
 *
 * 协作: SpectrumAnalyzer(频谱分析) / CrossCorrelator(互相关)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QList>

/**
 * @brief Walsh-Hadamard变换引擎
 *
 * Walsh-Hadamard变换是一种正交变换，仅使用+1/-1运算，
 * 计算复杂度O(N log N)，适用于信号压缩、纠错编码和模式识别。
 */
class WalshHadamard : public QObject {
    Q_OBJECT

public:
    /** @brief 排序模式 */
    enum class Ordering {
        Natural,        ///< 自然序(Hadamard序)
        Sequency,       ///< 序列序(Walsh序，按零交叉数排列)
        Dyadic          ///< 二进制序(Paley序)
    };
    Q_ENUM(Ordering)

    /** @brief 相关结果 */
    struct CorrelationResult {
        int lag = 0;                ///< 延迟位置
        double value = 0.0;         ///< 相关系数
    };

    /** @brief 统计信息 */
    struct Stats {
        quint64 totalTransforms = 0;            ///< 累计变换次数
        quint64 totalPointsProcessed = 0;       ///< 累计处理数据点数
        double  avgProcessingTimeMs = 0.0;      ///< 平均处理时间(ms)
    };

    explicit WalshHadamard(QObject* parent = nullptr);

    void setOrdering(Ordering ordering);

    QVector<double> forward(const QVector<double>& input);
    QVector<double> inverse(const QVector<double>& coefficients);

    QVector<QVector<double>> hadamardMatrix(int order) const;
    QVector<double> walshFunction(int index, int length) const;

    QList<CorrelationResult> correlate(const QVector<double>& a,
                                       const QVector<double>& b) const;
    QList<CorrelationResult> autoCorrelate(const QVector<double>& signal) const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void transformCompleted(int size);

private:
    void fastWHT(QVector<double>& data) const;
    void reorderToSequency(QVector<double>& data) const;
    void reorderToDyadic(QVector<double>& data) const;
    int bitReverse(int value, int bits) const;
    int grayCode(int value) const;
    int sequencyIndex(int index, int bits) const;

    Ordering m_ordering;     ///< 排序模式

    Stats m_stats;
    double m_timeSum = 0.0;
};
