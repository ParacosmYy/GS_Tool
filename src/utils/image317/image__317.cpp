/**
 * @file image__317.cpp
 * @brief image__317 implementation
 */
#include "image317/image__317.h"
QVector<double> image__317::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}

