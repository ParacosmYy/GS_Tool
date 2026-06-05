/**
 * @file ErasureCode3.h
 * @brief 擦除码3 — Cauchy Reed-Solomon+矩阵编码
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QPair>

class ErasureCode3 : public QObject
{
    Q_OBJECT

public:
    struct Stats {
        int totalEncodes = 0;
        int totalDecodes = 0;
        int totalShardsProcessed = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit ErasureCode3(QObject* parent = nullptr);

    void setParameters(int dataShards, int parityShards, int shardSize);
    QVector<QVector<quint8>> encode(const QVector<QVector<quint8>>& data);
    bool decode(QVector<QVector<quint8>>& shards,
                const QVector<int>& erased);
    QVector<int> checkShards(const QVector<QVector<quint8>>& shards) const;

    int dataShards() const { return m_dataShards; }
    int parityShards() const { return m_parityShards; }
    int totalShards() const { return m_dataShards + m_parityShards; }
    Stats stats() const { return m_stats; }
    void resetStatistics();

signals:
    void encodeCompleted(int dataShards, int parityShards);
    void decodeCompleted(int recoveredShards);

private:
    int m_dataShards = 4;
    int m_parityShards = 2;
    int m_shardSize = 4096;
    QVector<QVector<quint8>> m_codingMatrix;

    void buildCauchyMatrix();
    void gfMatrixMultiply(const QVector<QVector<quint8>>& A,
                           const QVector<QVector<quint8>>& B,
                           QVector<QVector<quint8>>& C) const;
    quint8 gfMul(quint8 a, quint8 b) const;
    quint8 gfInv(quint8 a) const;

    Stats m_stats;
    double m_timeSum = 0.0;
};
