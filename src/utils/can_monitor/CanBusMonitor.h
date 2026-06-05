/**
 * @file CanBusMonitor.h
 * @brief Monitor and decode CAN bus frames from serial adapter
 */
#pragma once
#include <QObject>
#include <QByteArray>
#include <QVector>
#include <QString>
#include <QDateTime>

/**
 * @brief Monitor and decode CAN bus frames from serial adapter
 */
class CanBusMonitor : public QObject {
    Q_OBJECT
public:
    /** @brief Statistics counters */
    struct Stats {
        quint64 operationsPerformed = 0;
        quint64 bytesProcessed = 0;
        quint64 errorsDetected = 0;
        quint64 lastOperationMs = 0;
    };

    explicit CanBusMonitor(QObject *parent = nullptr);
    ~CanBusMonitor() override;

    /** @brief Process input data and return result */
    QByteArray process(const QByteArray &input);

    /** @brief Reset all statistics */
    void resetStatistics();

    /** @brief Get current statistics */
    Stats statistics() const { return m_stats; }

signals:
    /** @brief Emitted when processing completes */
    void processingComplete(const QByteArray &result);
    /** @brief Emitted on error */
    void errorOccurred(const QString &message);

private:
    Stats m_stats;
};

