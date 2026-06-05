/**
 * @file BlindSourceSep.h
 * @brief 盲源分离引擎 — FastICA算法(独立成分分析)
 *
 * 功能: 实现FastICA算法进行独立成分分析(ICA)，从混合信号中
 *       分离出独立的源信号。支持deflation和symmetric两种策略、
 *       多种非线性函数、信号白化预处理。
 *       适用于EEG信号处理、音频源分离、通信信号解混。
 *
 * 协作: SpectrumAnalyzer(频域分析) / SignalDecomposer(信号分解)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 盲源分离 — FastICA算法
 *
 * 模型: X = A * S, X为观测混合信号, A为混合矩阵, S为独立源
 * 目标: 求解解混矩阵W使得 Y = W*X 逼近原始源S
 */
class BlindSourceSep : public QObject {
    Q_OBJECT

public:
    /** @brief 非线性函数类型 */
    enum Nonlinearity {
        LogCosh = 0,    ///< G(u) = log(cosh(a*u)), 默认a=1.5
        Gaussian = 1,   ///< G(u) = u*exp(-u^2/2)
        Kurtosis = 2,   ///< G(u) = u^3 (四阶累积量)
        Skew = 3        ///< G(u) = u^2 (偏度)
    };
    Q_ENUM(Nonlinearity)

    /** @brief 分离结果 */
    struct SeparationResult {
        QVector<QVector<double>> sources;   ///< 分离出的源信号
        QVector<QVector<double>> unmixingMatrix; ///< 解混矩阵W
        QVector<QVector<double>> mixingMatrix;   ///< 估计的混合矩阵A
        QVector<double> kurtosis;           ///< 各源信号峰度
        int iterations = 0;                ///< 迭代次数
        bool converged = false;            ///< 是否收敛
        bool success = false;             ///< 是否成功
    };

    /** @brief 操作统计 */
    struct Stats {
        quint64 totalSeparations = 0;      ///< 总分离次数
        quint64 totalSamplesProcessed = 0; ///< 总处理样本数
        quint64 totalIterations = 0;       ///< 总迭代次数
        double  avgProcessingTimeMs = 0.0; ///< 平均处理耗时(ms)
    };

    /**
     * @brief 构造函数
     * @param parent 父对象
     */
    explicit BlindSourceSep(QObject* parent = nullptr);

    /** @brief 析构函数 */
    ~BlindSourceSep() override;

    // ── 核心接口 ──

    /**
     * @brief FastICA分离(deflation方式)
     * @param observations 观测信号矩阵[sensor x samples]
     * @param numSources 目标源数(0=自动=传感器数)
     * @return 分离结果
     */
    SeparationResult separateDeflation(
        const QVector<QVector<double>>& observations,
        int numSources = 0);

    /**
     * @brief FastICA分离(symmetric方式)
     * @param observations 观测信号矩阵[sensor x samples]
     * @param numSources 目标源数
     * @return 分离结果
     */
    SeparationResult separateSymmetric(
        const QVector<QVector<double>>& observations,
        int numSources = 0);

    // ── 预处理 ──

    /**
     * @brief 信号白化(PCA)
     * @param observations 观测信号
     * @return 白化后的信号
     */
    QVector<QVector<double>> whiten(
        const QVector<QVector<double>>& observations) const;

    // ── 辅助 ──

    /**
     * @brief 计算信号峰度
     * @param signal 一维信号
     * @return 峰度值
     */
    static double kurtosis(const QVector<double>& signal);

    /**
     * @brief 设置非线性函数
     * @param nl 非线性函数类型
     */
    void setNonlinearity(Nonlinearity nl);

    /**
     * @brief 设置最大迭代次数
     * @param maxIter 最大迭代次数
     */
    void setMaxIterations(int maxIter);

    /**
     * @brief 设置收敛阈值
     * @param tolerance 收敛阈值
     */
    void setTolerance(double tolerance);

    // ── 统计 ──

    /** @brief 获取统计 */
    Stats stats() const;

    /** @brief 重置统计 */
    void resetStatistics();

signals:
    /** @brief 分离完成 @param numSources 源数 @param iterations 迭代次数 */
    void separationCompleted(int numSources, int iterations);
    /** @brief 迭代进度 @param component 当前成分 @param iteration 当前迭代 */
    void iterationProgress(int component, int iteration);

private:
    /**
     * @brief 非线性函数g(u)及其导数g'(u)
     * @param u 输入
     * @return (g(u), g'(u))
     */
    QPair<double, double> nonlinearity(double u) const;

    /**
     * @brief 中心化(去均值)
     * @param data 输入数据
     * @return 去均值后的数据
     */
    static QVector<QVector<double>> center(
        const QVector<QVector<double>>& data);

    /**
     * @brief Gram-Schmidt正交化
     * @param vectors 向量组
     * @return 正交化后的向量组
     */
    static QVector<QVector<double>> gramSchmidt(
        const QVector<QVector<double>>& vectors);

    /**
     * @brief 矩阵乘法 C = A * B^T
     */
    static QVector<QVector<double>> multiplyABt(
        const QVector<QVector<double>>& a,
        const QVector<QVector<double>>& b);

    Nonlinearity m_nonlinearity;     ///< 非线性函数
    int m_maxIterations;             ///< 最大迭代次数
    double m_tolerance;              ///< 收敛阈值
    double m_alpha;                  ///< LogCosh参数(默认1.5)

    Stats m_stats;                   ///< 操作统计
    double m_timeSum = 0.0;          ///< 累计耗时
};
