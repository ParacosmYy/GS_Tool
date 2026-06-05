/**
 * @file AkimaInterpolation.h
 * @brief Akima 分段三次插值算法
 * @author EmbedDebug Team
 * @date 2026-06-05
 *
 * 实现 Akima 局部插值方法，生成平滑的分段三次曲线。
 * 相比三次样条，Akima 插值不会出现过冲现象，
 * 特别适合含噪声或突变的工程数据。
 */

#ifndef AKIMAINTERPOLATION_H
#define AKIMAINTERPOLATION_H

#include <QElapsedTimer>
#include <QObject>
#include <QVector>
#include <algorithm>
#include <cmath>
#include <numeric>

/**
 * @class AkimaInterpolation
 * @brief Akima 分段三次插值引擎
 *
 * 给定一组 (x, y) 控制点，对任意查询点 xQuery 返回插值 y 值。
 * 斜率采用 Akima 局部加权公式计算，避免全局矩阵求解。
 */
class AkimaInterpolation : public QObject
{
    Q_OBJECT

public:
    /** @brief 运行时统计信息 */
    struct Stats {
        quint64 totalInterpolations = 0;  ///< 累计插值调用次数
        double avgProcessingTimeMs = 0.0; ///< 平均单次处理耗时(ms)
    };

    /** @brief 构造 Akima 插值器 @param parent 父对象 */
    explicit AkimaInterpolation(QObject *parent = nullptr);

    /**
     * @brief 执行 Akima 插值
     * @param x 控制点横坐标(严格递增,至少5个点)
     * @param y 控制点纵坐标
     * @param xQuery 待查询横坐标数组
     * @return 与 xQuery 对应的插值结果数组
     *
     * 若输入不满足要求则返回空 QVector 并发出警告。
     */
    QVector<double> interpolate(const QVector<double> &x,
                                const QVector<double> &y,
                                const QVector<double> &xQuery);

    /** @brief 获取运行时统计 */
    Stats stats() const { return m_stats; }

    /** @brief 重置所有统计计数器 */
    void resetStatistics();

signals:
    /** @brief 插值完成信号 @param pointCount 查询点数量 */
    void interpolationCompleted(int pointCount);

private:
    /**
     * @brief 计算 Akima 斜率
     * @param x 横坐标
     * @param y 纵坐标
     * @return 每个控制点的 Akima 斜率
     */
    static QVector<double> computeSlopes(const QVector<double> &x,
                                         const QVector<double> &y);

    /**
     * @brief 在单段内用 Hermite 三次公式求值
     * @param t 归一化参数 [0,1]
     * @param y0 左端点值
     * @param y1 右端点值
     * @param s0 左端点斜率
     * @param s1 右端点斜率
     * @param h 段宽
     * @return 插值结果
     */
    static double hermite(double t, double y0, double y1,
                          double s0, double s1, double h);

    Stats m_stats;         ///< 统计数据
    QElapsedTimer m_timer; ///< 计时器
    double m_timeAccum = 0.0; ///< 累计耗时(ms)
};

#endif // AKIMAINTERPOLATION_H
