/**
 * @file tree__311.cpp
 * @brief tree__311 implementation
 */
#include "tree311/tree__311.h"
QVector<double> tree__311::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}

