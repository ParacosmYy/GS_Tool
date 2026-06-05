#pragma once
#include <QObject>
#include <QVector>
#include <QByteArray>
class ConvolutionalInterleaver2 : public QObject {
    Q_OBJECT
public:
    struct Stats { int totalInterleaves = 0; int totalSymbolsProcessed = 0; double avgProcessingTimeMs = 0.0; };
    explicit ConvolutionalInterleaver2(QObject* parent = nullptr);
    void configure(int branches, int delay);
    QByteArray interleave(const QByteArray& data);
    QByteArray deinterleave(const QByteArray& data);
    void reset();
    Stats stats() const { return m_stats; }
    void resetStatistics();
signals:
    void interleaved(int symbols);
private:
    int m_branches = 0; int m_delay = 0;
    QVector<QVector<quint8>> m_fifo;
    Stats m_stats; double m_timeSum = 0.0;
};
