/**
 * @file PhaseUnwrapper.h
 * @brief 相位解包器 — 1D/2D相位跳变连续化
 *
 * 功能: 对卷绕相位 (wrapped phase) 执行解包操作，消除
 *       2pi 周期跳变，恢复真实的连续相位分布。支持一维
 *       信号和二维矩阵的相位解包，可配置跳变容差。
 *
 * 协作: FFT3(频谱相位提取) / HilbertTransformer(解析信号相位)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 相位解包器 — 消除相位卷绕跳变
 */
class PhaseUnwrapper : public QObject {
    Q_OBJECT

public:
    /** @brief 统计信息 */
    struct Stats {
        quint64 totalUnwrapped = 0;       ///< 累计解包次数
        double  avgProcessingTimeMs = 0.0; ///< 平均处理时间(ms)
    };

    /**
     * @brief 构造函数
     * @param tolerance 相位跳变容差(默认0.5，单位为2pi倍数)
     * @param parent 父对象
     */
    explicit PhaseUnwrapper(double tolerance = 0.5,
                              QObject* parent = nullptr);

    /**
     * @brief 一维相位解包
     * @param phase 卷绕相位序列(弧度)
     * @return 解包后的连续相位
     *
     * 逐点检测相邻相位差，当差值超过 tolerance*2*pi 时
     * 补偿整数个 2pi 使输出连续。
     */
    QVector<double> unwrap1D(const QVector<double>& phase);

    /**
     * @brief 二维相位解包
     * @param phase 卷绕相位矩阵(行优先)
     * @return 解包后的连续相位矩阵
     *
     * 采用质量引导策略: 先沿第一行和第一列解包构建骨架，
     * 再按行列优先顺序传播补偿。
     */
    QVector<QVector<double>> unwrap2D(
        const QVector<QVector<double>>& phase);

    /**
     * @brief 设置跳变容差
     * @param tol 容差值(0.0~1.0)
     */
    void setTolerance(double tol);

    /** @brief 获取统计 */
    const Stats& stats() const { return m_stats; }

    /** @brief 重置统计 */
    void resetStatistics();

    /** @brief 获取当前容差 */
    double tolerance() const { return m_tolerance; }

signals:
    /** @brief 解包完成 @param elements 处理元素数量 */
    void unwrapCompleted(int elements);

private:
    /**
     * @brief 计算相位差的卷绕校正量
     * @param diff 原始相位差
     * @return 校正后的相位差(在 [-pi, pi] 内)
     */
    double wrapDiff(double diff) const;

    double m_tolerance;  ///< 跳变容差
    double m_timeSum;    ///< 处理时间累加器
    Stats  m_stats;      ///< 统计信息
};
