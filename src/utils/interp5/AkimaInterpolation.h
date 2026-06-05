/**
 * @file AkimaInterpolation.h
 * @brief Akima插值 — 平滑非振荡的样条插值算法
 *
 * 功能: 实现Akima分段三次插值，避免Runge现象，
 *       产生视觉上平滑且无虚假振荡的插值曲线。
 *       适用于数据可视化和信号重建。
 *
 * 协作: DataInterpolator(插值) / WaveformGenerator(波形生成)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QPair>

#include <vector>

/**
 * @brief Akima插值 — 平滑非振荡分段三次插值
 */
class AkimaInterpolation : public QObject {
    Q_OBJECT

public:
    /** @brief 统计信息 */
    struct Stats {
        quint64 totalInterpolations   = 0;   ///< 累计插值次数
        quint64 totalPointsComputed   = 0;   ///< 累计计算点数
        quint64 totalKnots            = 0;    ///< 累计节点数
        double  avgProcessingTimeMs   = 0.0; ///< 平均处理时间(ms)
    };

    explicit AkimaInterpolation(QObject* parent = nullptr);

    /**
     * @brief 设置数据点(控制点)
     * @param x X坐标(必须严格递增)
     * @param y Y坐标
     * @return true=数据有效
     */
    bool setPoints(const QVector<double>& x, const QVector<double>& y);

    /**
     * @brief 在指定x处计算插值
     * @param x 目标X坐标
     * @return 插值Y值
     */
    double interpolate(double x) const;

    /**
     * @brief 批量插值
     * @param xPoints 目标X坐标数组
     * @return 插值Y值数组
     */
    QVector<double> interpolateBatch(const QVector<double>& xPoints) const;

    /**
     * @brief 在区间内均匀采样
     * @param numSamples 采样点数
     * @return (x数组, y数组)
     */
    QPair<QVector<double>, QVector<double>> sample(int numSamples) const;

    /**
     * @brief 计算导数(在指定x处)
     * @param x 目标X坐标
     * @return 插值导数值
     */
    double derivative(double x) const;

    /**
     * @brief 获取数据点数
     * @return 控制点数量
     */
    int pointCount() const { return static_cast<int>(m_x.size()); }

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    /** @brief 批量插值完成 @param count 点数 */
    void interpolationCompleted(int count);

private:
    /**
     * @brief 计算Akima斜率
     * @param x X坐标
     * @param y Y坐标
     * @return 每个节点的斜率
     */
    QVector<double> computeSlopes(const QVector<double>& x,
                                  const QVector<double>& y) const;

    /**
     * @brief 查找x所在的区间索引
     * @param x 目标x
     * @return 区间索引(0-based)
     */
    int findSegment(double x) const;

    std::vector<double> m_x;       ///< 控制点X坐标
    std::vector<double> m_y;       ///< 控制点Y坐标
    std::vector<double> m_slopes;  ///< Akima斜率
    bool m_valid;                   ///< 数据是否有效

    mutable Stats  m_stats;
    mutable double m_timeSumMs = 0.0;
};
