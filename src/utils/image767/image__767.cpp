/**
 * @file image__767.cpp
 * @brief image__767 implementation
 */
#include "image767/image__767.h"
QVector<double> image__767::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}

