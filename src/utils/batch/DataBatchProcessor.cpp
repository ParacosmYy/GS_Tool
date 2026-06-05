/**
 * @file DataBatchProcessor.cpp
 * @brief 数据批处理器实现 -- 累积+分批+超时刷新
 */

#include "utils/batch/DataBatchProcessor.h"

DataBatchProcessor::DataBatchProcessor(QObject* parent)
    : QObject(parent)
{
    setObjectName(QStringLiteral("DataBatchProcessor"));
    setupFlushTimer();
}

DataBatchProcessor::~DataBatchProcessor()
{
    if (m_flushTimer.isActive()) m_flushTimer.stop();
}

void DataBatchProcessor::setupFlushTimer()
{
    m_flushTimer.setSingleShot(true);
    connect(&m_flushTimer, &QTimer::timeout, this, [this]() {
        if (!m_buffer.isEmpty()) {
            ++m_stats.totalTimeoutFlushes;
            flush();
        }
    });
}

void DataBatchProcessor::setConfig(const BatchConfig& config)
{
    m_config = config;
    m_config.maxBatchSize = qBound(64, m_config.maxBatchSize, 1048576);
    m_config.maxBufferSize = qBound(1024, m_config.maxBufferSize, 16777216);
    m_config.flushIntervalMs = qBound(50, m_config.flushIntervalMs, 60000);
    setupFlushTimer();
}

const DataBatchProcessor::BatchConfig& DataBatchProcessor::config() const { return m_config; }

void DataBatchProcessor::feed(const QByteArray& data)
{
    if (data.isEmpty()) return;

    /* 溢出检查 */
    if (m_buffer.size() + data.size() > m_config.maxBufferSize) {
        if (m_config.dropOnOverflow) {
            int dropped = data.size();
            m_buffer.clear();
            ++m_stats.totalOverflows;
            emit overflow(dropped);
            return;
        }
        /* 不丢弃则强制刷新 */
        flush();
    }

    m_buffer.append(data);

    /* 启动超时定时器(首次数据到达) */
    if (m_config.strategy != BatchStrategy::SizeBased && !m_flushTimer.isActive()) {
        m_flushTimer.start(m_config.flushIntervalMs);
        m_batchTimer.start();
    }

    checkFlushCondition();
}

void DataBatchProcessor::checkFlushCondition()
{
    bool shouldFlush = false;
    switch (m_config.strategy) {
    case BatchStrategy::SizeBased:
        shouldFlush = m_buffer.size() >= m_config.maxBatchSize;
        break;
    case BatchStrategy::TimeBased:
        break; /* 由定时器驱动 */
    case BatchStrategy::Hybrid:
        shouldFlush = m_buffer.size() >= m_config.maxBatchSize;
        break;
    }
    if (shouldFlush) processBatch();
}

void DataBatchProcessor::processBatch()
{
    if (m_buffer.isEmpty()) return;

    QElapsedTimer timer;
    timer.start();

    QByteArray batch = m_buffer.left(m_config.maxBatchSize);
    m_buffer.remove(0, m_config.maxBatchSize);

    ++m_stats.totalBatchesProcessed;
    m_stats.totalBytesProcessed += static_cast<quint64>(batch.size());
    m_sumBatchSize += static_cast<quint64>(batch.size());

    double elapsed = static_cast<double>(timer.elapsed());
    m_sumProcessingTime += static_cast<quint64>(elapsed * 1000.0);

    if (m_stats.totalBatchesProcessed > 0) {
        m_stats.avgBatchSize = static_cast<double>(m_sumBatchSize) /
                               static_cast<double>(m_stats.totalBatchesProcessed);
    }
    if (m_stats.totalBatchesProcessed > 0) {
        m_stats.avgProcessingTimeMs = static_cast<double>(m_sumProcessingTime) /
                                      static_cast<double>(m_stats.totalBatchesProcessed) / 1000.0;
    }

    emit batchReady(batch);

    if (m_buffer.isEmpty() && m_flushTimer.isActive()) {
        m_flushTimer.stop();
    }
}

void DataBatchProcessor::flush()
{
    m_flushTimer.stop();
    if (m_buffer.isEmpty()) return;
    int bytes = m_buffer.size();
    ++m_stats.totalFlushes;

    QByteArray all = std::move(m_buffer);
    m_buffer.clear();

    ++m_stats.totalBatchesProcessed;
    m_stats.totalBytesProcessed += static_cast<quint64>(bytes);
    m_sumBatchSize += static_cast<quint64>(bytes);

    if (m_stats.totalBatchesProcessed > 0) {
        m_stats.avgBatchSize = static_cast<double>(m_sumBatchSize) /
                               static_cast<double>(m_stats.totalBatchesProcessed);
    }

    emit batchReady(all);
    emit flushed(bytes);
}

void DataBatchProcessor::clear()
{
    m_buffer.clear();
    m_flushTimer.stop();
}

int DataBatchProcessor::bufferedSize() const { return m_buffer.size(); }

DataBatchProcessor::Stats DataBatchProcessor::stats() const { return m_stats; }

void DataBatchProcessor::resetStatistics()
{
    m_stats = Stats{};
    m_sumBatchSize = 0;
    m_sumProcessingTime = 0;
}
