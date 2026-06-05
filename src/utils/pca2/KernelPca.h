/**
 * @file KernelPca.h
 * @brief 核主成分分析 — RBF/多项式/线性核函数
 *
 * 实现核主成分分析(Kernel PCA)，支持三种核函数:
 *   - RBF(径向基函数/高斯核): 适用于非线性流形
 *   - 多项式核: 适用于多项式关系
 *   - 线性核: 退化为标准PCA
 *
 * 协作: SymmetricEigen(特征分解) / DataNormalizer(归一化)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QString>
#include <QPair>

/**
 * @class KernelPca
 * @brief 核主成分分析引擎
 *
 * 通过核技巧将数据映射到高维空间后执行PCA，
 * 可以捕获数据中的非线性结构。
 */
class KernelPca : public QObject
{
    Q_OBJECT

public:
    /** @brief 核函数类型 */
    enum class KernelType {
        RBF,           ///< 径向基函数(高斯核)
        Polynomial,    ///< 多项式核
        Linear         ///< 线性核(标准PCA)
    };
    Q_ENUM(KernelType)

    /** @brief 变换结果 */
    struct TransformResult {
        QVector<QVector<double>> projectedData; ///< 投影后的数据
        QVector<double> eigenvalues;            ///< 核矩阵特征值
        QVector<QVector<double>> eigenvectors;  ///< 特征向量
        double explainedVariance = 0.0;         ///< 解释方差比
        int componentsUsed = 0;                 ///< 使用的主成分数
    };

    /** @brief 统计信息 */
    struct Stats {
        quint64 totalFits = 0;                ///< 累计拟合次数
        quint64 totalTransforms = 0;          ///< 累计变换次数
        quint64 totalSamplesProcessed = 0;    ///< 累计处理样本数
        double  avgProcessingTimeMs = 0.0;    ///< 平均处理耗时(ms)
    };

    /** @brief 构造函数 @param parent 父对象 */
    explicit KernelPca(QObject* parent = nullptr);

    /**
     * @brief 设置核函数类型
     * @param type 核类型
     */
    void setKernelType(KernelType type);

    /**
     * @brief 设置核函数参数
     * @param gamma RBF的gamma参数(默认1.0)
     * @param degree 多项式的阶数(默认3)
     * @param coef0 多项式的常数项(默认1.0)
     */
    void setKernelParameters(double gamma, int degree = 3, double coef0 = 1.0);

    /**
     * @brief 设置目标主成分数
     * @param components 主成分数(0=自动选择)
     */
    void setComponentCount(int components);

    /**
     * @brief 设置方差保留比例(自动选择主成分数时使用)
     * @param ratio 保留比例(0.0~1.0, 默认0.95)
     */
    void setVarianceRatio(double ratio);

    /**
     * @brief 拟合模型
     * @param data 输入数据(每行一个样本)
     */
    void fit(const QVector<QVector<double>>& data);

    /**
     * @brief 变换数据(投影到核主成分空间)
     * @param data 输入数据
     * @return 变换结果
     */
    TransformResult transform(const QVector<QVector<double>>& data);

    /**
     * @brief 拟合并变换(便捷方法)
     * @param data 输入数据
     * @return 变换结果
     */
    TransformResult fitTransform(const QVector<QVector<double>>& data);

    /**
     * @brief 变换单个新样本
     * @param sample 样本向量
     * @return 投影坐标
     */
    QVector<double> transformSample(const QVector<double>& sample);

    /**
     * @brief 计算核矩阵
     * @param data 输入数据
     * @return 核矩阵(n x n)
     */
    QVector<QVector<double>> computeKernelMatrix(
        const QVector<QVector<double>>& data) const;

    /**
     * @brief 获取解释方差比
     * @return 各主成分的方差比
     */
    QVector<double> explainedVarianceRatio() const;

    /** @brief 模型是否已拟合 */
    bool isFitted() const;

    /** @brief 获取统计信息 */
    Stats stats() const;

    /** @brief 重置统计 */
    void resetStatistics();

signals:
    /** @brief 拟合完成 @param components 主成分数 @param variance 解释方差 */
    void fitCompleted(int components, double variance);

    /** @brief 变换完成 @param samples 样本数 @param dimensions 降维后维度 */
    void transformCompleted(int samples, int dimensions);

private:
    /** @brief 计算两个向量的核函数值 */
    double kernelValue(const QVector<double>& a,
                       const QVector<double>& b) const;

    /** @brief 特征值分解(幂迭代+Deflation) */
    void eigenDecompose(QVector<QVector<double>>& matrix,
                        QVector<double>& eigenvalues,
                        QVector<QVector<double>>& eigenvectors,
                        int numComponents) const;

    /** @brief 中心化核矩阵 */
    QVector<QVector<double>> centerKernelMatrix(
        const QVector<QVector<double>>& K) const;

    KernelType m_kernelType = KernelType::RBF; ///< 核类型
    double m_gamma = 1.0;      ///< RBF gamma参数
    int m_degree = 3;          ///< 多项式阶数
    double m_coef0 = 1.0;     ///< 多项式常数项
    int m_components = 0;      ///< 目标主成分数
    double m_varianceRatio = 0.95; ///< 方差保留比例

    QVector<QVector<double>> m_trainingData; ///< 训练数据
    QVector<double> m_eigenvalues;           ///< 特征值
    QVector<QVector<double>> m_eigenvectors; ///< 特征向量
    QVector<double> m_kColMean;              ///< 核矩阵列均值
    double m_kMean = 0.0;                    ///< 核矩阵总均值
    bool m_fitted = false;                   ///< 是否已拟合

    mutable Stats m_stats;         ///< 操作统计
    mutable double m_timeSum = 0.0;///< 累计耗时
};
