/**
 * @file image__787.cpp
 * @brief image__787 implementation
 */
#include "image787/image__787.h"
QVector<double> image__787::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}

