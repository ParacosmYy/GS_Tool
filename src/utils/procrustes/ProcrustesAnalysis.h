/**
 * @file ProcrustesAnalysis.h
 * @brief Procrustes 形状对齐分析
 * @author EmbedDebug Team
 * @date 2026-06-05
 *
 * 实现 Procrustes 分析，通过平移、缩放和旋转
 * 将源点集对齐到目标点集，使平方距离最小。
 * 广泛用于形状分析、点云配准和模式识别。
 */

#ifndef PROCRUSTESANALYSIS_H
#define PROCRUSTESANALYSIS_H

#include <QElapsedTimer>
#include <QObject>
#include <QVector>
#include <cmath>

/**
 * @class ProcrustesAnalysis
 * @brief Procrustes 形状对齐引擎
 *
 * 支持任意维度 d 的点集对齐(2D/3D/...)。
 * 使用 SVD 分解求最优旋转矩阵。
 */
class ProcrustesAnalysis : public QObject
{
    Q_OBJECT

public:
    /** @brief 运行时统计信息 */
    struct Stats {
        quint64 totalAlignments = 0;    ///< 累计对齐调用次数
        double avgProcessingTimeMs = 0.0; ///< 平均单次处理耗时(ms)
    };

    /** @brief 构造 Procrustes 分析器 @param parent 父对象 */
    explicit ProcrustesAnalysis(QObject *parent = nullptr);

    /**
     * @brief 将源点集 Procrustes 对齐到目标点集
     * @param source n x d 源点集(n 个 d 维点)
     * @param target n x d 目标点集(与 source 同维度)
     * @return n x d 对齐后的点集
     *
     * 流程: 中心化 → 归一化尺度 → SVD 求旋转 → 变换。
     * 若输入不合法返回空数组。
     */
    QVector<QVector<double>> align(const QVector<QVector<double>> &source,
                                   const QVector<QVector<double>> &target);

    /** @brief 获取运行时统计 */
    Stats stats() const { return m_stats; }

    /** @brief 重置所有统计计数器 */
    void resetStatistics();

signals:
    /** @brief 对齐完成信号 @param pointCount 点数量 */
    void alignmentCompleted(int pointCount);

private:
    /**
     * @brief 计算点集质心(各维度均值)
     * @param points n x d 点集
     * @return d 维质心向量
     */
    static QVector<double> centroid(const QVector<QVector<double>> &points);

    /**
     * @brief 中心化点集(减去质心)
     * @param points 点集
     * @param c 质心
     * @return 中心化后的点集
     */
    static QVector<QVector<double>> centerPoints(
        const QVector<QVector<double>> &points, const QVector<double> &c);

    /**
     * @brief 计算点集的 Frobenius 范数(尺度)
     * @param points n x d 点集
     * @return Frobenius 范数
     */
    static double frobeniusNorm(const QVector<QVector<double>> &points);

    /**
     * @brief 2x2 矩阵 SVD 分解(解析公式)
     * @param a 矩阵元素 [a00, a01, a10, a11]
     * @param U 输出左奇异向量(4个元素)
     * @param S 输出奇异值(2个元素)
     * @param Vt 输出右奇异向量(4个元素)
     */
    static void svd2x2(double a00, double a01, double a10, double a11,
                       double U[4], double S[2], double Vt[4]);

    Stats m_stats;          ///< 统计数据
    QElapsedTimer m_timer;  ///< 计时器
    double m_timeAccum = 0.0; ///< 累计耗时(ms)
};

#endif // PROCRUSTESANALYSIS_H
