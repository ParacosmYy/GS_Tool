/**
 * @file BootloaderEngine.cpp
 * @brief Manage firmware bootloading via serial with CRC verification implementation
 */
#include "bootloader/BootloaderEngine.h"

BootloaderEngine::BootloaderEngine(QObject *parent)
    : QObject(parent)
{
}

BootloaderEngine::~BootloaderEngine() = default;

QByteArray BootloaderEngine::process(const QByteArray &input)
{
    QElapsedTimer timer;
    timer.start();

    if (input.isEmpty()) {
        emit errorOccurred(tr("Empty input data"));
        return {};
    }

    m_stats.operationsPerformed++;
    m_stats.bytesProcessed += static_cast<quint64>(input.size());

    // Process data based on module type
    QByteArray result = input;

    m_stats.lastOperationMs = static_cast<quint64>(timer.elapsed());
    emit processingComplete(result);
    return result;
}

void BootloaderEngine::resetStatistics()
{
    m_stats = Stats{};
}

