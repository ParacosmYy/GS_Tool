/**
 * @file PeakDetector.h
 * @brief 峰值检测引擎 — 幅度/局部极大/导数/显著性检测
 *
 * 功能: 4种峰值检测方法，支持最小间距/最小高度/显著性过滤，
 *       适用于波形分析和频谱峰值提取。
 *
 * 协作: RidgeDetector(脊线追踪) / FftEngine(频谱峰值)
 */
#ifndef PEAKDETECTOR_H
#define PEAKDETECTOR_H

#include <QObject>
#include <QVector>
#include <QList>

class PeakDetector : public QObject {
    Q_OBJECT

public:
    /** @brief 检测方法 */
    enum class DetectMethod {
        AmplitudeThreshold, ///< 幅度阈值
        LocalMaximum,       ///< 局部极大值
        Derivative,         ///< 导数零交叉
        Prominence          ///< 显著性检测
    };
    Q_ENUM(DetectMethod)

    /** @brief 峰值 */
    struct Peak {
        int index = 0;              ///< 峰值位置
        double value = 0.0;         ///< 峰值大小
        double width = 0.0;         ///< 峰宽
        double prominence = 0.0;    ///< 显著性
        double height = 0.0;        ///< 相对高度
    };

    /** @brief 统计 */
    struct Stats {
        quint64 totalScans = 0;             ///< 累计扫描次数
        quint64 totalPeaksFound = 0;        ///< 累计发现峰值数
        double  peakAverageHeight = 0.0;    ///< 平均峰值高度
        double  peakAverageProminence = 0.0;///< 平均显著性
        int     maxPeaksPerScan = 0;        ///< 单次扫描最大峰值数
    };

    explicit PeakDetector(QObject* parent = nullptr);

    void setMethod(DetectMethod method);
    void setMinHeight(double height);
    void setMinDistance(int distance);
    void setMinProminence(double prominence);

    QList<Peak> detect(const QVector<double>& data);
    QList<Peak> detectInRange(const QVector<double>& data, int start, int end);

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void peaksDetected(int count);

private:
    QList<Peak> detectAmplitude(const QVector<double>& data, int start, int end);
    QList<Peak> detectLocalMax(const QVector<double>& data, int start, int end);
    QList<Peak> detectDerivative(const QVector<double>& data, int start, int end);
    QList<Peak> detectProminence(const QVector<double>& data, int start, int end);

    double computeProminence(const QVector<double>& data, int peakIdx) const;
    QList<Peak> mergeClosePeaks(const QList<Peak>& peaks) const;

    DetectMethod m_method;
    double m_minHeight;
    int m_minDistance;
    double m_minProminence;

    double m_heightSum;
    double m_prominenceSum;
    Stats m_stats;
};

#endif // PEAKDETECTOR_H
