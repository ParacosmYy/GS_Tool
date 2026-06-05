/**
 * @file DataSampler.cpp
 * @brief 数据采样器实现 -- 随机/系统/蓄水池采样算法
 */

#include "utils/sampler/DataSampler.h"

#include <QRandomGenerator>
#include <algorithm>

DataSampler::DataSampler(QObject* parent) : QObject(parent)
{
    setObjectName(QStringLiteral("DataSampler"));
    m_timer.start();
}

void DataSampler::setStrategy(SamplingStrategy s) { m_strategy = s; m_feedCount = 0; }
DataSampler::SamplingStrategy DataSampler::strategy() const { return m_strategy; }
void DataSampler::setSampleSize(int size) { m_sampleSize = qBound(1, size, 10000); }
int DataSampler::sampleSize() const { return m_sampleSize; }
void DataSampler::setSamplingRate(double rate) { m_samplingRate = qBound(0.001, rate, 1.0); }

void DataSampler::feed(const QByteArray& data)
{
    if (data.isEmpty()) return;
    m_stats.totalBytesFed += static_cast<quint64>(data.size());
    ++m_feedCount;

    switch (m_strategy) {
    case SamplingStrategy::Random:     sampleRandom(data); break;
    case SamplingStrategy::Systematic: sampleSystematic(data); break;
    case SamplingStrategy::Stratified: sampleRandom(data); break; /* 分层退化为随机 */
    case SamplingStrategy::Reservoir:  sampleReservoir(data); break;
    }

    /* 更新统计 */
    m_stats.samplingRate = (m_stats.totalSamplesCollected > 0 && m_feedCount > 0)
        ? static_cast<double>(m_stats.totalSamplesCollected) / static_cast<double>(m_feedCount)
        : 0.0;
    m_stats.avgSampleSize = (m_stats.totalSamplesCollected > 0)
        ? static_cast<double>(m_totalSampleBytes) / static_cast<double>(m_stats.totalSamplesCollected)
        : 0.0;
}

void DataSampler::sampleRandom(const QByteArray& data)
{
    double r = QRandomGenerator::global()->generateDouble();
    if (r <= m_samplingRate) addSample(data);
}

void DataSampler::sampleSystematic(const QByteArray& data)
{
    if (m_nextSystematicSample <= 0) {
        addSample(data);
        m_nextSystematicSample = qMax(1, static_cast<int>(1.0 / m_samplingRate));
    }
    --m_nextSystematicSample;
}

void DataSampler::sampleReservoir(const QByteArray& data)
{
    if (m_samples.size() < m_sampleSize) {
        addSample(data);
        if (m_samples.size() >= m_sampleSize) emit reservoirFull();
    } else {
        /* Vitter's Algorithm R: 随机替换 */
        int idx = QRandomGenerator::global()->bounded(m_feedCount);
        if (idx < m_sampleSize) {
            m_totalSampleBytes -= static_cast<quint64>(m_samples[idx].data.size());
            m_samples[idx].data = data;
            m_samples[idx].timestampMs = m_timer.elapsed();
            m_samples[idx].sequenceNumber = m_feedCount;
            m_totalSampleBytes += static_cast<quint64>(data.size());
            ++m_stats.reservoirReplacements;
        }
    }
}

void DataSampler::addSample(const QByteArray& data)
{
    SampleRecord rec;
    rec.data = data;
    rec.timestampMs = m_timer.elapsed();
    rec.sequenceNumber = m_feedCount;

    /* 超出样本容量则淘汰最旧 */
    if (m_samples.size() >= m_sampleSize && m_strategy != SamplingStrategy::Reservoir) {
        SampleRecord evicted = m_samples.takeFirst();
        m_totalSampleBytes -= static_cast<quint64>(evicted.data.size());
        ++m_stats.totalSamplesEvicted;
        emit sampleEvicted(evicted);
    }

    m_samples.append(rec);
    m_totalSampleBytes += static_cast<quint64>(data.size());
    ++m_stats.totalSamplesCollected;
    emit sampleCollected(rec);
}

QList<DataSampler::SampleRecord> DataSampler::samples() const { return m_samples; }

QByteArray DataSampler::aggregatedSample() const
{
    QByteArray result;
    for (const auto& s : m_samples) result.append(s.data);
    return result;
}

void DataSampler::clear()
{
    m_samples.clear();
    m_feedCount = 0;
    m_nextSystematicSample = 0;
    m_totalSampleBytes = 0;
}

DataSampler::Stats DataSampler::stats() const { return m_stats; }

void DataSampler::resetStatistics()
{
    m_stats = Stats{};
    m_totalSampleBytes = 0;
}
