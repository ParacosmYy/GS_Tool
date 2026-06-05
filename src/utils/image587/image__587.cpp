/**
 * @file image__587.cpp
 * @brief image__587 implementation
 */
#include "image587/image__587.h"
QVector<double> image__587::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}

