#pragma once

#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief Hessenberg7 - Hessenberg约化第7代实现
 *
 * 将一般矩阵约化为上Hessenberg形式 H = Q^T A Q，
 * 为QR迭代等特征值算法提供预处理步骤。
 */
class Hessenberg7 : public QObject {
    Q_OBJECT
public:
    struct Stats { int totalReductions = 0; double avgProcessingTimeMs = 0.0; };
    explicit Hessenberg7(QObject* parent = nullptr);
    Stats stats() const { return m_stats; }
    void resetStatistics();

    /**
     * @brief 执行Hessenberg约化 A = Q H Q^T
     * @param matrix 输入矩阵
     * @return 是否约化成功
     */
    bool reduce(const QVector<QVector<double>>& matrix);

    /**
     * @brief 获取Hessenberg矩阵H
     * @return 上Hessenberg矩阵
     */
    QVector<QVector<double>> hessenbergMatrix() const;

    /**
     * @brief 获取正交变换矩阵Q
     * @return 正交矩阵
     */
    QVector<QVector<double>> transformMatrix() const;

    /**
     * @brief 检查矩阵是否已经是Hessenberg形式
     * @param matrix 待检查矩阵
     * @param tolerance 数值容差
     * @return 是否为Hessenberg矩阵
     */
    bool isHessenberg(const QVector<QVector<double>>& matrix,
                      double tolerance = 1e-10) const;

signals:
    void reductionCompleted(int matrixSize);

private:
    Stats m_stats; double m_timeSum = 0.0;
};
