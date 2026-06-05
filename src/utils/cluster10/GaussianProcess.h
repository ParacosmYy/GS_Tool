/**
 * @file GaussianProcess.h
 * @brief 高斯过程回归 — RBF 核函数实现
 *
 * 功能: 实现高斯过程回归 (Gaussian Process Regression)，
 *       使用径向基函数 (RBF) 核，支持噪声观测、超参数优化、
 *       不确定性估计。适用于传感器数据插值和异常检测。
 *
 * 协作: TrendPredictor(趋势预测) / AnomalyDetector(异常检测)
 */

#pragma once

#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief 高斯过程回归器
 *
 * 给定训练数据 {(x_i, y_i)}，高斯过程提供在任意新点 x* 处
 * 的预测均值和方差 (不确定性)。RBF 核控制平滑程度。
 * 时间复杂度 O(n^3) 用于训练 (Cholesky 分解)，O(n^2) 用于预测。
 */
class GaussianProcess : public QObject {
    Q_OBJECT

public:
    /** @brief 预测结果 */
    struct Prediction {
        double mean = 0.0;         ///< 预测均值
        double variance = 0.0;     ///< 预测方差 (不确定性)
        double stdDev = 0.0;       ///< 标准差
    };

    /** @brief 超参数 */
    struct HyperParams {
        double lengthScale = 1.0;  ///< RBF 长度尺度 (越大越平滑)
        double signalVariance = 1.0; ///< 信号方差
        double noiseVariance = 0.01; ///< 观测噪声方差
    };

    /** @brief 统计信息 */
    struct Stats {
        int totalFits = 0;                 ///< 累计拟合次数
        int totalPredictions = 0;          ///< 累计预测次数
        int totalTrainingPoints = 0;       ///< 累计训练数据点数
        double avgProcessingTimeMs = 0.0;  ///< 平均处理时间(ms)
    };

    explicit GaussianProcess(QObject* parent = nullptr);

    /**
     * @brief 设置超参数
     * @param params RBF 核超参数
     */
    void setHyperParams(const HyperParams& params);

    /**
     * @brief 获取当前超参数
     * @return 超参数
     */
    HyperParams hyperParams() const;

    /**
     * @brief 拟合训练数据
     * @param xTrain 训练输入 (自变量)
     * @param yTrain 训练输出 (因变量)
     * @return true 拟合成功
     */
    bool fit(const QVector<double>& xTrain, const QVector<double>& yTrain);

    /**
     * @brief 在新点处预测
     * @param xNew 新输入点
     * @return 预测均值和方差
     */
    Prediction predict(double xNew) const;

    /**
     * @brief 批量预测多个点
     * @param xNew 新输入点列表
     * @return 预测结果列表
     */
    QVector<Prediction> predictBatch(const QVector<double>& xNew) const;

    /**
     * @brief 通过边际似然梯度优化超参数
     * @param learningRate 学习率
     * @param iterations 迭代次数
     */
    void optimizeHyperParams(double learningRate = 0.01, int iterations = 50);

    /** @brief 获取统计信息 */
    const Stats& stats() const { return m_stats; }

    /** @brief 重置统计信息 */
    void resetStatistics();

signals:
    /** @brief 拟合完成 @param nSamples 训练数据量 */
    void fitCompleted(int nSamples);

    /** @brief 超参数优化完成 @param lengthScale 长度尺度 @param variance 信号方差 */
    void hyperParamsOptimized(double lengthScale, double variance);

private:
    double rbfKernel(double x1, double x2) const;
    double rbfKernelDx(double x1, double x2) const;
    bool cholesky(QVector<QVector<double>>& A, int n);
    void solveTriangular(const QVector<QVector<double>>& L,
                         QVector<double>& b, int n);

    HyperParams m_params;                   ///< 超参数
    QVector<double> m_xTrain;              ///< 训练输入
    QVector<double> m_yTrain;              ///< 训练输出
    QVector<double> m_alpha;               ///< K^{-1} y (Cholesky 结果)
    QVector<QVector<double>> m_L;          ///< 核矩阵的 Cholesky 分解

    Stats m_stats;                          ///< 统计信息
    double m_timeSum = 0.0;                 ///< 处理时间累加器
};
