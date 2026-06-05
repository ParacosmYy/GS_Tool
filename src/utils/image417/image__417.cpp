/**
 * @file image__417.cpp
 * @brief image__417 implementation
 */
#include "image417/image__417.h"
QVector<double> image__417::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}

