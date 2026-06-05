/**
 * @file FirFilterDesigner.h
 * @brief FIR滤波器设计器 — 窗口法/频率采样法
 *
 * 功能: 设计低通/高通/带通/带阻FIR滤波器，支持
 *       矩形/Hamming/Hanning/Blackman/Kaiser窗，
 *       统计设计次数/阶数/耗时。
 */
#ifndef FIRFILTERDESIGNER_H
#define FIRFILTERDESIGNER_H

#include <QObject>
#include <QVector>

/**
 * @class FirFilterDesigner
 * @brief FIR数字滤波器设计工具
 */
class FirFilterDesigner : public QObject {
    Q_OBJECT
public:
    /** 滤波器类型 */
    enum class FilterType { LowPass, HighPass, BandPass, BandStop };
    /** 窗函数类型 */
    enum class WindowType { Rectangular, Hamming, Hanning, Blackman, Kaiser };

    /** 设计结果 */
    struct DesignResult {
        QVector<double> coefficients;   ///< 滤波器系数
        double actualRippleDb = 0.0;    ///< 实际纹波(dB)
        double cutoffNorm = 0.0;        ///< 归一化截止频率
    };

    /** 设计统计 */
    struct Stats {
        quint64 totalDesigns = 0;
        quint64 totalCoefficientsGenerated = 0;
        double  averageProcessingTimeMs = 0.0;
    };

    explicit FirFilterDesigner(QObject* parent = nullptr);

    /** @brief 设计滤波器 @param type 类型 @param order 阶数 @param cutoffNorm 归一化截止(0~1) @param window 窗函数 @return 设计结果 */
    DesignResult design(FilterType type, int order, double cutoffNorm,
                        WindowType window) const;

    /** @brief 设计带通/带阻 @param type 类型 @param order 阶数 @param lowNorm 低截止 @param highNorm 高截止 @param window 窗函数 @return 结果 */
    DesignResult designBand(FilterType type, int order,
                            double lowNorm, double highNorm,
                            WindowType window) const;

    /** @brief Kaiser窗参数估算 @param rippleDb 纹波(dB) @param transitionWidth 过渡带宽 @return (阶数, beta) */
    QPair<int, double> kaiserParams(double rippleDb, double transitionWidth) const;

    /** @brief 生成窗函数 @param length 长度 @param type 窗类型 @param beta Kaiser参数 @return 窗序列 */
    QVector<double> generateWindow(int length, WindowType type,
                                   double beta = 0.0) const;

    /** @brief 频率响应 @param coeffs 系数 @param numPoints 频率点数 @return (频率, 幅度dB) */
    QVector<QPair<double, double>> frequencyResponse(
        const QVector<double>& coeffs, int numPoints = 512) const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void designCompleted(int order, double cutoffNorm);

private:
    /** sinc函数 */
    static double sinc(double x);
    /** 应用窗函数到理想脉冲响应 */
    QVector<double> applyWindow(const QVector<double>& ideal,
                                WindowType type, double beta) const;

    mutable Stats m_stats;
    mutable double m_timeSum;
};

#endif // FIRFILTERDESIGNER_H
