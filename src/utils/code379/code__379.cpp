/**
 * @file code__379.cpp
 * @brief code__379 implementation
 */
#include "code379/code__379.h"
QVector<double> code__379::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}

