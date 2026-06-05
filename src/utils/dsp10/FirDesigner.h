/**
 * @file FirDesigner.h
 * @brief FIR滤波器设计器 — 基于Parks-McClellan Remez交换算法
 *
 * 功能: 利用Remez交换算法设计最优等纹波FIR滤波器，
 *       支持低通/高通/带通/带阻等多种滤波器类型，
 *       通过交错定理求解最优极值频率集合。
 *
 * 协作: AdaptiveFft(频谱分析) / Resampler(多相重采样)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QPair>
#include <QString>
#include <QElapsedTimer>

/**
 * @brief FIR滤波器类型枚举
 */
enum class FirType {
    LowPass,    ///< 低通
    HighPass,   ///< 高通
    BandPass,   ///< 带通
    BandStop    ///< 带阻
};

/**
 * @brief Parks-McClellan FIR滤波器设计器
 */
class FirDesigner : public QObject {
    Q_OBJECT

public:
    /** @brief 运行统计 */
    struct Stats {
        int    totalDesigns = 0;          ///< 累计设计次数
        int    totalIterations = 0;       ///< 累计迭代次数
        int    totalCoefficients = 0;     ///< 累计生成系数总数
        double avgProcessingTimeMs = 0.0; ///< 平均处理耗时(ms)
    };

    explicit FirDesigner(QObject* parent = nullptr);

    /**
     * @brief 设计FIR滤波器
     * @param type 滤波器类型
     * @param filterOrder 滤波器阶数(必须为偶数)
     * @param freqBands 频段边界列表 [0~1归一化频率]
     * @param desiredGains 各频段期望增益
     * @param weights 各频段权重
     * @return 滤波器系数(N+1个)
     */
    QVector<double> design(FirType type,
                           int filterOrder,
                           const QVector<QPair<double, double>>& freqBands,
                           const QVector<double>& desiredGains,
                           const QVector<double>& weights);

    /**
     * @brief 计算频率响应
     * @param coeffs 滤波器系数
     * @param numPoints 频率采样点数
     * @return (频率数组, 幅度响应数组)
     */
    QPair<QVector<double>, QVector<double>>
    frequencyResponse(const QVector<double>& coeffs, int numPoints = 512) const;

    /**
     * @brief 应用滤波器到数据序列
     * @param coeffs 滤波器系数
     * @param input 输入数据
     * @return 滤波后数据(与输入等长，前N个为暂态)
     */
    QVector<double> applyFilter(const QVector<double>& coeffs,
                                const QVector<double>& input) const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    /** @brief 设计完成 @param order 阶数 @param iterations 迭代次数 */
    void designCompleted(int order, int iterations);

private:
    /**
     * @brief Remez交换算法核心迭代
     * @param gridSize 网格点数
     * @param bandEdges 频段边界
     * @param desired 期望响应
     * @param weight 权重
     * @param extrPrefix 极值点数(N+2)
     * @return 最优系数
     */
    QVector<double> remezExchange(int gridSize,
                                  const QVector<double>& bandEdges,
                                  const QVector<double>& desired,
                                  const QVector<double>& weight,
                                  int extrPrefix);

    /** @brief 构建密集频率网格 */
    QVector<double> buildDenseGrid(int gridSize,
                                   const QVector<QPair<double, double>>& freqBands,
                                   const QVector<double>& desiredGains,
                                   const QVector<double>& weights,
                                   QVector<double>& outDesired,
                                   QVector<double>& outWeight) const;

    /** @brief 计算拉格朗日插值 (barycentric form) */
    double lagrangeInterp(const QVector<double>& x,
                          const QVector<double>& y,
                          double xVal) const;

    static constexpr int kMaxRemezIterations = 80; ///< Remez最大迭代次数
    static constexpr int kGridDensity = 16;         ///< 网格密度倍数

    Stats              m_stats;
    double             m_timeSum = 0.0;
    mutable QElapsedTimer m_timer;
};
