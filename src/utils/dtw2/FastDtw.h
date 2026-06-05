/**
 * @file FastDtw.h
 * @brief 快速DTW距离计算 — 多尺度动态时间规整
 *
 * 功能: 支持标准DTW/FastDTW(多尺度近似)/约束DTW(带状约束)，
 *       统计计算次数/平均距离/平均路径长度。
 */
#ifndef FASTDTW_H
#define FASTDTW_H

#include <QObject>
#include <QVector>
#include <QList>
#include <QPair>

/**
 * @class FastDtw
 * @brief 高效DTW距离计算器，支持多尺度加速
 */
class FastDtw : public QObject {
    Q_OBJECT
public:
    /** DTW模式 */
    enum class Mode {
        Standard,   ///< 标准DTW O(n²)
        FastDtw,    ///< 多尺度近似DTW
        Constrained ///< Sakoe-Chiba带状约束
    };

    /** DTW结果 */
    struct DtwResult {
        double distance = 0.0;                      ///< DTW距离
        QList<QPair<int, int>> warpPath;             ///< 对齐路径
        double normalizedDistance = 0.0;              ///< 归一化距离
        double averageProcessingTimeMs = 0.0;
    };

    /** 计算统计 */
    struct Stats {
        quint64 totalComputations = 0;
        double  avgDistance = 0.0;
        double  avgPathLength = 0.0;
        double  averageProcessingTimeMs = 0.0;
    };

    explicit FastDtw(QObject* parent = nullptr);

    void setMode(Mode m);
    void setRadius(int r);
    void setDistanceMetric(const QString& metric);

    /** 计算DTW */
    DtwResult compute(const QVector<double>& seq1, const QVector<double>& seq2);

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void computationComplete(double distance, int pathLength);

private:
    DtwResult standardDtw(const QVector<double>& s1, const QVector<double>& s2);
    DtwResult fastDtwImpl(const QVector<double>& s1, const QVector<double>& s2, int radius);
    DtwResult constrainedDtw(const QVector<double>& s1, const QVector<double>& s2);

    double pointDistance(double a, double b) const;
    QList<QPair<int, int>> traceback(const QVector<QVector<int>>& path,
                                      int n, int m) const;

    Mode m_mode;
    int m_radius;
    QString m_metric;
    Stats m_stats;
    double m_timeSum;
};

#endif // FASTDTW_H
