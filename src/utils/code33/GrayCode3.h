/**
 * @file GrayCode3.h
 * @brief 格雷码增强 — 编码解码/多位翻转/距离矩阵/信号映射
 */
#pragma once
#include <QObject>
#include <QVector>
#include <QString>
#include <QSet>
class GrayCode3 : public QObject {
    Q_OBJECT
public:
    struct Stats { int totalEncodes = 0; int totalDecodes = 0; double avgProcessingTimeMs = 0.0; };
    explicit GrayCode3(QObject* parent = nullptr);
    quint32 encode(quint32 binary) const;
    quint32 decode(quint32 gray) const;
    QVector<quint32> generateSequence(int bits) const;
    int hammingDistance(quint32 a, quint32 b) const;
    QVector<QVector<int>> distanceMatrix(int bits) const;
    QVector<quint32> neighbors(quint32 gray, int bits) const;
    bool isValidSequence(const QVector<quint32>& sequence, int bits) const;
    QString toString(quint32 gray, int bits) const;
    Stats stats() const { return m_stats; }
    void resetStatistics();
signals:
    void sequenceGenerated(int bits, int length);
private:
    Stats m_stats; double m_timeSum = 0.0;
};
