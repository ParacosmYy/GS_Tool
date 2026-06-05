/**
 * @file KernelPca.h
 * @brief 核主成分分析 — 非线性降维
 *
 * 功能: 使用核技巧(RBF/多项式/线性)实现非线性PCA，
 *       将高维数据映射到特征空间后执行主成分分析。
 *
 * 协作: GaussianMixture(分布建模) / ArimaModel(时序分析)
 */
#ifndef KERNELPCA_H
#define KERNELPCA_H

#include <QObject>
#include <QVector>

/**
 * @brief 核主成分分析器
 */
class KernelPca : public QObject {
    Q_OBJECT

public:
    /** @brief 核函数类型 */
    enum class Kernel {
        Rbf,            ///< 径向基核(Gaussian)
        Polynomial,     ///< 多项式核
        Linear          ///< 线性核
    };
    Q_ENUM(Kernel)

    /** @brief 核参数 */
    struct KernelParams {
        double gamma = 1.0;         ///< RBF核参数γ
        double degree = 3.0;        ///< 多项式核度数
        double coef0 = 1.0;         ///< 多项式核偏移
    };

    /** @brief 统计 */
    struct Stats {
        quint64 totalTransforms = 0;        ///< 累计变换次数
        double  avgProcessingTimeMs = 0.0;  ///< 平均处理耗时(ms)
    };

    explicit KernelPca(QObject* parent = nullptr);

    /**
     * @brief 拟合核PCA模型
     * @param data 输入数据(每行一个样本)
     * @param kernel 核函数类型
     * @param params 核参数
     * @param nComponents 保留的主成分数(0=自动)
     * @return 实际保留的主成分数
     */
    int fit(const QVector<QVector<double>>& data,
            Kernel kernel,
            const KernelParams& params,
            int nComponents = 0);

    /** @brief 拟合核PCA(使用默认参数) */
    int fit(const QVector<QVector<double>>& data,
            int nComponents = 0);

    /**
     * @brief 变换单个样本
     * @param sample 输入样本
     * @return 降维后的坐标
     */
    QVector<double> transform(const QVector<double>& sample) const;

    /**
     * @brief 批量变换
     * @param data 输入数据
     * @return 降维后的数据
     */
    QVector<QVector<double>> transformBatch(
        const QVector<QVector<double>>& data) const;

    /** @brief 获取特征值 */
    QVector<double> eigenvalues() const { return m_eigenvalues; }

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    /** @brief 变换完成 @param dimensions 输出维度 */
    void transformCompleted(int dimensions);

private:
    /** @brief 计算核函数值 */
    double kernelValue(const QVector<double>& x,
                       const QVector<double>& y) const;

    /** @brief 幂迭代求最大特征向量 */
    QVector<double> powerIteration(
        const QVector<QVector<double>>& matrix, int maxIter) const;

    QVector<QVector<double>> m_trainData;   ///< 训练数据
    QVector<QVector<double>> m_alphas;      ///< 投影系数
    QVector<double> m_eigenvalues;          ///< 特征值
    Kernel m_kernel = Kernel::Rbf;          ///< 核函数类型
    KernelParams m_params;                   ///< 核参数
    int m_nComponents = 0;                  ///< 主成分数
    Stats m_stats;                           ///< 统计信息
    mutable double m_timeSum = 0.0;          ///< 累计耗时
};

#endif // KERNELPCA_H
