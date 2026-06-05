/**
 * @file NumericalDerivative.h
 * @brief 数值微分引擎 — 一阶/二阶/高阶微分计算
 *
 * 功能: 支持前向/后向/中心差分和Savitzky-Golay滤波微分，
 *       可选平滑窗口以减少噪声放大。
 *
 * 协作: PeakDetector(峰值检测) / DataTransformer(预处理)
 */
#ifndef NUMERICALDERIVATIVE_H
#define NUMERICALDERIVATIVE_H

#include <QObject>
#include <QVector>

class NumericalDerivative : public QObject {
    Q_OBJECT

public:
    /** @brief 差分方法 */
    enum class DiffMethod {
        Forward,        ///< 前向差分
        Backward,       ///< 后向差分
        Central,        ///< 中心差分
        SavitzkyGolay   ///< Savitzky-Golay微分
    };
    Q_ENUM(DiffMethod)

    /** @brief 统计 */
    struct Stats {
        quint64 totalDerivatives = 0;       ///< 累计微分次数
        quint64 totalPointsProcessed = 0;   ///< 累计处理点数
        double  peakDerivative = 0.0;       ///< 峰值导数
        double  averageDerivative = 0.0;    ///< 平均导数绝对值
    };

    explicit NumericalDerivative(QObject* parent = nullptr);

    /** @brief 设置差分方法 @param method 方法 */
    void setMethod(DiffMethod method);

    /** @brief 设置采样间隔 @param dx 间隔 */
    void setDx(double dx);

    /** @brief 计算一阶微分 @param data 数据 @return 导数 */
    QVector<double> derivative(const QVector<double>& data);

    /** @brief 计算二阶微分 @param data 数据 @return 二阶导数 */
    QVector<double> secondDerivative(const QVector<double>& data);

    /** @brief 计算N阶微分 @param data 数据 @param order 阶数 @return N阶导数 */
    QVector<double> derivativeN(const QVector<double>& data, int order);

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    /** @brief 微分计算完成 @param order 阶数 @param count 点数 */
    void derivativeComputed(int order, int count);

private:
    QVector<double> diffForward(const QVector<double>& data) const;
    QVector<double> diffBackward(const QVector<double>& data) const;
    QVector<double> diffCentral(const QVector<double>& data) const;
    QVector<double> diffSavitzkyGolay(const QVector<double>& data) const;

    DiffMethod m_method;        ///< 差分方法
    double m_dx;                ///< 采样间隔
    double m_derivSum;          ///< 导数累加器

    Stats m_stats;
};

#endif // NUMERICALDERIVATIVE_H
