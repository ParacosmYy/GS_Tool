/**
 * @file LatticeReducer.h
 * @brief LLL格基规约算法 — 格向量最优化
 *
 * 实现Lenstra-Lenstra-Lovasz(LLL)格基规约算法:
 *   - 将格基向量规约为更短、更正交的基
 *   - 支持整数格和实数格
 *   - 提供Gram-Schmidt正交化过程
 *   - 应用于密码分析、整数规划、CVP/SVP近似
 *
 * 协作: SymmetricEigen(矩阵分析) / DataNormalizer(归一化)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QString>
#include <QPair>

/**
 * @class LatticeReducer
 * @brief LLL格基规约引擎
 *
 * 输入一组基向量构成的格，输出LLL规约后的基。
 * 规约后的基向量更短且更接近正交。
 */
class LatticeReducer : public QObject
{
    Q_OBJECT

public:
    /** @brief 规约结果 */
    struct ReductionResult {
        QVector<QVector<double>> reducedBasis;  ///< 规约后的基向量
        QVector<double> basisNorms;              ///< 各基向量范数
        int swapCount = 0;                       ///< 交换次数
        int reductionSteps = 0;                  ///< 规约步数
        double originalDeterminant = 0.0;        ///< 原始格行列式
        double reducedDeterminant = 0.0;         ///< 规约后行列式
        bool success = false;                    ///< 是否成功
    };

    /** @brief 统计信息 */
    struct Stats {
        quint64 totalReductions = 0;        ///< 累计规约次数
        quint64 totalSwaps = 0;             ///< 累计交换次数
        quint64 totalVectorOps = 0;         ///< 累计向量运算次数
        double  avgProcessingTimeMs = 0.0;  ///< 平均规约耗时(ms)
    };

    /** @brief 构造函数 @param parent 父对象 */
    explicit LatticeReducer(QObject* parent = nullptr);

    /**
     * @brief 设置LLL参数delta
     * @param delta 松弛参数(0.25~1.0, 越大越严格, 默认0.75)
     */
    void setDelta(double delta);

    /**
     * @brief 设置收敛精度
     * @param epsilon 数值精度(默认1e-10)
     */
    void setEpsilon(double epsilon);

    /**
     * @brief 设置最大规约步数
     * @param maxSteps 最大步数(0=不限)
     */
    void setMaxSteps(int maxSteps);

    /**
     * @brief 执行LLL规约
     * @param basis 输入基向量(每行一个向量)
     * @return 规约结果
     */
    ReductionResult reduce(const QVector<QVector<double>>& basis);

    /**
     * @brief Gram-Schmidt正交化
     * @param basis 输入基
     * @return (正交化基, 投影系数μ)
     */
    QPair<QVector<QVector<double>>, QVector<QVector<double>>>
    gramSchmidt(const QVector<QVector<double>>& basis) const;

    /**
     * @brief 计算格的行列式(近似)
     * @param basis 基向量
     * @return 行列式绝对值
     */
    double determinant(const QVector<QVector<double>>& basis) const;

    /**
     * @brief 计算向量范数
     * @param v 向量
     * @return 欧几里得范数
     */
    static double vectorNorm(const QVector<double>& v);

    /**
     * @brief 计算两个向量的内积
     * @param a 向量a
     * @param b 向量b
     * @return 内积
     */
    static double dotProduct(const QVector<double>& a,
                             const QVector<double>& b);

    /**
     * @brief 计算最短向量近似解(SVP)
     * @param basis 格基
     * @return 最短向量的范数
     */
    double shortestVectorApprox(const QVector<QVector<double>>& basis);

    /** @brief 获取统计信息 */
    Stats stats() const;

    /** @brief 重置统计 */
    void resetStatistics();

signals:
    /** @brief 规约完成 @param swaps 交换次数 @param steps 步数 */
    void reductionCompleted(int swaps, int steps);

    /** @brief 基向量交换 @param i 交换位置 */
    void basisSwap(int i);

private:
    /** @brief 向量减法 */
    static QVector<double> vecSub(const QVector<double>& a,
                                  const QVector<double>& b);

    /** @brief 向量数乘 */
    static QVector<double> vecScale(const QVector<double>& v, double s);

    /** @brief 取整 */
    static double nearestInt(double x);

    /** @brief 验证基的有效性 */
    bool validateBasis(const QVector<QVector<double>>& basis) const;

    double m_delta = 0.75;       ///< LLL参数delta
    double m_epsilon = 1e-10;    ///< 数值精度
    int m_maxSteps = 0;          ///< 最大步数(0=不限)

    mutable Stats m_stats;         ///< 操作统计
    mutable double m_timeSum = 0.0;///< 累计耗时
};
