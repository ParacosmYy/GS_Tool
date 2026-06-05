/**
 * @file image__387.cpp
 * @brief image__387 implementation
 */
#include "image387/image__387.h"
QVector<double> image__387::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}

