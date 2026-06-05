/**
 * @file image__717.cpp
 * @brief image__717 implementation
 */
#include "image717/image__717.h"
QVector<double> image__717::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}

