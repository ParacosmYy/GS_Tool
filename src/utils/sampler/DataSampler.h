/**
 * @file DataSampler.h
 * @brief 数据采样器 -- 从串口数据流中提取统计代表性样本
 *
 * 支持4种采样策略: 随机/系统/分层/蓄水池。
 */

#ifndef DATASAMPLER_H
#define DATASAMPLER_H

#include <QByteArray>
#include <QElapsedTimer>
#include <QList>
#include <QMap>
#include <QObject>
#include <QString>

class DataSampler : public QObject {
    Q_OBJECT

public:
    enum class SamplingStrategy { Random, Systematic, Stratified, Reservoir };

    struct SampleRecord {
        QByteArray data;
        qint64 timestampMs = 0;
        int sequenceNumber = 0;
        double samplingWeight = 1.0;
    };

    struct Stats {
        quint64 totalBytesFed = 0;
        quint64 totalSamplesCollected = 0;
        quint64 totalSamplesEvicted = 0;
        double samplingRate = 0.0;
        double avgSampleSize = 0.0;
        quint64 reservoirReplacements = 0;
    };

    explicit DataSampler(QObject* parent = nullptr);

    void setStrategy(SamplingStrategy strategy);
    SamplingStrategy strategy() const;
    void setSampleSize(int size);
    int sampleSize() const;
    void setSamplingRate(double rate);
    void feed(const QByteArray& data);
    QList<SampleRecord> samples() const;
    QByteArray aggregatedSample() const;
    void clear();
    Stats stats() const;
    void resetStatistics();

signals:
    void sampleCollected(const SampleRecord& record);
    void sampleEvicted(const SampleRecord& record);
    void reservoirFull();

private:
    SamplingStrategy m_strategy = SamplingStrategy::Random;
    int m_sampleSize = 100;
    double m_samplingRate = 0.1;
    QList<SampleRecord> m_samples;
    int m_feedCount = 0;
    int m_nextSystematicSample = 0;
    QElapsedTimer m_timer;
    Stats m_stats;
    quint64 m_totalSampleBytes = 0;
    void sampleRandom(const QByteArray& data);
    void sampleSystematic(const QByteArray& data);
    void sampleReservoir(const QByteArray& data);
    void addSample(const QByteArray& data);
};

#endif // DATASAMPLER_H
