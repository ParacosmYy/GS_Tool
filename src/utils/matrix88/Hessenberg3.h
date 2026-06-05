#pragma once
#include <QObject>
#include <QVector>

/**
 * @brief Hessenberg矩阵约化
 *
 * 将一般矩阵约化为上Hessenberg形式，为QR迭代做准备。
 */
class Hessenberg3 : public QObject {
    Q_OBJECT
public:
    struct Stats {
        int totalReductions = 0;
        int totalTransformations = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit Hessenberg3(QObject* parent = nullptr);

    /** @brief 将矩阵约化为上Hessenberg形式 */
    QVector<QVector<double>> reduce(const QVector<QVector<double>>& matrix);

    /** @brief 获取正交变换矩阵Q */
    QVector<QVector<double>> transformationMatrix() const;

    Stats stats() const { return m_stats; }
    void resetStatistics();

signals:
    void reductionCompleted(int dimension, double offDiagNorm);

private:
    Stats m_stats;
    double m_timeSum = 0.0;
    QVector<QVector<double>> m_Q;
};
