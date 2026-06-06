/**
 * @file algo_7113.cpp
 */
#include "crypto7113/algo_7113.h"
QVector<double> algo_7113::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
