/**
 * @file SchurVector.h
 * @brief Schur向量提取 — 块三角形式+不变子空间+有序Schur分解
 *
 * 功能: 实数矩阵的Schur分解，提取Schur向量(不变子空间基)，
 *       支持块上三角形式、有序Schur分解(按特征值排序)、
 *       选取特定特征值的Schur向量。
 *
 * 协作: DataTransformer(数据变换) / DataDecomposer(矩阵分解)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief Schur向量提取引擎
 *
 * 对实矩阵执行 QR 迭代得到实 Schur 形式 (准上三角)，
 * 提取对应特定特征值的 Schur 向量，构成不变子空间。
 */
class SchurVector : public QObject {
    Q_OBJECT

public:
    /** @brief Schur分解结果 */
    struct SchurResult {
        QVector<QVector<double>> T;        ///< Schur形式(准上三角)
        QVector<QVector<double>> Q;        ///< 正交Schur向量矩阵
        QVector<double> eigenvalues;       ///< 特征值(实部)
        QVector<double> eigenvaluesImag;   ///< 特征值虚部
        int iterations = 0;                ///< 迭代次数
    };

    /** @brief 特征值选择条件 */
    struct EigenvalueSelector {
        double minReal = -1e300;        ///< 实部下界
        double maxReal = 1e300;         ///< 实部上界
        double minAbs = 0.0;            ///< 绝对值下界
        double maxAbs = 1e300;          ///< 绝对值上界
        bool selectLargest = false;     ///< 选择最大特征值
        int count = -1;                 ///< 选取个数(-1表示全部满足条件的)
    };

    /** @brief 统计信息 */
    struct Stats {
        quint64 totalDecompositions = 0;        ///< 累计分解次数
        quint64 totalEigenvaluesExtracted = 0;  ///< 累计提取特征值数
        double  avgProcessingTimeMs = 0.0;      ///< 平均处理时间(ms)
    };

    explicit SchurVector(QObject* parent = nullptr);

    void setMaxIterations(int iterations);
    void setTolerance(double tolerance);

    SchurResult decompose(const QVector<QVector<double>>& matrix);
    QVector<QVector<double>> extractSubspace(const SchurResult& result,
                                             const EigenvalueSelector& selector) const;
    QVector<QVector<double>> reorderSchur(SchurResult& result,
                                          const EigenvalueSelector& selector) const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void decompositionCompleted(int size, int iterations);

private:
    void qrIteration(QVector<QVector<double>>& A,
                     QVector<QVector<double>>& Q,
                     int& iterations);
    void householderQR(const QVector<QVector<double>>& A,
                       QVector<QVector<double>>& Q,
                       QVector<QVector<double>>& R) const;
    void extractEigenvalues(const QVector<QVector<double>>& T,
                            QVector<double>& real,
                            QVector<double>& imag) const;
    double frobeniusNorm(const QVector<QVector<double>>& A) const;
    QVector<QVector<double>> identity(int n) const;
    QVector<QVector<double>> matMul(const QVector<QVector<double>>& A,
                                    const QVector<QVector<double>>& B) const;

    int m_maxIterations;        ///< 最大迭代次数
    double m_tolerance;         ///< 收敛容差

    Stats m_stats;
    double m_timeSum = 0.0;
};
