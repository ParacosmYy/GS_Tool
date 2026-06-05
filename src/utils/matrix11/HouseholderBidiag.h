/**
 * @file HouseholderBidiag.h
 * @brief Householder双对角化 — SVD预处理变换
 *
 * 功能: 将一般矩阵A通过Householder变换约化为双对角形式 B = U^T * A * V，
 *       作为SVD分解的预处理步骤。支持任意m×n实矩阵。
 *
 * 协作: SpectrumAnalyzer(频谱分析) / DataDecomposer(数据分解)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief Householder双对角化引擎
 */
class HouseholderBidiag : public QObject {
    Q_OBJECT

public:
    /** @brief 矩阵维度 */
    struct Dimensions {
        int rows = 0;        ///< 行数m
        int cols = 0;        ///< 列数n
        int minDim = 0;      ///< min(m,n)
    };

    /** @brief 双对角化结果 */
    struct BidiagResult {
        QVector<QVector<double>> U;         ///< 左正交矩阵U(m×m)
        QVector<QVector<double>> B;         ///< 双对角矩阵B(m×n)
        QVector<QVector<double>> V;         ///< 右正交矩阵V(n×n)
        int householderSteps = 0;           ///< Householder变换步数
    };

    /** @brief 运行统计 */
    struct Stats {
        int totalBidiagonalizations = 0;    ///< 累计双对角化次数
        int totalElementsProcessed = 0;     ///< 累计处理矩阵元素数
        double avgProcessingTimeMs = 0.0;   ///< 平均处理时间(ms)
        double lastOrthogonalityError = 0.0;///< 最近一次正交性误差
    };

    explicit HouseholderBidiag(QObject* parent = nullptr);

    /** @brief 执行双对角化 @param matrix 输入矩阵(m×n) @return 双对角化结果 */
    BidiagResult bidiagonalize(const QVector<QVector<double>>& matrix);

    /** @brief 仅计算双对角矩阵(不累积U和V，更快) @param matrix 输入矩阵 @return 双对角矩阵 */
    QVector<QVector<double>> bidiagonalizeFast(const QVector<QVector<double>>& matrix);

    /** @brief 验证正交性: ||U^T*U - I||_F @param U 矩阵 @return Frobenius范数误差 */
    double verifyOrthogonality(const QVector<QVector<double>>& U) const;

    /** @brief 矩阵乘法 C = A * B @param A 矩阵A @param B 矩阵B @return 乘积 */
    static QVector<QVector<double>> matMul(const QVector<QVector<double>>& A,
                                           const QVector<QVector<double>>& B);

    /** @brief 矩阵转置 @param A 输入矩阵 @return 转置矩阵 */
    static QVector<QVector<double>> transpose(const QVector<QVector<double>>& A);

    /** @brief 获取最近一次的维度信息 @return 维度结构 */
    Dimensions lastDimensions() const { return m_lastDims; }

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    /** @brief 双对角化完成 @param steps 变换步数 @param orthError 正交性误差 */
    void bidiagonalizationComplete(int steps, double orthError);

private:
    /** @brief 生成列方向Householder向量 @param col 列数据 @param sigma 剩余范数符号 @return (v, beta) */
    QPair<QVector<double>, double> householderVector(const QVector<double>& col,
                                                     double sigma) const;

    /** @brief 应用左侧Householder变换: U_k = (I - beta*v*v^T) * A @param matrix 目标矩阵 @param v Householder向量 @param beta 系数 @param rowStart 起始行 @param colStart 起始列 */
    void applyLeftTransform(QVector<QVector<double>>& matrix,
                            const QVector<double>& v, double beta,
                            int rowStart, int colStart);

    /** @brief 应用右侧Householder变换: A * (I - beta*v*v^T) @param matrix 目标矩阵 @param v Householder向量 @param beta 系数 @param rowStart 起始行 @param colStart 起始列 */
    void applyRightTransform(QVector<QVector<double>>& matrix,
                             const QVector<double>& v, double beta,
                             int rowStart, int colStart);

    /** @brief 累积变换到U或V @param mat U或V矩阵 @param v Householder向量 @param beta 系数 @param startIdx 起始索引 */
    void accumulateTransform(QVector<QVector<double>>& mat,
                             const QVector<double>& v, double beta,
                             int startIdx);

    Dimensions m_lastDims;                  ///< 最近计算的矩阵维度
    Stats m_stats;                          ///< 运行统计
    double m_timeSum = 0.0;                 ///< 处理时间累加器
};
