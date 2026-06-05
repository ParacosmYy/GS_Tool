/**
 * @file SavitzkyGolay.h
 * @brief Savitzky-Golay滤波器 — 多项式平滑微分
 *
 * 功能: 基于局部多项式最小二乘拟合的平滑与微分滤波器。
 *       支持任意窗口大小、多项式阶数和微分阶数。
 *       适用于信号平滑、一阶/二阶微分计算等。
 *
 * 协作: MedianFilter(脉冲去噪) / DigitalFilter(一般滤波)
 */
#ifndef SAVITZKYGOLAY_H
#define SAVITZKYGOLAY_H

#include <QObject>
#include <QVector>

/**
 * @brief Savitzky-Golay滤波器 — 多项式平滑微分
 */
class SavitzkyGolay : public QObject {
    Q_OBJECT

public:
    /** @brief 统计信息 */
    struct Stats {
        quint64 totalDesigns = 0;          ///< 累计设计次数
        quint64 totalApplications = 0;     ///< 累计应用次数
        double  avgProcessingTimeMs = 0.0; ///< 平均处理时间(ms)
    };

    explicit SavitzkyGolay(QObject* parent = nullptr);

    /** @brief 设计Savitzky-Golay滤波器系数
     *  @param windowSize 窗口大小(必须为奇数, >= 3)
     *  @param polyOrder 多项式阶数(必须 < windowSize)
     *  @param derivative 微分阶数(0=平滑, 1=一阶导, 2=二阶导)
     *  @return 卷积系数 */
    QVector<double> design(int windowSize, int polyOrder, int derivative = 0);

    /** @brief 应用S-G滤波器
     *  @param signal 输入信号
     *  @return 滤波后信号 */
    QVector<double> apply(const QVector<double>& signal);

    /** @brief 获取当前滤波器系数 @return 系数 */
    QVector<double> coefficients() const { return m_coeffs; }

    /** @brief 获取当前窗口大小 @return 窗口大小 */
    int windowSize() const { return m_windowSize; }

    /** @brief 获取统计 */
    const Stats& stats() const { return m_stats; }

    /** @brief 重置统计 */
    void resetStatistics();

signals:
    /** @brief 滤波器设计完成 @param windowSize 窗口大小 @param polyOrder 阶数 */
    void designCompleted(int windowSize, int polyOrder);

    /** @brief 滤波应用完成 @param inputSize 输入信号长度 */
    void applicationCompleted(int inputSize);

private:
    /** @brief 构建Vandermonde矩阵并求解最小二乘 @return 卷积系数 */
    QVector<double> computeCoeffs(int halfWin, int polyOrder, int deriv) const;

    QVector<double> m_coeffs;      ///< 当前卷积系数
    int m_windowSize;              ///< 当前窗口大小
    double m_timeSum;              ///< 处理时间累加器
    Stats  m_stats;                ///< 统计信息
};

#endif // SAVITZKYGOLAY_H
