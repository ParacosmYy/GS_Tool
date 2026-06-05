/**
 * @file tree__381.cpp
 * @brief tree__381 implementation
 */
#include "tree381/tree__381.h"
QVector<double> tree__381::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}

