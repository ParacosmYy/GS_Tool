/**
 * @file interp__591.cpp
 * @brief interp__591 implementation
 */
#include "interp591/interp__591.h"
QVector<double> interp__591::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}

