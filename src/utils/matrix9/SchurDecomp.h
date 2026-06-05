/**
 * @file SchurDecomp.h
 * @brief Schur分解 — QR迭代实现实Schur分解
 *
 * 功能: 实现实Schur分解，将方阵分解为 Q * T * Q^T，
 *       其中Q为正交矩阵，T为拟上三角矩阵(实Schur形式)。
 *       对角块为1x1(实特征值)或2x2(复共轭特征值对)。
 *       适用于特征值计算、矩阵函数计算和稳定性分析。
 *
 * 协作: EigenDecomp(特征值分解) / MatrixSolve(线性求解)
 */
#pragma once

#include <QObject>
#include <QVector>

#include <complex>
#include <vector>

/**
 * @brief Schur分解 — QR迭代算法
 */
class SchurDecomp : public QObject {
    Q_OBJECT

public:
    /** @brief 分解结果 */
    struct Result {
        std::vector<std::vector<double>> Q;     ///< 正交矩阵Q
        std::vector<std::vector<double>> T;     ///< 拟上三角矩阵T
        std::vector<std::complex<double>> eigenvalues; ///< 特征值
        int iterations = 0;                      ///< 迭代次数
        bool converged = false;                  ///< 是否收敛
    };

    /** @brief 统计信息 */
    struct Stats {
        int totalDecompositions = 0;  ///< 累计分解次数
        int totalIterations = 0;      ///< 累计迭代次数
        int totalConverged = 0;       ///< 累计收敛次数
        double avgProcessingTimeMs = 0.0; ///< 平均处理时间(ms)
    };

    /**
     * @brief 构造函数
     * @param maxIter 最大迭代次数
     * @param tolerance 收敛容差
     * @param parent 父对象
     */
    explicit SchurDecomp(int maxIter = 300, double tolerance = 1e-12,
                         QObject* parent = nullptr);

    /**
     * @brief 执行Schur分解
     * @param matrix 输入方阵
     * @return 分解结果
     */
    Result decompose(const std::vector<std::vector<double>>& matrix);

    /**
     * @brief 仅计算特征值(不保存变换矩阵)
     * @param matrix 输入方阵
     * @return 特征值列表
     */
    std::vector<std::complex<double>> eigenvaluesOnly(
        const std::vector<std::vector<double>>& matrix);

    /**
     * @brief 从Schur形式计算矩阵指数
     * @param T Schur矩阵T
     * @param Q 正交矩阵Q
     * @return exp(A) = Q * exp(T) * Q^T
     */
    std::vector<std::vector<double>> matrixExponential(
        const std::vector<std::vector<double>>& T,
        const std::vector<std::vector<double>>& Q) const;

    /** @brief 获取统计信息 */
    const Stats& stats() const { return m_stats; }

    /** @brief 重置统计信息 */
    void resetStatistics();

private:
    /**
     * @brief Householder向量化
     * @param x 输入向量
     * @param v 输出Householder向量
     * @return beta系数
     */
    double householder(const std::vector<double>& x, std::vector<double>& v) const;

    /**
     * @brief Hessenberg化简(预处理步骤)
     * @param A 输入矩阵(被修改)
     * @param Q 累积变换矩阵
     */
    void hessenbergReduce(std::vector<std::vector<double>>& A,
                          std::vector<std::vector<double>>& Q) const;

    /**
     * @brief 单步QR迭代(带Wilkinson位移)
     * @param H Hessenberg矩阵
     * @param Q 累积变换矩阵
     * @param lo 当前处理子矩阵下界
     * @param hi 当前处理子矩阵上界
     */
    void qrStep(std::vector<std::vector<double>>& H,
                std::vector<std::vector<double>>& Q,
                int lo, int hi);

    /**
     * @brief 从拟上三角矩阵提取特征值
     * @param T Schur矩阵
     * @return 特征值列表
     */
    std::vector<std::complex<double>> extractEigenvalues(
        const std::vector<std::vector<double>>& T) const;

    /**
     * @brief 计算2x2块的特征值
     * @param a11 元素
     * @param a12 元素
     * @param a21 元素
     * @param a22 元素
     * @return 一对特征值
     */
    std::pair<std::complex<double>, std::complex<double>> eigenvalues2x2(
        double a11, double a12, double a21, double a22) const;

    int m_maxIter;                  ///< 最大迭代次数
    double m_tolerance;             ///< 收敛容差

    Stats m_stats;                  ///< 统计信息
    double m_timeSum = 0.0;         ///< 累计耗时
};
