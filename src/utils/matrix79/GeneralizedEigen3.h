#pragma once
#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief GeneralizedEigen3 - 广义特征值问题求解器
 *
 * 求解Ax = λBx广义特征值问题，支持对称正定B矩阵
 * 的Cholesky分解方法和QZ分解方法。
 */
class GeneralizedEigen3 : public QObject {
    Q_OBJECT
public:
    struct Stats {
        int totalEigensolves = 0;
        int totalEigenvalues = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit GeneralizedEigen3(QObject* parent = nullptr);

    /** @brief 求解广义特征值Ax = λBx */
    QVector<double> eigenvalues(const QVector<QVector<double>>& A,
                                const QVector<QVector<double>>& B);

    /** @brief 求解广义特征值和特征向量 */
    QPair<QVector<double>, QVector<QVector<double>>> eigenvectors(
        const QVector<QVector<double>>& A,
        const QVector<QVector<double>>& B);

    /** @brief 设置求解方法: "cholesky" 或 "qz" */
    void setMethod(const QString& method);

    Stats stats() const { return m_stats; }
    void resetStatistics();

signals:
    void eigensolveCompleted(int eigenvalueCount, bool converged);

private:
    Stats m_stats;
    double m_timeSum = 0.0;
    QString m_method = "cholesky";
};
