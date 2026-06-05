/**
 * @file SymmetricEigen2.h
 * @brief 对称矩阵特征值分解 — 三对角QL算法
 *
 * 功能: 使用Householder约化+QL迭代算法计算实对称矩阵的全部
 *       特征值和特征向量, 数值稳定且高效。
 *
 * 协作: DataCorrelator(相关矩阵) / SpectrumAnalyzer(频谱分析)
 */

#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 对称矩阵特征值分解器
 *
 * 算法流程:
 * 1. Householder变换将对称矩阵约化为三对角矩阵
 * 2. 隐式位移QL迭代求解三对角矩阵特征值
 * 3. 累积变换得到特征向量
 *
 * 复杂度: O(n^3), 其中n为矩阵维度。
 */
class SymmetricEigen2 : public QObject
{
    Q_OBJECT

public:
    /** @brief 分解结果 */
    struct EigenResult {
        QVector<double> eigenvalues;            ///< 特征值(升序排列)
        QVector<QVector<double>> eigenvectors;  ///< 特征向量(列向量)
        int iterations = 0;                     ///< QL迭代次数
        bool converged = false;                 ///< 是否收敛
    };

    /** @brief 统计信息 */
    struct Stats {
        int totalDecompositions = 0;         ///< 累计分解次数
        int totalIterations = 0;             ///< 累计迭代次数
        int totalConverged = 0;              ///< 收敛次数
        int totalDiverged = 0;               ///< 未收敛次数
        int maxMatrixSize = 0;               ///< 最大处理矩阵维度
        double avgProcessingTimeMs = 0.0;    ///< 平均处理耗时(ms)
    };

    explicit SymmetricEigen2(QObject* parent = nullptr);

    /**
     * @brief 对称矩阵特征值分解
     * @param matrix 对称矩阵(行优先存储, n*n)
     * @param n 矩阵维度
     * @param maxIterations 最大迭代次数
     * @return 特征值和特征向量
     */
    EigenResult decompose(const QVector<double>& matrix, int n,
                          int maxIterations = 100);

    /**
     * @brief 三对角矩阵特征值分解(QL算法)
     * @param diagonal 对角线元素
     * @param subdiagonal 次对角线元素
     * @param maxIterations 最大迭代次数
     * @return 特征值和特征向量
     */
    EigenResult decomposeTridiagonal(const QVector<double>& diagonal,
                                     const QVector<double>& subdiagonal,
                                     int maxIterations = 100);

    /**
     * @brief 仅计算特征值(不计算特征向量)
     * @param matrix 对称矩阵
     * @param n 矩阵维度
     * @return 特征值(升序)
     */
    QVector<double> eigenvaluesOnly(const QVector<double>& matrix, int n);

    /**
     * @brief Householder约化为三对角矩阵
     * @param matrix 对称矩阵
     * @param n 维度
     * @param diagonal 输出对角线
     * @param subdiagonal 输出次对角线
     * @param transform 输出变换矩阵(可选,传nullptr则不计算)
     */
    void householderTridiagonal(const QVector<double>& matrix, int n,
                                QVector<double>& diagonal,
                                QVector<double>& subdiagonal,
                                QVector<double>* transform = nullptr);

    /**
     * @brief 验证对称性
     * @param matrix 矩阵
     * @param n 维度
     * @param tolerance 容差
     * @return 是否对称
     */
    bool isSymmetric(const QVector<double>& matrix, int n,
                     double tolerance = 1e-10) const;

    /**
     * @brief 矩阵乘法(C = A * B)
     * @param a 矩阵A(n*n)
     * @param b 矩阵B(n*n)
     * @param n 维度
     * @return 矩阵C
     */
    static QVector<double> matMul(const QVector<double>& a,
                                  const QVector<double>& b, int n);

    Stats stats() const;
    void resetStatistics();

signals:
    /** @brief 分解完成信号 @param n 矩阵维度 @param converged 是否收敛 */
    void decomposed(int n, bool converged);

private:
    /**
     * @brief QL迭代核心
     * @param d 对角线(会被修改)
     * @param e 次对角线(会被修改)
     * @param z 变换累积矩阵
     * @param n 维度
     * @param maxIter 最大迭代次数
     * @return 迭代次数, -1表示未收敛
     */
    int qlIterate(QVector<double>& d, QVector<double>& e,
                  QVector<double>& z, int n, int maxIter);

    Stats m_stats;
    double m_timeSum = 0.0;
};
