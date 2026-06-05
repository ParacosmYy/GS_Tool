/**
 * @file tree__411.cpp
 * @brief tree__411 implementation
 */
#include "tree411/tree__411.h"
QVector<double> tree__411::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}

