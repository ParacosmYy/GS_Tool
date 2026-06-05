/**
 * @file HexEditor.cpp
 * @brief Hex editor widget for binary data viewing and editing implementation
 */
#include "hexedit/HexEditor.h"

HexEditor::HexEditor(QObject *parent)
    : QObject(parent)
{
}

HexEditor::~HexEditor() = default;

QByteArray HexEditor::process(const QByteArray &input)
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

void HexEditor::resetStatistics()
{
    m_stats = Stats{};
}

