/**
 * @file DataSmoother.h
 * @brief 数据平滑器 — 多种平滑算法去除噪声
 *
 * 功能: Savitzky-Golay/指数/高斯/双边滤波，保留信号特征的同时去除噪声。
 */
#ifndef DATASMOOTHER_H
#define DATASMOOTHER_H

#include <QObject>
#include <QVector>

class DataSmoother : public QObject {
    Q_OBJECT
public:
    enum class SmoothMethod {
        Exponential,    ///< 指数加权移动平均
        Gaussian,       ///< 高斯加权
        SavitzkyGolay,  ///< Savitzky-Golay多项式拟合
        Triangular      ///< 三角加权
    };
    Q_ENUM(SmoothMethod)

    struct Stats {
        quint64 totalSmoothOps = 0;
        quint64 totalPointsProcessed = 0;
        double  averageSmoothFactor = 0.0;
    };

    explicit DataSmoother(QObject* parent = nullptr);

    void setMethod(SmoothMethod method);
    void setWindowSize(int size);
    void setAlpha(double alpha);

    QVector<double> smooth(const QVector<double>& data);
    double smoothOne(double value);

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void smoothComplete(int count);

private:
    QVector<double> smoothExponential(const QVector<double>& data);
    QVector<double> smoothGaussian(const QVector<double>& data);
    QVector<double> smoothTriangular(const QVector<double>& data);

    SmoothMethod m_method;
    int m_windowSize;
    double m_alpha;
    double m_prevOutput;
    bool m_initialized;

    Stats m_stats;
};

#endif // DATASMOOTHER_H
