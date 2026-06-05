/**
 * @file LuDecomposition.h
 * @brief LU分解 — 部分主元选取/求解/行列式/逆矩阵
 *
 * 功能: 对方阵执行LU分解(PA=LU)，支持线性方程组求解、
 *       行列式计算和矩阵求逆，使用部分主元选取保证数值稳定性。
 *
 * 协作: GaussianMixture2(矩阵运算) / SVD(奇异值分解)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief LU分解器(部分主元选取)
 */
class LuDecomposition : public QObject {
    Q_OBJECT

public:
    /** @brief 统计信息 */
    struct Stats {
        quint64 totalDecompositions = 0;    ///< 累计分解次数
        double  avgProcessingTimeMs = 0.0;  ///< 平均处理耗时(ms)
    };

    /** @brief 构造函数 @param parent 父对象 */
    explicit LuDecomposition(QObject* parent = nullptr);

    /**
     * @brief LU分解(PA = LU)
     * @param A 方阵输入
     * @return 是否分解成功(奇异矩阵返回false)
     */
    bool decompose(const QVector<QVector<double>>& A);

    /**
     * @brief 求解线性方程组 Ax = b
     * @param b 右端向量
     * @return 解向量(未分解或失败返回空)
     */
    QVector<double> solve(const QVector<double>& b);

    /**
     * @brief 计算行列式
     * @return 行列式值(未分解返回0)
     */
    double determinant();

    /**
     * @brief 计算逆矩阵
     * @return 逆矩阵(未分解或奇异返回空)
     */
    QVector<QVector<double>> inverse();

    /** @brief 是否已分解 @return 分解状态 */
    bool isDecomposed() const { return m_decomposed; }

    /** @brief 获取统计 @return 统计信息常引用 */
    const Stats& stats() const { return m_stats; }

    /** @brief 重置统计 */
    void resetStatistics();

signals:
    /** @brief 分解完成 @param n 矩阵维度 */
    void decompositionCompleted(int n);

private:
    /**
     * @brief 前代求解 Ly = Pb
     * @param b 变换后的右端向量
     * @return y向量
     */
    QVector<double> forwardSub(const QVector<double>& b) const;

    /**
     * @brief 回代求解 Ux = y
     * @param y 中间向量
     * @return x向量
     */
    QVector<double> backSub(const QVector<double>& y) const;

    int m_n;                                    ///< 矩阵维度
    QVector<QVector<double>> m_lu;              ///< LU合成矩阵
    QVector<int> m_perm;                        ///< 置换向量
    int m_parity;                               ///< 置换奇偶性(行列式符号)
    bool m_decomposed;                          ///< 分解标志
    bool m_singular;                            ///< 奇异标志
    Stats m_stats;                              ///< 统计信息
    double m_timeSum;                           ///< 累计耗时
};
