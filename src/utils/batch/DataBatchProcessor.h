/**
 * @file DataBatchProcessor.h
 * @brief 数据批处理器 -- 按大小/时间/Hybrid策略累积数据并分批处理
 */

#ifndef DATABATCHPROCESSOR_H
#define DATABATCHPROCESSOR_H

#include <QByteArray>
#include <QElapsedTimer>
#include <QList>
#include <QObject>
#include <QTimer>

class DataBatchProcessor : public QObject {
    Q_OBJECT

public:
    enum class BatchStrategy { SizeBased, TimeBased, Hybrid };

    struct BatchConfig {
        int maxBatchSize = 1024;
        int flushIntervalMs = 1000;
        BatchStrategy strategy = BatchStrategy::Hybrid;
        bool dropOnOverflow = false;
        int maxBufferSize = 65536;
    };

    struct Stats {
        quint64 totalBatchesProcessed = 0;
        quint64 totalBytesProcessed = 0;
        quint64 totalFlushes = 0;
        quint64 totalTimeoutFlushes = 0;
        quint64 totalOverflows = 0;
        double avgBatchSize = 0.0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit DataBatchProcessor(QObject* parent = nullptr);
    ~DataBatchProcessor() override;

    void setConfig(const BatchConfig& config);
    const BatchConfig& config() const;
    void feed(const QByteArray& data);
    void flush();
    void clear();
    int bufferedSize() const;
    Stats stats() const;
    void resetStatistics();

signals:
    void batchReady(const QByteArray& batch);
    void overflow(int droppedBytes);
    void flushed(int bytesProcessed);

private:
    BatchConfig m_config;
    QByteArray m_buffer;
    QTimer m_flushTimer;
    QElapsedTimer m_batchTimer;
    Stats m_stats;
    quint64 m_sumBatchSize = 0;
    quint64 m_sumProcessingTime = 0;
    void processBatch();
    void checkFlushCondition();
    void setupFlushTimer();
};

#endif // DATABATCHPROCESSOR_H
