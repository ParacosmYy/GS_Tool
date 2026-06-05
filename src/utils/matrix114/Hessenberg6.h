#pragma once
#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief 上Hessenberg矩阵约化求解器实现
 *
 * 通过Householder变换将一般矩阵约化为上Hessenberg形式(H[i][j]=0, i>j+1)，
 * 作为QR特征值算法的预处理步骤，也用于求解Hessenberg线性方程组。
 */
class Hessenberg6 : public QObject {
    Q_OBJECT
public:
    /// 统计信息结构体
    struct Stats { int totalReduced = 0; double avgProcessingTimeMs = 0.0; };

    explicit Hessenberg6(QObject* parent = nullptr);

    /** @brief 设置待约化矩阵的维度 */
    void setDimension(int n);

    /** @brief 设置矩阵元素，格式为(行,列,值) */
    void setEntry(int row, int col, double value);

    /** @brief 执行Hessenberg约化，返回变换后矩阵和正交变换Q */
    QPair<QVector<QVector<double>>, QVector<QVector<double>>> reduce();

    /** @brief 求解Hessenberg线性方程组Hx=b */
    QVector<double> solve(const QVector<QVector<double>>& H, const QVector<double>& b);

    Stats stats() const { return m_stats; }
    void resetStatistics();

signals:
    /** @brief 约化完成信号，返回矩阵维度 */
    void reductionCompleted(int dimension);

private:
    Stats m_stats;
    double m_timeSum = 0.0;
    int m_dimension = 0;
};
