/**
 * @file WindowFunctionLib.h
 * @brief 窗函数库 — 信号处理窗函数集合
 *
 * 功能: 提供Hanning/Hamming/Blackman/Kaiser/FlatTop/Gaussian等
 *       多种窗函数，支持窗函数设计/应用，统计调用次数。
 */
#ifndef WINDOWFUNCTIONLIB_H
#define WINDOWFUNCTIONLIB_H

#include <QObject>
#include <QVector>

class WindowFunctionLib : public QObject {
    Q_OBJECT
public:
    enum Type {
        Hanning, Hamming, Blackman, BlackmanHarris,
        Kaiser, FlatTop, Gaussian, Tukey, Bartlett, Rectangular
    };

    struct Stats {
        quint64 totalCreated = 0;
        quint64 totalApplied = 0;
        double  avgProcessingTimeMs = 0.0;
    };

    explicit WindowFunctionLib(QObject* parent = nullptr);

    /** @brief 创建窗函数 @param type 类型 @param length 长度 @param param 额外参数(Kaiser的beta/Gaussian的sigma/Tukey的alpha) */
    QVector<double> create(Type type, int length, double param = 0.0) const;

    /** @brief 应用窗函数 @param signal 信号 @param window 窗函数 @return 加窗信号 */
    QVector<double> apply(const QVector<double>& signal,
                          const QVector<double>& window) const;

    /** @brief 窗函数名称 @param type 类型 @return 名称字符串 */
    static QString typeName(Type type);

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

private:
    mutable Stats m_stats;
    mutable double m_timeSum;
};

#endif // WINDOWFUNCTIONLIB_H
