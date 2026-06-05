/**
 * @file RichardsonExtrapolation.h
 * @brief Richardson外推引擎 — 数值精度提升方法
 *
 * 功能: 通过对不同步长的近似值进行外推来消除低阶误差项，
 *       显著提高数值微分/积分的精度。支持通用多项式外推
 *       和基于差分的精度改进。
 *
 * 协作: NumericalDifferentiator(微分精度提升) / GaussLegendre(积分精度)
 */
#ifndef RICHARDSONEXTRAPOLATION_H
#define RICHARDSONEXTRAPOLATION_H

#include <QObject>
#include <QVector>
#include <functional>

/**
 * @brief Richardson外推引擎
 */
class RichardsonExtrapolation : public QObject {
    Q_OBJECT

public:
    /** @brief 统计信息 */
    struct Stats {
        quint64 totalExtrapolations = 0;    ///< 累计外推次数
        double  avgProcessingTimeMs = 0.0;  ///< 平均处理耗时(ms)
    };

    explicit RichardsonExtrapolation(QObject* parent = nullptr);

    /**
     * @brief 对多个近似值进行Richardson外推
     * @param approximations 不同步长下的近似值(从粗到细)
     * @param steps 对应的步长(从大到小)
     * @return 外推后的高精度结果
     *
     * 使用Neville表格算法进行通用阶数的外推，
     * 至少需要2组(近似值,步长)才能进行一级外推。
     */
    double extrapolate(const QVector<double>& approximations,
                       const QVector<double>& steps);

    /**
     * @brief 对函数f在x处的值进行步长细化和外推
     * @param f 目标函数
     * @param x 求值点
     * @param h 初始步长
     * @param order 误差阶数(默认2表示O(h²)误差)
     * @return 外推后的高精度值
     *
     * 使用4个递减步长(h, h/2, h/4, h/8)进行3级外推。
     */
    double refine(std::function<double(double)> f,
                  double x, double h, int order = 2);

    /** @brief 获取统计信息 */
    const Stats& stats() const { return m_stats; }

    /** @brief 重置统计信息 */
    void resetStatistics();

signals:
    /** @brief 外推完成信号 @param result 外推结果 */
    void extrapolationCompleted(double result);

private:
    Stats m_stats;              ///< 统计信息
    double m_timeSumMs;         ///< 累计耗时(ms)
};

#endif // RICHARDSONEXTRAPOLATION_H
