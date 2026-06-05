/**
 * @file graph__404.cpp
 * @brief graph__404 implementation
 */
#include "graph404/graph__404.h"
QVector<double> graph__404::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}

